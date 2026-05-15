#include <rex/cvar.h>
#include <rex/ppc/types.h>
#include <rex/system/kernel_state.h>
#include <chrono>
#include <cstdint>
#include <cmath>
#include <cstring>
#include <rex/ui/imgui_dialog.h>
#include "imgui.h"
#include "hooks.h"
#include "rex_macros.h"

// CVAR DEFINITIONS (Will appear in F4 menu)
// The '.lifecycle(kRequiresRestart)' forces the user to restart the game if they change the value.

REXCVAR_DEFINE_BOOL(skip_intro, false, "MCLA/Patches", "Skip the intro videos to prevent graphical issues.")
    .lifecycle(rex::cvar::Lifecycle::kRequiresRestart);

REXCVAR_DEFINE_BOOL(fps_60, false, "MCLA/Patches", "Increases vsync target to 60 FPS and enables deltatime.")
    .lifecycle(rex::cvar::Lifecycle::kHotReload);

REXCVAR_DEFINE_BOOL(disable_motion_blur, false, "MCLA/Patches", "Disable Motion Blur completely.")
    .lifecycle(rex::cvar::Lifecycle::kRequiresRestart);

REXCVAR_DEFINE_BOOL(disable_imposter_shadows, false, "MCLA/Patches", "Performance Mode: Foliage won't cast shadows.")
    .lifecycle(rex::cvar::Lifecycle::kRequiresRestart);

REXCVAR_DEFINE_BOOL(disable_msaa, false, "MCLA/Patches", "Disable Anti-Aliasing (MSAA).")
    .lifecycle(rex::cvar::Lifecycle::kRequiresRestart);

REXCVAR_DEFINE_BOOL(break_pairwise_collision, false, "MCLA/Patches", "Disables pairwise collision resolution.")
    .lifecycle(rex::cvar::Lifecycle::kRequiresRestart);

REXCVAR_DEFINE_BOOL(disable_rubberbanding, false, "MCLA/Patches", "Disables the AI RubberBand system, keeping the race fair.")
    .lifecycle(rex::cvar::Lifecycle::kRequiresRestart);

REXCVAR_DEFINE_BOOL(dbg_print, false, "MCLA/Patches", "Enable DbgPrint console outputs.")
    .lifecycle(rex::cvar::Lifecycle::kRequiresRestart);

REXCVAR_DEFINE_BOOL(physics_noclip, true, "MCLA/Physics", "Disable CCD/Pairwise Collision (Noclip)")
    .lifecycle(rex::cvar::Lifecycle::kHotReload);

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

bool Patch_DisableRubberBanding() {
    return REXCVAR_GET(disable_rubberbanding);
}
