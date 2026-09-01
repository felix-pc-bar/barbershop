#pragma once

#include "general3d.h"
#include <filesystem>

extern float globfpsTarget;
extern float globFOVrads;
extern Position3d globLightNormal;
extern int globScreenwidth;
extern int globScreenheight;
extern int globIntScaling; // screen integer scaling value
extern bool globDrawPoints;
extern bool globWireframe;
extern bool globDeferGFXcreation; // if true, only init SDL, renderers etc at last moment (allow terminal only mode in gameloop)
extern bool globPrintFPS;
// TODO: make barbershop/font-ed or whatever subdir be created automatically based on an app name of some description
extern std::filesystem::path globAppdatalocation; // filepath for appdata, should be crossplatform, "" if it couldn't be generated
