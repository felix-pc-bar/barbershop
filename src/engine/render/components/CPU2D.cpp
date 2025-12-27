#include <iostream>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <limits>
#include <cmath>
#include <SDL_render.h>

#include "CPU2D.h"
#include "../../engTools.h"
#include "../../logic2d.h"
#include "../../engconfig.h"

CPU2D::CPU2D(SDL_Texture* screentex, SDL_Renderer* renderer, int width, int height, std::vector<uint32_t>* screenbuffer) //constructor
{
	texture = screentex;
	sdlRenderer = renderer;
	this->width = width;
	this->height = height;
	this->bufMain = screenbuffer;
}

void CPU2D::Clear(uint32_t color)
{
	std::fill(bufMain->begin(), bufMain->end(), color);
}
void CPU2D::Present()
{
	SDL_UpdateTexture(texture, nullptr, bufMain->data(), width * sizeof(uint32_t));
	SDL_RenderCopy(sdlRenderer, texture, nullptr, nullptr);
	SDL_RenderPresent(sdlRenderer);
}

void CPU2D::drawPoint(Position3d pos, int sizePx)
{
	Point2d point(pos);
	if (pos.cameraspace().z <= 0 || point.x == -99999) { return; }
	int offset = std::floor(sizePx / 2.0f);
	for (int x = point.x - offset; x < point.x + offset + sizePx % 2; x++)
	{
		for (int y = point.y - offset; y < point.y + offset + sizePx % 2; y++)
		{
			if (x - point.x == 0 || y - point.y == 0 || sizePx != 3)
			{
				this->SetPixel(x, y, 0xFF808080);
			}
		}
	}
	return;
}

inline void CPU2D::SetPixel(int x, int y, uint32_t color)
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

	// Optionally blend alpha too — here we just preserve max of src/dst
	uint8_t outA = std::max(srcA, (uint8_t)(dest >> 24));

	// Repack
	dest = (outA << 24) | (outR << 16) | (outG << 8) | outB;
}
