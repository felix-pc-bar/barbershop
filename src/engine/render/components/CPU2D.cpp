#include <vector>
#include <cstdint>
#include <algorithm>
#include <cmath>
#include <SDL_render.h>

#include "CPU2D.h"
#include "../../general2d.h"
#include "../../stubble/stubble.h"
// #include "../../stubble/types.h"
#include "bmpfont.h"

Hairline::Hairline(int width, int height, std::vector<uint32_t>* screenbuffer) //constructor
{
	this->width = width;
	this->height = height;
	this->bufMain = screenbuffer;
	StubbleParser* tempParser = new StubbleParser();
	// auto result = tempParser->import("content/defaultfont.stbbl", TypesEnum::_bmpFont);
	// if (result.has_value()) { this->backupFont = std::get<bmpFont*>(result.value()); }
}

void Hairline::transformPixelBuffer(pixelBuffer* buf, int dx, int dy, int scaling, bool pixelBorders, uint32_t outlineColour, uint32_t colour, uint32_t backgroundColour)
{
	Colour tmp(backgroundColour);
	if (tmp.alpha != 0)
	{
		for (int y = 0; ; y++)
		{
			for (int x = 0; x < buf->width; x++)
			{
				if (y * buf->width + static_cast<int>(x / scaling) >= buf->values.size()) { goto doneBuf; }
				uint32_t col = (buf->values[y * buf->width + x] == 1) ? colour : backgroundColour;
				this->SetBox(dx + (x * scaling),  dy + (y * scaling), scaling, col);
			}
		}
	}
	else
	{
		for (int y = 0; ; y++)
		{
			for (int x = 0; x < buf->width; x++)
			{
				if (y * buf->width + static_cast<int>(x / scaling) >= buf->values.size()) { goto doneBuf; }
				if (buf->values[y * buf->width + x] == 1)
				{
					this->SetBox(dx + (x * scaling),  dy + (y * scaling), scaling, colour);
				}
			}
		}
	}
doneBuf:
	if (pixelBorders)
	{
		// vert borders
		for (int x = dx; x <= dx + (buf->width * scaling); x += scaling)
		{
			this->drawLine({x, dy}, {x, dy + (buf->height() * scaling)}, 0xFF808080, 1);
		}
		// horizontal
		for (int y = dy; y <= dy + (buf->height() * scaling); y += scaling)
		{
			this->drawLine({dx, y}, {dx + (buf->width * scaling), y}, 0xFF808080, 1);
		}
	}
	if (Colour(outlineColour).alpha != 0)
	{
		int bl_x = dx - 1;
		int bl_y = dy - 1;
		int tr_x = dx + (buf->width * scaling);
		int tr_y = dy + (buf->height() * scaling);
		this->drawLine({bl_x, bl_y}, {tr_x, bl_y}, outlineColour, 1);
		this->drawLine({tr_x, bl_y}, {tr_x, tr_y}, outlineColour, 1);
		this->drawLine({tr_x, tr_y}, {bl_x, tr_y}, outlineColour, 1);
		this->drawLine({bl_x, tr_y}, {bl_x, bl_y}, outlineColour, 1);
	}
	return;
}

void Hairline::drawRectangle(Point2d botLeft, Point2d topRight, uint32_t colour)
{
	for (int x = botLeft.x; x < topRight.x; x++)
	{
		for (int y = botLeft.y; y < topRight.y; y++)
		{
			SetPixel(x, y, colour);
		}
	}
	return;
}

void Hairline::SetBox(int xPos, int yPos, int size, uint32_t colour)
{
	if (xPos < -size || xPos > this->width || yPos < -size || yPos > this->height) { return; }
	if (size == 1) { SetPixel(xPos, yPos, colour); return; }
	for (int y = 0; y < size; y++)
	{
		for (int x = 0; x < size; x++)
		{
			SetPixel(xPos + x, yPos + y, colour);
		}
	}
	return;
}

