#include <vector>
#include <cstdint>
#include <algorithm>
#include <cmath>
#include <SDL_render.h>

#include "CPU2D.h"
#include "../../general3d.h"
#include "../../general2d.h"

Hairline::Hairline(int width, int height, std::vector<uint32_t>* screenbuffer) //constructor
{
	this->width = width;
	this->height = height;
	this->bufMain = screenbuffer;
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
	if (p2.x - p1.x >= p2.y - p1.y)
	{
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

inline void Hairline::SetPixel(int x, int y, uint32_t color)
{
	int screenY = this->height - y;
	if ((unsigned)x >= (unsigned)width || (unsigned)screenY >= (unsigned)height)
		return;

	uint32_t& dest = (*bufMain)[screenY * width + x];

	uint8_t srcA = color >> 24;
	if (srcA == 255) {
		// Fully opaque: just write
		dest = color;
		return;
	}
	if (srcA == 0) {
		// Fully transparent: do nothing
		return;
	}
	// Extract source color components
	uint8_t srcR = (color >> 16) & 0xFF;
	uint8_t srcG = (color >> 8) & 0xFF;
	uint8_t srcB = color & 0xFF;

	// Extract destination color components
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
