#pragma once

#include <cstdint>
#include <vector>
#include <SDL_render.h>

#include "../../engTools.h"
#include "../../logic2d.h"

class CPU2D{
public:
	CPU2D(SDL_Texture* screentex, SDL_Renderer* renderer, int width, int height, std::vector<uint32_t>* screenbuffer); //constructor

	void Clear(uint32_t color);
	void Present(); // push pixels to texture and draw to screen
	void SetPixel(int x, int y, uint32_t color);
	void drawPoint(Point2d pt, int sizePx);
	void drawLine(Point2d p1, Point2d p2, int stroke = 1);
	SDL_Renderer* sdlRenderer;
	SDL_Texture* texture;
	std::vector<uint32_t>* bufMain; // Shaded pixel buffer
	int width;
	int height;
};