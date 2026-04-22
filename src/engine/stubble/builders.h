#pragma once
#include "types.h"

#include "../material.h"

Colour* _colRGB(std::vector<extendedValue> args);
Colour* _colRGBA(std::vector<extendedValue> args);

Material* _matColPtShd(std::vector<extendedValue> args);
