#include "globals.h"
#include "engTools.h"

float globfpsTarget = 30.0f;
float globFOVrads = pi / 1.5f;
bool globDrawPoints = false; //whether to draw the (ground plane?) points
bool globWireframe = false; // Toggle between shaded and wireframe

Position3d globLightNormal = Position3d(3.0f, 5.0f, 1.0f).normalise();

//int screenheight = 360;
//int screenwidth = 640;
int globScreenheight = 540;
int globScreenwidth = 960;
//int screenheight = 1080;
 //int screenwidth = 1920;

/*
1920x1080
960x540
640x360
480x270
385x216
320x180
240x135
192x108
*/