void Hairline::drawPoint(Point2d pt, int sizePx)
{
	int offset = std::floor(sizePx / 2.0f);
	for (int x = pt.x - offset; x < pt.x + offset + sizePx % 2; x++)
	{
		for (int y = pt.y - offset; y < pt.y + offset + sizePx % 2; y++)
		{
			if (x - pt.x == 0 || y - pt.y == 0 || sizePx != 3)
			{
				this->SetPixel(x, y, 0xFF808080);
			}
		}
	}
	return;
}

void Hairline::drawLine(Point2d p1, Point2d p2, uint32_t col, int stroke)
{
	if (p1.x == p2.x && p1.y == p2.y)
	{
		this->SetPixel(p1.x, p1.y, col);
		return;
	}
	if (abs(p2.x - p1.x) >= abs(p2.y - p1.y))
	{
		if (p1.x > p2.x)
		{
			auto tmp = p1;
			p1 = p2;
			p2 = tmp;
		}
		float yStep = 0.0f;
		if (p2.x - p1.x != 0)
		{
			yStep = (float)(p2.y - p1.y) / (float)(p2.x - p1.x);
		}
		for (int i = 0; i <= p2.x - p1.x; i++)
		{
			this->SetPixel(p1.x + i, (int)(p1.y + (yStep * i)), col);
		}
	}
	else
	{
		if (p1.y > p2.y)
		{
			auto tmp = p1;
			p1 = p2;
			p2 = tmp;
		}
		float xStep = 0.0f;
		if (p2.y - p1.y != 0)
		{
			xStep = (float)(p2.x - p1.x) / (float)(p2.y - p1.y); 
		}
		for (int i = 0; i <= p2.y - p1.y; i++)
		{
			this->SetPixel((int)(p1.x + (xStep * i)), p1.y + i, col);
		}
	}
	return;
}

void Hairline::drawText(Point2d position, std::string text, bmpFont* font, int scaling, uint32_t colour)
{
	if (font == nullptr) { font = this->backupFont; }
	int xoffset = 0;
	for (int i = 0; i < text.size(); i++)
	{
		bmpGlyph* currentGlyph = font->getChar(text.at(i));
		transformPixelBuffer(currentGlyph->bitmap, position.x + xoffset + ((font->defaultKerning + currentGlyph->placementX) * scaling), position.y + (currentGlyph->placementY * scaling), scaling, false, 0x00000000, colour, 0x00000000);
		xoffset += (currentGlyph->bitmap->width + font->defaultKerning + currentGlyph->placementX) * scaling;
	}
}

inline void Hairline::SetPixel(int x, int y, uint32_t colour)
{
	int screenY = this->height - y;
	if ((unsigned)x >= (unsigned)width || (unsigned)screenY >= (unsigned)height)
		return;

	uint32_t& dest = (*bufMain)[screenY * width + x];

	uint8_t srcA = colour >> 24;
	if (srcA == 255) {
		// Fully opaque: just write
		dest = colour;
		return;
	}
	if (srcA == 0) {
		// Fully transparent: do nothing
		return;
	}
	// Extract source colour components
	uint8_t srcR = (colour >> 16) & 0xFF;
	uint8_t srcG = (colour >> 8) & 0xFF;
	uint8_t srcB = colour & 0xFF;

	// Extract destination colour components
	uint8_t dstR = (dest >> 16) & 0xFF;
	uint8_t dstG = (dest >> 8) & 0xFF;
	uint8_t dstB = dest & 0xFF;

	// Blend (non-premultiplied alpha)
	uint8_t outR = (srcR * srcA + dstR * (255 - srcA)) / 255;
	uint8_t outG = (srcG * srcA + dstG * (255 - srcA)) / 255;
	uint8_t outB = (srcB * srcA + dstB * (255 - srcA)) / 255;

	// Optionally blend alpha too � here we just preserve max of src/dst
	uint8_t outA = std::max(srcA, (uint8_t)(dest >> 24));

	// Repack
	dest = (outA << 24) | (outR << 16) | (outG << 8) | outB;
}
