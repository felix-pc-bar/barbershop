#pragma once

#include "general3d.h"

extern float globfpsTarget;
extern float globFOVrads;
extern Position3d globLightNormal;
extern int globScreenwidth;
extern int globScreenheight;
extern bool globDrawPoints;
extern bool globWireframe;

// set te screen dimensions to those reported by SDL, in ordered for 
// proper integer scaling to be achieved.
void setScreenDimensions();
