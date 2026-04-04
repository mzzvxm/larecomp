// larecomp - ReXGlue Recompiled Project
//
// This file is yours to edit. 'rexglue migrate' will NOT overwrite it.
// Customize your app by overriding virtual hooks from rex::ReXApp.

#pragma once

#include <rex/rex_app.h>
#include <rex/logging.h>
#include <rex/ui/window.h>
#include <chrono>
#include <windows.h>
#include <dbghelp.h>

#pragma comment(lib, "dbghelp.lib")

// Declaração externa para que o header conheça a variável definida no main.cpp
extern uint8_t* g_guest_mem;

class LarecompApp : public rex::ReXApp {
 public:
  LarecompApp(rex::ui::WindowedAppContext& ctx, std::string_view name, rex::PPCImageInfo ppc_info)
      : rex::ReXApp(ctx, name, ppc_info) {

      // Inicializa o logging para arquivo e terminal
      rex::InitLogging("debug_la.txt", spdlog::level::debug);
      REXLOG_INFO("LA Recompiled - @by mzzvxm");

      // Configuração de Flush para não perder logs em caso de crash repentino
      spdlog::flush_on(spdlog::level::warn);
      spdlog::flush_every(std::chrono::seconds(1));

      // Hook de Exceções para Stack Trace
      SetUnhandledExceptionFilter([](EXCEPTION_POINTERS* ep) -> LONG {
          REXLOG_ERROR("!!! CRASH DETECTADO !!!");
          REXLOG_ERROR("Código: 0x{:08X} no endereço 0x{:016X}",
              ep->ExceptionRecord->ExceptionCode,
              (uintptr_t)ep->ExceptionRecord->ExceptionAddress);

          HANDLE hProcess = GetCurrentProcess();
          HANDLE hThread  = GetCurrentThread();
          SymInitialize(hProcess, nullptr, TRUE);

          CONTEXT ctx = *ep->ContextRecord;

          STACKFRAME64 frame{};
          frame.AddrPC.Offset    = ctx.Rip;
          frame.AddrPC.Mode      = AddrModeFlat;
          frame.AddrFrame.Offset = ctx.Rbp;
          frame.AddrFrame.Mode   = AddrModeFlat;
          frame.AddrStack.Offset = ctx.Rsp;
          frame.AddrStack.Mode   = AddrModeFlat;

          HMODULE hExe = GetModuleHandleA(nullptr);
          uintptr_t base = (uintptr_t)hExe;

          REXLOG_ERROR("--- Stack Trace Real ---");
          char sym_buf[sizeof(SYMBOL_INFO) + 256];
          auto* sym = reinterpret_cast<SYMBOL_INFO*>(sym_buf);
          sym->SizeOfStruct = sizeof(SYMBOL_INFO);
          sym->MaxNameLen   = 255;

          for (int i = 0; i < 24; i++) {
              if (!StackWalk64(IMAGE_FILE_MACHINE_AMD64, hProcess, hThread,
                               &frame, &ctx, nullptr,
                               SymFunctionTableAccess64, SymGetModuleBase64, nullptr))
                  break;
              if (frame.AddrPC.Offset == 0) break;

              uintptr_t addr   = (uintptr_t)frame.AddrPC.Offset;
              uintptr_t offset = addr - base;

              if (SymFromAddr(hProcess, addr, nullptr, sym)) {
                  REXLOG_ERROR("  #{}: {} (0x{:016X}) [base+0x{:X}]", i, sym->Name, addr, offset);
              } else {
                  REXLOG_ERROR("  #{}: 0x{:016X} [base+0x{:X}]", i, addr, offset);
              }
          }

          // Registradores no momento do crash (restaurando RCX e RDX)
          REXLOG_ERROR("Registradores: RIP={:016X} RAX={:016X} RCX={:016X} RDX={:016X} RSP={:016X}",
              ep->ContextRecord->Rip, ep->ContextRecord->Rax,
              ep->ContextRecord->Rcx, ep->ContextRecord->Rdx,
              ep->ContextRecord->Rsp);

          // Dumpa os primeiros 8 return addresses da stack manualmente via RSP
          REXLOG_ERROR("Stack manual (via RSP):");
          uintptr_t* rsp = reinterpret_cast<uintptr_t*>(ep->ContextRecord->Rsp);
          for (int i = 0; i < 8; i++) {
              __try {
                  uintptr_t ret = rsp[i];
                  uintptr_t off = ret - base;
                  REXLOG_ERROR("  RSP+{:02X}: 0x{:016X} [base+0x{:X}]", i*8, ret, off);
              } __except(EXCEPTION_EXECUTE_HANDLER) { break; }
          }

          spdlog::default_logger()->flush();
          spdlog::shutdown();
          return EXCEPTION_CONTINUE_SEARCH;
      });

      if (auto* w = window()) {
          w->SetTitle("Midnig Los - by @mzzvxm");
      }
  }

  static std::unique_ptr<rex::ui::WindowedApp> Create(
      rex::ui::WindowedAppContext& ctx) {
    // Nota: Usamos o construtor customizado definido acima
    return std::unique_ptr<LarecompApp>(new LarecompApp(ctx, "larecomp",
        PPCImageConfig));
  }

  // Overrides virtuais podem ser adicionados abaixo conforme necessário
  void OnPostSetup() override {
      REXLOG_INFO("by @mzzvxm. base memory: 0x{:016X}", (uintptr_t)g_guest_mem);
  }
};