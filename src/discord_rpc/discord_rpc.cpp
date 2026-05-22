#include "discord_rpc/discord_rpc.h"

#include <atomic>
#include <mutex>
#include <string>

#include <rex/cvar.h>
#include <rex/discord_rpc.h>

// -----------------------------------------------------------------------------
// Notes
// -----------------------------------------------------------------------------
//
// This module assumes your ReXGlue SDK has the Discord RPC PR API:
//
//   rex::discord_rpc::Presence
//   rex::discord_rpc::Start(application_id, presence)
//   rex::discord_rpc::SetDetails(text)
//   rex::discord_rpc::SetState(text)
//   rex::discord_rpc::Stop()
//
// If your local copy of the PR uses slightly different names, keep this module
// and only adapt the calls inside:
//   LARECOMP_Discord_Init()
//   LARECOMP_Discord_Shutdown()
//   LARECOMP_Discord_SetStateText()
//
// -----------------------------------------------------------------------------

REXCVAR_DEFINE_BOOL(larecomp_discord_rpc, true, "Discord",
                    "Enable Discord Rich Presence.")
    .lifecycle(rex::cvar::Lifecycle::kHotReload);

REXCVAR_DEFINE_BOOL(larecomp_discord_rpc_log_state, true, "Discord",
                    "Log Discord RPC state changes with printf.")
    .lifecycle(rex::cvar::Lifecycle::kHotReload);

REXCVAR_DEFINE_STRING(larecomp_discord_large_image, "mcla_logo", "Discord",
                      "Discord RPC large image key.")
    .lifecycle(rex::cvar::Lifecycle::kHotReload);

namespace {

std::mutex g_rpc_mutex;
std::atomic_bool g_rpc_started{false};

LarecompDiscordState g_current_state = LarecompDiscordState::Unknown;
std::string g_current_details;
std::string g_current_state_text;

constexpr const char* kDiscordApplicationId = "1503923771264729309";

const char* DetailsForState(LarecompDiscordState state) {
  switch (state) {
    case LarecompDiscordState::Boot:
      return "Inicializando";
    case LarecompDiscordState::Loading:
      return "Carregando";
    case LarecompDiscordState::MainMenu:
      return "No menu principal";
    case LarecompDiscordState::Garage:
      return "Na garagem";
    case LarecompDiscordState::FreeRoam:
      return "Passeando por Los Angeles";
    case LarecompDiscordState::Race:
      return "Em corrida";
    case LarecompDiscordState::Pause:
      return "Pausado";
    case LarecompDiscordState::Credits:
      return "Vendo os créditos";
    default:
      return "Jogando";
  }
}

const char* StateTextForState(LarecompDiscordState state) {
  switch (state) {
    case LarecompDiscordState::Boot:
      return "Abrindo LA Recomp";
    case LarecompDiscordState::Loading:
      return "Carregando sessão";
    case LarecompDiscordState::MainMenu:
      return "Escolhendo modo de jogo";
    case LarecompDiscordState::Garage:
      return "Customizando o carro";
    case LarecompDiscordState::FreeRoam:
      return "Cruise / Free Roam";
    case LarecompDiscordState::Race:
      return "Corrida ativa";
    case LarecompDiscordState::Pause:
      return "Menu de pausa";
    case LarecompDiscordState::Credits:
      return "Credits";
    default:
      return "LA Recomp";
  }
}

void LogStateChange(const std::string& details, const std::string& state_text) {
  if (!REXCVAR_GET(larecomp_discord_rpc_log_state)) {
    return;
  }

  // Deliberately avoids ReX logging category dependencies.
  // Swap this to REXLOG_INFO if you prefer.
  std::printf("[LARECOMP DISCORD RPC] details='%s' state='%s'\n",
              details.c_str(),
              state_text.c_str());
}

}  // namespace

void LARECOMP_Discord_Init() {
  if (!REXCVAR_GET(larecomp_discord_rpc)) {
    return;
  }

  std::lock_guard<std::mutex> lock(g_rpc_mutex);

  if (g_rpc_started.load()) {
    return;
  }

  rex::discord_rpc::Presence rpc;

  rpc.details_ = "";
  rpc.state_ = "";
  rpc.large_image_key_ = REXCVAR_GET(larecomp_discord_large_image);
  rpc.large_image_text_ = "LARecomp";

  rex::discord_rpc::Start(kDiscordApplicationId, rpc);

  g_rpc_started.store(true);
  g_current_state = LarecompDiscordState::Boot;
  g_current_details = rpc.details_;
  g_current_state_text = rpc.state_;

  LogStateChange(g_current_details, g_current_state_text);
}

void LARECOMP_Discord_Shutdown() {
  if (!g_rpc_started.load()) {
    return;
  }

  // If your SDK copy does not expose Stop(), comment this line.
  rex::discord_rpc::Stop();

  g_rpc_started.store(false);
}

void LARECOMP_Discord_SetStateText(LarecompDiscordState state,
                                   std::string_view details,
                                   std::string_view state_text) {
  if (!REXCVAR_GET(larecomp_discord_rpc)) {
    return;
  }

  if (!g_rpc_started.load()) {
    return;
  }

  std::lock_guard<std::mutex> lock(g_rpc_mutex);

  const std::string new_details(details);
  const std::string new_state_text(state_text);

  // Prevent hook spam from hammering Discord if a hook runs many times.
  if (g_current_state == state &&
      g_current_details == new_details &&
      g_current_state_text == new_state_text) {
    return;
  }

  g_current_state = state;
  g_current_details = new_details;
  g_current_state_text = new_state_text;

  rex::discord_rpc::SetDetails(g_current_details);
  rex::discord_rpc::SetState(g_current_state_text);

  LogStateChange(g_current_details, g_current_state_text);
}

void LARECOMP_Discord_SetState(LarecompDiscordState state) {
  LARECOMP_Discord_SetStateText(
      state,
      DetailsForState(state),
      StateTextForState(state));
}

void LARECOMP_Discord_SetLoading() {
  LARECOMP_Discord_SetState(LarecompDiscordState::Loading);
}

void LARECOMP_Discord_SetMainMenu() {
  LARECOMP_Discord_SetState(LarecompDiscordState::MainMenu);
}

void LARECOMP_Discord_SetGarage() {
  LARECOMP_Discord_SetState(LarecompDiscordState::Garage);
}

void LARECOMP_Discord_SetFreeRoam() {
  LARECOMP_Discord_SetState(LarecompDiscordState::FreeRoam);
}

void LARECOMP_Discord_SetRace() {
  LARECOMP_Discord_SetState(LarecompDiscordState::Race);
}

void LARECOMP_Discord_SetPause() {
  LARECOMP_Discord_SetState(LarecompDiscordState::Pause);
}

void LARECOMP_Discord_SetCredits() {
  LARECOMP_Discord_SetState(LarecompDiscordState::Credits);
}
