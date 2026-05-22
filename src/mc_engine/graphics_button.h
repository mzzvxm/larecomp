#pragma once

#include <cstdint>

#include <rex/ppc.h>

namespace mc::ui {

// Init called from LarecompApp::OnPostSetup().
void InitGraphicsButtonPatch();

// Opens the ReXGlue/F4 overlay using the PC-side window path.
void RequestOpenRexGraphicsMenu();

// Current research/probe hooks.
void Hook_ProbeMcUIPauseMenuCtor(PPCRegister& r3);
void Hook_ProbeMcUIPauseCtor(PPCRegister& r3);
void Hook_ProbeActionIntern(PPCRegister& r3);
void Hook_ProbeMenuActionAppend(PPCRegister& r3, PPCRegister& r4);
void Hook_ProbePauseActionHandler(PPCRegister& r3, PPCRegister& r4);

// Generic memory/object probe, useful while reversing layouts.
void DebugProbeMenuObject(uint32_t guest_this);

} // namespace mc::ui