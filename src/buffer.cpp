#include "buffer.h"

pixelBuffer::pixelBuffer(int _width, int _height, short int fill)
{
	if (fill == -1)
	{
		for (long int i = 0; i < _width * _height; i++)
		{
			this->values.emplace_back(i % 2);
		}
	}
	else
	{
		this->width = _width;
		for (long int i = 0; i <= _width * _height; i++)
		{
			this->values.emplace_back(fill);
		}
	}
	this->width = _width;
}

void pixelBuffer::set(int x, int y, short int value)
{
	if (y * this->width + x < this->values.size() && x < this->width && x >= 0) { this->values[y * this->width + x] = value; }
	return;
}
