#pragma once

#include <rex/rex_app.h>
#include <rex/logging.h>
#include <rex/ui/window.h>

#include <chrono>
#include <cstdint>
#include <memory>
#include <string_view>
#include <filesystem>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include <dbghelp.h>
#include <spdlog/spdlog.h>
#endif

// External declaration so that the header file can access the variable defined in main.cpp
extern uint8_t* g_guest_mem;

namespace larecomp::log {
inline const rex::LogCategoryId App =
    rex::RegisterLogCategory("larecomp.app");

inline const rex::LogCategoryId Crash =
    rex::RegisterLogCategory("larecomp.crash");
}

#define LARECOMP_APP_INFO(...)     REXLOG_CAT_INFO(::larecomp::log::App, __VA_ARGS__)
#define LARECOMP_APP_WARN(...)     REXLOG_CAT_WARN(::larecomp::log::App, __VA_ARGS__)
#define LARECOMP_APP_ERROR(...)    REXLOG_CAT_ERROR(::larecomp::log::App, __VA_ARGS__)

#define LARECOMP_CRASH_INFO(...)   REXLOG_CAT_INFO(::larecomp::log::Crash, __VA_ARGS__)
#define LARECOMP_CRASH_WARN(...)   REXLOG_CAT_WARN(::larecomp::log::Crash, __VA_ARGS__)
#define LARECOMP_CRASH_ERROR(...)  REXLOG_CAT_ERROR(::larecomp::log::Crash, __VA_ARGS__)

class LarecompApp : public rex::ReXApp {
 public:
  LarecompApp(rex::ui::WindowedAppContext& ctx,
              std::string_view name,
              rex::PPCImageInfo ppc_info)
      : rex::ReXApp(ctx, name, ppc_info) {
    InitLarecompLogging();

    LARECOMP_APP_INFO("LA Recompiled - @by mzzvxm");

    InstallCrashLogger();

    if (auto* w = window()) {
      w->SetTitle("Midnig Los - by @mzzvxm");
    }
  }

  static std::unique_ptr<rex::ui::WindowedApp> Create(
      rex::ui::WindowedAppContext& ctx) {
    return std::unique_ptr<LarecompApp>(
        new LarecompApp(ctx, "larecomp", PPCImageConfig));
  }

  void OnConfigurePaths(rex::PathConfig& paths) override {
    const auto root = ExeDir();

    const auto default_game_data = root / "assets";

    // If the user has manually passed --game_data_root and it is valid,
    // keeps the manual path.
    if (!paths.game_data_root.empty() &&
        std::filesystem::exists(paths.game_data_root / "default.xex")) {
        return;
    }

    // Default path: the “asset” folder next to larecomp.exe
    paths.game_data_root = default_game_data;

    // Optional: only define update if the folder exists.
    const auto update = root / "update";
    if (std::filesystem::exists(update)) {
        paths.update_data_root = update;
    }
}

  void OnPostSetup() override {
    LARECOMP_APP_INFO("by @mzzvxm. base memory: 0x{:016X}",
                      reinterpret_cast<std::uintptr_t>(g_guest_mem));
  }

 private:
  static void InitLarecompLogging() {
    static bool initialized = false;
    if (initialized) {
      return;
    }

    initialized = true;

    rex::LogConfig config;
    config.default_level = spdlog::level::debug;
    config.log_to_console = true;
    config.log_file = "debug_la.txt";

    config.console_pattern = "[%^%l%$] [%n] [t%t] %v";
    config.file_pattern =
        "[%Y-%m-%d %H:%M:%S.%e] [%l] [%n] [t%t] %v";

    config.flush_level = spdlog::level::warn;

    config.category_levels["core"] = spdlog::level::debug;
    config.category_levels["cpu"] = spdlog::level::info;
    config.category_levels["gpu"] = spdlog::level::info;
    config.category_levels["krnl"] = spdlog::level::debug;
    config.category_levels["larecomp.app"] = spdlog::level::debug;
    config.category_levels["larecomp.crash"] = spdlog::level::trace;

    rex::InitLogging(config);
    rex::RegisterLogLevelCallback();

    spdlog::flush_on(spdlog::level::warn);
    spdlog::flush_every(std::chrono::seconds(1));
  }

#if defined(_WIN32)
  static void InstallCrashLogger() {
    static bool installed = false;
    if (installed) {
      return;
    }

    installed = true;
    SetUnhandledExceptionFilter(&LarecompApp::CrashHandler);
  }

  static LONG WINAPI CrashHandler(EXCEPTION_POINTERS* ep) {
    if (!ep || !ep->ExceptionRecord || !ep->ContextRecord) {
      return EXCEPTION_CONTINUE_SEARCH;
    }

    LARECOMP_CRASH_ERROR("!!! CRASH DETECTED !!!");
    LARECOMP_CRASH_ERROR("Codigo: 0x{:08X} no endereco 0x{:016X}",
                         static_cast<unsigned>(ep->ExceptionRecord->ExceptionCode),
                         reinterpret_cast<std::uintptr_t>(
                             ep->ExceptionRecord->ExceptionAddress));

    spdlog::apply_all([](std::shared_ptr<spdlog::logger> logger) {
      if (logger) {
        logger->flush();
      }
    });

    return EXCEPTION_CONTINUE_SEARCH;
  }
#else
  static void InstallCrashLogger() {}
#endif

static std::filesystem::path ExeDir() {
#if defined(_WIN32)
    wchar_t buffer[MAX_PATH]{};
    const DWORD len = GetModuleFileNameW(nullptr, buffer, MAX_PATH);

    if (len > 0 && len < MAX_PATH) {
        return std::filesystem::path(buffer).parent_path();
    }
#endif

    return std::filesystem::current_path();
}
};