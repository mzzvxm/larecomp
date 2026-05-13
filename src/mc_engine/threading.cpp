#include "threading.h"

#include "logging.h"

#include <rex/cvar.h>
#include <rex/ppc.h>
#include <rex/types.h>
#include <rex/system/xthread.h>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <timeapi.h>
#include <chrono>
#include <mutex>
#include <thread>
#endif

namespace mc {

void EnableHighResTimer() {
#if defined(_WIN32)
    static std::once_flag s_init;
    std::call_once(s_init, [] {
        timeBeginPeriod(1);
        MC_INFO("[threading] high-res timer enabled");
    });
#endif
}

void DisableHighResTimer() {
#if defined(_WIN32)
    timeEndPeriod(1);
    MC_INFO("[threading] high-res timer disabled");
#endif
}

} // namespace mc

// ---------------------------------------------------------------------------
// PPC kernel bypass hooks (Windows only)
// ---------------------------------------------------------------------------

#if defined(_WIN32)

// Sleep (0x8244FEC0)
u32 Sleep_hook(u32 ms) {
    mc::EnableHighResTimer();

    if (uint32_t(ms) == 0) {
        SwitchToThread();
        return 0;
    }

    auto target = std::chrono::steady_clock::now()
                + std::chrono::milliseconds(uint32_t(ms));

    if (uint32_t(ms) >= 2) {
        std::this_thread::sleep_for(
            std::chrono::milliseconds(uint32_t(ms)) - std::chrono::microseconds(1500));
    } else {
        SwitchToThread();
    }

    while (std::chrono::steady_clock::now() < target)
        YieldProcessor();

    return 0;
}
REX_HOOK(mc_Sleep, Sleep_hook);

// ResumeThread (0x8244FE58)
u32 ResumeThread_hook(u32 handle) {
    auto thread = REX_KERNEL_OBJECTS()->LookupObject<rex::system::XThread>(handle);
    if (thread)
        thread->Resume();
    return 0;
}
REX_HOOK(mc_ResumeThread, ResumeThread_hook);

#endif // _WIN32
