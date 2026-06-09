#include "globals.h"
#include "general3d.h"

float globfpsTarget = 30.0f;
float globFOVrads = pi / 1.5f;
bool globDrawPoints = false; //whether to draw the (ground plane?) points
bool globWireframe = false; // Toggle between shaded and wireframe

bool globDeferGFXcreation = true;

Position3d globLightNormal = Position3d(3.0f, 5.0f, 1.0f).normalise();


int globScreenheight = 1080;
int globScreenwidth = 1920;

/*
1920x1080 /1
960x540 /2 
640x360 /3 
480x270 /4 
385x216 /5 
320x180 /6 
240x135 /8 
192x108 /10
*/
