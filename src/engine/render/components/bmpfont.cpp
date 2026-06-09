#include "bmpfont.h"

bmpFont::bmpFont()
{
    for (auto ptr: this->glyphs)
    {
        ptr = new bmpGlyph();
    }
}