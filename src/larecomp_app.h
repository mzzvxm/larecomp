#pragma once

#include <rex/rex_app.h>
#include <rex/system/flags.h>
#include "mc_engine/threading.h"
#include "spdlog/spdlog_console.h"
#include "mc_engine/logging.h"
#include "spdlog/crash_handler.h"

#include <cstdint>
#include <memory>
#include <string_view>
#include <filesystem>

extern uint8_t* g_guest_mem;

class LarecompApp : public rex::ReXApp {
 public:
  LarecompApp(rex::ui::WindowedAppContext& ctx,
              std::string_view name,
              rex::PPCImageInfo ppc_info)
      : rex::ReXApp(ctx, name, ppc_info) {
    InitLarecompLogging();
    MC_INFO("LA Recompiled - @by mzzvxm");
    InstallCrashLogger();
    if (auto* w = window()) {
      w->SetTitle("Midnight Club LosAngeles - by @mzzvxm");
    }
  }

  static std::unique_ptr<rex::ui::WindowedApp> Create(
      rex::ui::WindowedAppContext& ctx) {
    return std::unique_ptr<LarecompApp>(
        new LarecompApp(ctx, "larecomp", PPCImageConfig));
  }

  void OnShutdown() override { mc::DisableHighResTimer(); }

  void OnConfigurePaths(rex::PathConfig& paths) override {
    const auto root = ExeDir();
    const auto default_game_data = root / "assets";

    // If the user has manually passed --game_data_root and it is valid,
    // keeps the manual path.
    if (!paths.game_data_root.empty() &&
        std::filesystem::exists(paths.game_data_root / "default.xex")) {
      return;
    }

    paths.game_data_root = default_game_data;

    const auto update = root / "update";
    if (std::filesystem::exists(update)) {
      paths.update_data_root = update;
    }
  }

  void OnPostSetup() override {
    MC_INFO("by @mzzvxm. base memory: 0x{:016X}",
                      reinterpret_cast<std::uintptr_t>(g_guest_mem));
  }
};
