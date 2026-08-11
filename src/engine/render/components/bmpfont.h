#pragma once
#include "../../buffer.h"

#include <array>

class bmpGlyph
{
public:
	bmpGlyph() = default;
	bmpGlyph(int width, int height);

	pixelBuffer* bitmap;
	// offsets for kerning etc
	int placementX;
	int placementY;
	bool isPrintable;
};

class bmpFont
{
public:
	bmpFont();
	bmpFont(int width, int height);
	// these are used for selection
	std::string name;
	int sizepx; // This is just used to categorise and find fonts; "get me a 10px size font for this draw" etc
	int defaultKerning; // number of pixels to space characters by by default

	std::array<bmpGlyph*, 128> glyphs; // we only store the ascii set (for now)
	bmpGlyph* getChar(char c); // fetch the bmpGlyph for a given character
};
