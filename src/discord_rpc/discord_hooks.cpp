#include "discord_rpc/discord_rpc.h"

#include <cstdio>

// -----------------------------------------------------------------------------
// Discord RPC hooks - no automatic FreeRoam version
// -----------------------------------------------------------------------------
//
// Why:
//   Runtime kept getting stuck as "Passeando por Los Angeles".
//   That means some FreeRoam hook is firing too early/repeatedly.
//
// This version intentionally has NO hook that calls:
//   LARECOMP_Discord_SetFreeRoam()
//
// It only detects:
//   - Garage
//   - Race
//   - Credits
//
// Once these are confirmed, we can add FreeRoam again using a safer transition.
// -----------------------------------------------------------------------------

namespace {

void RpcHookLog(const char* name) {
  std::printf("[LARECOMP RPC HOOK] %s\n", name);
}

}  // namespace

// -----------------------------------------------------------------------------
// Garage
// -----------------------------------------------------------------------------
//
// Strongest garage candidate from the latest report:
//   0x82255E08 is a large garage state/UI writer function.
//   It references GARAGEMOVIE, FullGarageError, GarageHydraulics,
//   GarageAirbags, etc., and writes garage-related globals.
//
// Secondary script/native candidates:
//   0x8238F608 is linked to Garage_TransitionIntoGarage,
//   Garage_TransitionOutOfGarage, Garage_TransitionToTestDrive,
//   Garage_WarpTo, etc.
// -----------------------------------------------------------------------------

void RpcHook_GarageMain_82255E08() {
  RpcHookLog("GarageMain 0x82255E08");
  LARECOMP_Discord_SetRace();
}

void RpcHook_GarageTransitionNative_8238F608() {
  RpcHookLog("GarageTransitionNative 0x8238F608");
  LARECOMP_Discord_SetGarage();
}

// -----------------------------------------------------------------------------
// Race
// -----------------------------------------------------------------------------
//
// The latest report's strongest race candidates are checkpoint/race-description
// natives. These may only fire once a race actually initializes or updates,
// which is better than the earlier RACE_EVENT string-xrefs.
//
// Start with checkpoint manager functions:
//   0x8231B808
//   0x8231B738
//   0x8231B848
//   0x8231B4C0
//
// If these prove too noisy, keep only the one that fires during race.
// -----------------------------------------------------------------------------

void RpcHook_RaceCheckpointCalc_8231B808() {
  RpcHookLog("RaceCheckpointCalc 0x8231B808");
  LARECOMP_Discord_SetRace();
}

void RpcHook_RaceCheckpointFind_8231B738() {
  RpcHookLog("RaceCheckpointFind 0x8231B738");
  LARECOMP_Discord_SetRace();
}

void RpcHook_RaceCheckpointOrdered_8231B848() {
  RpcHookLog("RaceCheckpointOrdered 0x8231B848");
  LARECOMP_Discord_SetRace();
}

void RpcHook_RaceCheckpointGet_8231B4C0() {
  RpcHookLog("RaceCheckpointGet 0x8231B4C0");
  LARECOMP_Discord_SetRace();
}

// Race results screen candidate. This should NOT set FreeRoam in this version.
// Instead, it keeps Race until we have a safe FreeRoam signal.
void RpcHook_RaceResults_822729A8() {
  RpcHookLog("RaceResults 0x822729A8");
  LARECOMP_Discord_SetRace();
}
