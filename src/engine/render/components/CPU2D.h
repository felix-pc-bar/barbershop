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

	void SetPixel(int x, int y, uint32_t colour);
	void SetBox(int xPos, int yPos, int size, uint32_t colour);

	void drawPoint(Point2d pt, int sizePx);
	void drawLine(Point2d p1, Point2d p2, uint32_t col, int stroke);
	void drawRectangle(Point2d botLeft, Point2d topRight, uint32_t colour); // untested...
	void drawText(Point2d position, std::string text, bmpFont* font = nullptr, int scaling = 1, uint32_t colour = 0xFFFFFFFF);
	void transformPixelBuffer(pixelBuffer* buf, int dx = 0, int dy = 0, int scaling = 1, bool pixelBorders = false, uint32_t outlineColour = 0x00000000, uint32_t colour = 0xFFFFFFFF, uint32_t backgroundColour = 0xFF000000); // takes a pixelbuffer and maps it with transform to hairline output

	std::vector<uint32_t>* bufMain; // Shaded pixel buffer
	int width;
	int height;
	bmpFont* backupFont;
};
