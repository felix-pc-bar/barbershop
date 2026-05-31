#include "buffer.h"

pixelBuffer::pixelBuffer(int _width, int _height, short int fill)
{
	for (long int i = 0; i <= _width * _height; i++)
	{
		this->values.emplace_back(fill);
	}
	this->width = _width;
}
