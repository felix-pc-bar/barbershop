#pragma once

#include "general3d.h"

extern float globfpsTarget;
extern float globFOVrads;
extern Position3d globLightNormal;
extern int globScreenwidth;
extern int globScreenheight;
extern bool globDrawPoints;
extern bool globWireframe;
extern bool globDeferGFXcreation; // if true, only init SDL, renderers etc at last moment (allow terminal only mode in gameloop)
