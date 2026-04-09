
// larecomp - ReXGlue Recompiled Project
//
// This file is yours to edit. 'rexglue migrate' will NOT overwrite it.

#include "generated/larecomp_config.h"
#include "generated/larecomp_init.h"

#include "larecomp_app.h"

#include <rex/cvar.h>

#include <cmath>

// Pointer to X360 base memory (defined in rexglue core)
uint8_t* g_guest_mem = nullptr;

extern "C" float roundevenf(float x) {
    return std::nearbyint(x);
}

extern "C" double roundeven(double x) {
    return std::nearbyint(x);
}

// CVAR DEFINITIONS (Will appear in F4 menu)
// The '.lifecycle(kRequiresRestart)' forces the user to restart the game if they change the value.


REXCVAR_DEFINE_BOOL(skip_intro, true, "Patches", "Skip the intro videos to prevent graphical issues.")
    .lifecycle(rex::cvar::Lifecycle::kRequiresRestart);

REXCVAR_DEFINE_BOOL(fps_60, false, "Patches", "Increases vsync target to 60 FPS and enables deltatime.")
    .lifecycle(rex::cvar::Lifecycle::kRequiresRestart);

REXCVAR_DEFINE_BOOL(disable_motion_blur, false, "Patches", "Disable Motion Blur completely.")
    .lifecycle(rex::cvar::Lifecycle::kRequiresRestart);

REXCVAR_DEFINE_BOOL(disable_imposter_shadows, false, "Patches", "Performance Mode: Foliage won't cast shadows.")
    .lifecycle(rex::cvar::Lifecycle::kRequiresRestart);

REXCVAR_DEFINE_BOOL(disable_msaa, false, "Patches", "Disable Anti-Aliasing (MSAA).")
    .lifecycle(rex::cvar::Lifecycle::kRequiresRestart);

REXCVAR_DEFINE_BOOL(dbg_print, false, "Patches", "Enable DbgPrint console outputs.")
    .lifecycle(rex::cvar::Lifecycle::kRequiresRestart);

REXCVAR_DEFINE_BOOL(break_pairwise_collision, false, "Patches", "Disables pairwise collision resolution.")
    .lifecycle(rex::cvar::Lifecycle::kRequiresRestart);


// HOOK FUNCTIONS (Called in the middle of translated Assembly execution)

bool SkipIntro() { 
    return REXCVAR_GET(skip_intro); 
}

bool Patch_60FPS_Jump() { 
    return REXCVAR_GET(fps_60); 
}

// 8-bit 60FPS patch. Assumes 'r3' by default, check in IDA.
bool Patch_60FPS_Byte(PPCRegister& r11) { 
    if (REXCVAR_GET(fps_60)) {
        r11.u64 = 1; // Replaces the original value with 1 (li r11, 1)
        return true; // Skips the original instruction
    }
    return false;
}

// The same hook works for both Motion Blur instructions!
bool Patch_DisableMotionBlur(PPCRegister& r3) {
    if (REXCVAR_GET(disable_motion_blur)) {
        r3.u64 = 0; // li r3, 0
        return true;
    }
    return false;
}

bool Patch_DisableMSAA(PPCRegister& r11) {
    if (REXCVAR_GET(disable_msaa)) {
        r11.u64 = 1; // li r11, 1
        return true;
    }
    return false;
}

// Returns 'true' to inject a 'blr' (return from collision function)
bool Patch_PhysicsCollision() {
    return REXCVAR_GET(break_pairwise_collision);
}

REX_DEFINE_APP(larecomp, LarecompApp::Create)