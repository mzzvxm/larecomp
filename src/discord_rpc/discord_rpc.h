#pragma once

#include <cstdint>
#include <string_view>

enum class LarecompDiscordState : uint32_t {
  Boot = 0,
  Loading,
  MainMenu,
  Garage,
  FreeRoam,
  Race,
  Pause,
  Credits,
  Unknown,
};

void LARECOMP_Discord_Init();
void LARECOMP_Discord_Shutdown();

void LARECOMP_Discord_SetState(LarecompDiscordState state);
void LARECOMP_Discord_SetStateText(LarecompDiscordState state,
                                   std::string_view details,
                                   std::string_view state_text);

void LARECOMP_Discord_SetLoading();
void LARECOMP_Discord_SetMainMenu();
void LARECOMP_Discord_SetGarage();
void LARECOMP_Discord_SetFreeRoam();
void LARECOMP_Discord_SetRace();
void LARECOMP_Discord_SetPause();
void LARECOMP_Discord_SetCredits();
