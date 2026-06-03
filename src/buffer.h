#pragma once

#include <vector>

class pixelBuffer
{
public:
	int width;
	std::vector<short int> values;

	pixelBuffer(int _width, int _height, short int fill = -1);
	void set(int x, int y, short int value);
};
