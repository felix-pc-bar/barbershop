#include "bmpfont.h"

bmpGlyph::bmpGlyph(int width, int height)
{
	this->bitmap = new pixelBuffer(width, height);
}

bmpFont::bmpFont()
{
	for (auto ptr: this->glyphs)
	{
		ptr = new bmpGlyph(1, 1);
	}
}

bmpFont::bmpFont(int width, int height)
{
	for (int i = 0; i < this->glyphs.size(); i++)
	{
		this->glyphs[i] = new bmpGlyph(width, height);
	}
}
