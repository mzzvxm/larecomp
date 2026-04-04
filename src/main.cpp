
// larecomp - ReXGlue Recompiled Project
//
// This file is yours to edit. 'rexglue migrate' will NOT overwrite it.

#include "generated/larecomp_config.h"
#include "generated/larecomp_init.h"

#include "larecomp_app.h"

#include <cmath>

// Ponteiro para a memória base do X360 (definido no núcleo do rexglue)
uint8_t* g_guest_mem = nullptr;

extern "C" float roundevenf(float x) {
    return std::nearbyint(x);
}

extern "C" double roundeven(double x) {
    return std::nearbyint(x);
}

REX_DEFINE_APP(larecomp, LarecompApp::Create)
