#pragma once
#include "types.h"

#include "../material.h"
#include <vector>

Colour* _colRGB(std::vector<extendedValue> args); // R,G,B (float)
Colour* _colRGBA(std::vector<extendedValue> args); // R,G,B,A (float)

Material* _matColPtShd(std::vector<extendedValue> args); // Material from Colour, Point size (int), shade? (bool)

Object3D* _objPathpp(std::vector<extendedValue> args); // Object3D from Wavefront filepath (std::string), Material

Position3d* _posXYZf(std::vector<extendedValue> args); // Position3d from x, y, z (floats)

Quaternion* _quatAngleAxis(std::vector<extendedValue> args); // Quaternion from angle (float; rads), axis around rotation is made (Position3d as vector)

Camera* _camPosQuatFov(std::vector<extendedValue> args); // Camera from Position3d, Quaternion, FoV (float)

Scene* _scnFull(std::vector<extendedValue> args); // Scene from vector of Cameras, of Objects
