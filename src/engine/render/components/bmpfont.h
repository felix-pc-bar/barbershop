#pragma once
#include "../../buffer.h"

#include <array>

class bmpGlyph
{
	pixelBuffer bitmap;
	// offsets for kerning etc
	int placementX;
	int placementY;
};

class bmpfont
{
public:
	// these are used for selection
	std::string name;
	uint sizepx;

	std::array<bmpGlyph*, 128> glyphs; // we only store the ascii set (for now)
};
