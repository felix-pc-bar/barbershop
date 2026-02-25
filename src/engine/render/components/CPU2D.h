#pragma once

#include <cstdint>
#include <vector>
#include <SDL_render.h>

#include "../../general3d.h"
#include "../../general2d.h"

class Hairline{
public:
	Hairline(int width, int height, std::vector<uint32_t>* screenbuffer); //constructor

	void SetPixel(int x, int y, uint32_t color);
	void drawPoint(Point2d pt, int sizePx);
	void drawLine(Point2d p1, Point2d p2, uint32_t col, int stroke = 1);

	std::vector<uint32_t>* bufMain; // Shaded pixel buffer
	int width;
	int height;
};