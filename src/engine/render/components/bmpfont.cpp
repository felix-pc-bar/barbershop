#include "bmpfont.h"

bmpGlyph::bmpGlyph(int width, int height)
{
	this->bitmap = new pixelBuffer(width, height);
	this->isPrintable = false;
}

bmpFont::bmpFont()
{
	for (auto ptr: this->glyphs)
	{
		ptr = new bmpGlyph(1, 1);
	}
	this->defaultKerning = 2;
}

bmpFont::bmpFont(int width, int height)
{
	for (int i = 0; i < this->glyphs.size(); i++)
	{
		this->glyphs[i] = new bmpGlyph(width, height);
	}
	this->defaultKerning = 2;
}

bmpGlyph* bmpFont::getChar(char c)
{
	if (this->glyphs[c]->bitmap != nullptr && this->glyphs[c]->isPrintable)
	{
		return this->glyphs[c];
	}
	return this->glyphs[0]; // ASCII `0` is NULL. We use it to store the fallback glyph
}
