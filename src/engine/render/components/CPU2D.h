#pragma once

#include <cstdint>
#include <vector>
#include <SDL_render.h>

#include "../../general2d.h"
#include "../../buffer.h"
#include "bmpfont.h"

class Hairline{
public:
	Hairline(int width, int height, std::vector<uint32_t>* screenbuffer); //constructor

	void SetPixel(int x, int y, uint32_t color);
	void SetBox(int xPos, int yPos, int size, uint32_t color);

	void drawPoint(Point2d pt, int sizePx);
	void drawLine(Point2d p1, Point2d p2, uint32_t col, int stroke);
	void drawText(Point2d position, std::string text);
	void transformPixelBuffer(pixelBuffer* buf, int dx = 0, int dy = 0, int scaling = 1, bool pixelBorders = false); // takes a pixelbuffer and maps it with transform to hairline output
	std::vector<uint32_t>* bufMain; // Shaded pixel buffer
	int width;
	int height;
	bmpFont* backupFont;
};
