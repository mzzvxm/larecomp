#pragma once

void InitHooks();

#include <rex/ppc/context.h>
bool Patch_AspectRatio_82233EB4(PPCRegister& f0);
bool Patch_AspectRatio_82214BB8(PPCRegister& f10);
bool Patch_AspectRatio_822E5E68(PPCRegister& f12);
bool Patch_AspectRatio_8223E5E0(PPCRegister& f13);

bool Patch_DisableDoF();

inline double fpsCount;
inline bool showfps;
