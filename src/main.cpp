// larecomp - ReXGlue Recompiled Project
//
// This file is yours to edit. 'rexglue migrate' will NOT overwrite it.

#include "generated/larecomp_init.h"

#include "larecomp_app.h"

#include <rex/cvar.h>

#include <cmath>
#include <cstdint>

uint8_t* g_guest_mem = nullptr;

extern "C" float roundevenf(float x) {
    return std::nearbyint(x);
}

extern "C" double roundeven(double x) {
    return std::nearbyint(x);
}

REX_DEFINE_APP(larecomp, LarecompApp::Create)