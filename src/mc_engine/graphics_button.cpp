#include "graphics_button.h"

#include "logging.h"

#include <atomic>
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <chrono>
#include <thread>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

extern uint8_t* g_guest_mem;

namespace mc::ui {
namespace {

std::atomic_bool g_f4_in_flight{false};

constexpr uint32_t kGuestBase = 0x82000000u;

// Strings runtime. Elas NÃO vão para o default.xex.
// O jogo só precisa enxergar ponteiros válidos se/ quando copiarmos para
// memória guest. Para hooks host-side, usamos direto como const char*.
constexpr const char* kGraphicsLabel = "Graphics";
constexpr const char* kGraphicsAction = "PM_Graphics";

// Handler conhecido pelo v5.
constexpr uint32_t kPauseActionEvent = 0x37;

// Possíveis listas observadas em construtores de menus/opções.
// Ainda não assumimos que são corretas para o pause menu principal.
struct CandidateList {
    uint32_t list_off;
    uint32_t count_off;
};

constexpr CandidateList kCandidateLists[] = {
    {0x44,  0x48},
    {0x350, 0x354},
};

std::atomic_bool g_initialized{false};
std::atomic_bool g_install_attempted{false};

uint8_t* GuestPtr(uint32_t ea) {
    if (!g_guest_mem)
        return nullptr;

    if (ea < kGuestBase)
        return nullptr;

    return g_guest_mem + (ea - kGuestBase);
}

bool IsGuestPtr(uint32_t ea) {
    return ea >= 0x82000000u && ea < 0x90000000u;
}

uint8_t ReadGuest8(uint32_t ea) {
    auto* p = GuestPtr(ea);
    return p ? p[0] : 0;
}

uint16_t ReadGuestBE16(uint32_t ea) {
    auto* p = GuestPtr(ea);
    if (!p)
        return 0;

    return (uint16_t(p[0]) << 8) |
           (uint16_t(p[1]) << 0);
}

uint32_t ReadGuestBE32(uint32_t ea) {
    auto* p = GuestPtr(ea);
    if (!p)
        return 0;

    return (uint32_t(p[0]) << 24) |
           (uint32_t(p[1]) << 16) |
           (uint32_t(p[2]) << 8)  |
           (uint32_t(p[3]) << 0);
}

void WriteGuestBE16(uint32_t ea, uint16_t v) {
    auto* p = GuestPtr(ea);
    if (!p)
        return;

    p[0] = uint8_t((v >> 8) & 0xFF);
    p[1] = uint8_t((v >> 0) & 0xFF);
}

void WriteGuestBE32(uint32_t ea, uint32_t v) {
    auto* p = GuestPtr(ea);
    if (!p)
        return;

    p[0] = uint8_t((v >> 24) & 0xFF);
    p[1] = uint8_t((v >> 16) & 0xFF);
    p[2] = uint8_t((v >> 8) & 0xFF);
    p[3] = uint8_t((v >> 0) & 0xFF);
}

bool LooksLikePointerArray(uint32_t list_ea, uint16_t count) {
    if (!IsGuestPtr(list_ea))
        return false;

    if (count == 0 || count > 64)
        return false;

    for (uint16_t i = 0; i < count; ++i) {
        const uint32_t item = ReadGuestBE32(list_ea + uint32_t(i) * 4);
        if (!IsGuestPtr(item))
            return false;
    }

    return true;
}

#if defined(_WIN32)

struct FindMainWindowData {
    DWORD pid = 0;
    HWND hwnd = nullptr;
};

BOOL CALLBACK EnumMainWindowProc(HWND hwnd, LPARAM lParam) {
    auto* data = reinterpret_cast<FindMainWindowData*>(lParam);

    DWORD window_pid = 0;
    GetWindowThreadProcessId(hwnd, &window_pid);

    if (window_pid != data->pid)
        return TRUE;

    if (!IsWindowVisible(hwnd))
        return TRUE;

    if (GetWindow(hwnd, GW_OWNER) != nullptr)
        return TRUE;

    wchar_t title[256]{};
    GetWindowTextW(hwnd, title, 256);

    // Aceita janela visível do processo mesmo se o título estiver vazio,
    // mas prefere uma janela com título.
    data->hwnd = hwnd;

    if (title[0] != L'\0')
        return FALSE;

    return TRUE;
}

HWND FindCurrentProcessMainWindow() {
    FindMainWindowData data{};
    data.pid = GetCurrentProcessId();

    EnumWindows(EnumMainWindowProc, reinterpret_cast<LPARAM>(&data));
    return data.hwnd;
}

void SendF4HeldAsync(HWND hwnd) {
    if (g_f4_in_flight.exchange(true))
        return;

    std::thread([hwnd]() {
        if (hwnd) {
            SetForegroundWindow(hwnd);
            SetFocus(hwnd);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        INPUT down{};
        down.type = INPUT_KEYBOARD;
        down.ki.wVk = VK_F4;

        UINT sent_down = SendInput(1, &down, sizeof(INPUT));

        MC_INFO("[graphics-button] VK_F4 down sent={}", sent_down);

        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        INPUT up{};
        up.type = INPUT_KEYBOARD;
        up.ki.wVk = VK_F4;
        up.ki.dwFlags = KEYEVENTF_KEYUP;

        UINT sent_up = SendInput(1, &up, sizeof(INPUT));

        MC_INFO("[graphics-button] VK_F4 up sent={}", sent_up);

        g_f4_in_flight = false;
    }).detach();
}

void PostF4ToGameWindow() {
    HWND hwnd = FindCurrentProcessMainWindow();

    if (!hwnd) {
        MC_WARN("[graphics-button] could not find process window");
        return;
    }

    MC_INFO("[graphics-button] found game window hwnd=0x{:016X}",
            reinterpret_cast<std::uintptr_t>(hwnd));

    SendF4HeldAsync(hwnd);

    MC_INFO("[graphics-button] requested ReXGlue overlay via held VK_F4");
}

#endif

void TryInstallGraphicsButtonRuntime(uint32_t maybe_menu_this) {
    MC_INFO("[graphics-button] trying runtime install from menu object 0x{:08X}",
            maybe_menu_this);

    if (!IsGuestPtr(maybe_menu_this)) {
        MC_WARN("[graphics-button] menu object is not a valid guest ptr: 0x{:08X}",
                maybe_menu_this);
        return;
    }

    for (const auto& c : kCandidateLists) {
        const uint32_t list_ea = ReadGuestBE32(maybe_menu_this + c.list_off);
        const uint16_t count = ReadGuestBE16(maybe_menu_this + c.count_off);

        MC_INFO("[graphics-button] probing list off=0x{:X}/0x{:X}: list=0x{:08X} count={}",
                c.list_off, c.count_off, list_ea, count);

        if (!LooksLikePointerArray(list_ea, count))
            continue;

        MC_INFO("[graphics-button] candidate list found: this=0x{:08X} list_off=0x{:X} list=0x{:08X} count={}",
                maybe_menu_this, c.list_off, list_ea, count);
    }
}

} // namespace

void InitGraphicsButtonPatch() {
    if (g_initialized.exchange(true))
        return;

    MC_INFO("[graphics-button] initialized. label='{}' action='{}'",
            kGraphicsLabel,
            kGraphicsAction);
}

void RequestOpenRexGraphicsMenu() {
#if defined(_WIN32)
    PostF4ToGameWindow();
#else
    MC_WARN("[graphics-button] RequestOpenRexGraphicsMenu is only implemented on Windows");
#endif
}

bool Hook_HandlePauseActionGraphics(PPCRegister& r3, PPCRegister& r4) {
    // Por enquanto, esse hook não consegue saber se PM_Graphics foi clicado,
    // porque o botão visual ainda não existe e a action não entra no dispatcher.
    //
    // Mantemos este hook como ponto final para quando o botão visual for instalado.
    // Quando a action PM_Graphics existir no sistema do jogo, aqui abrimos o overlay.

    if (uint32_t(r4.u64) != kPauseActionEvent)
        return false;

    return false;
}

bool Hook_TryInstallGraphicsButton(PPCRegister& r3, PPCRegister& r4) {
    TryInstallGraphicsButtonRuntime(uint32_t(r3.u64));
    return false;
}

} // namespace mc::ui