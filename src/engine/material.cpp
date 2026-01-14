#include <iostream>

#include "material.h"

Material::Material()
{
	this->colour = { 1,0,1 }; //Magenta
	this->omitDbg = false;
	this->pointWidth = 0;
	this->shadeMat = true;
	this->ditherValue = 0.0f;
}

Material::Material(Colour col, int pointSize, bool shade, bool allowDebugVis, uint16_t ditherVal)
{
	this->colour = col;
	this->pointWidth = pointSize;
	this->shadeMat = shade;
	this->omitDbg = allowDebugVis;
	this->ditherValue = ditherVal;
}


Colour::Colour(float r, float g, float b) : red(r), green(g), blue(b), alpha(1){}

Colour::Colour(float r, float g, float b, float a) : red(r), green(g), blue(b), alpha(a){}

Colour::Colour() : red(1), green(0), blue(1), alpha(1){}

Colour::Colour(const std::string& gruvName)
{
	auto result = gruvCols().find(gruvName);
	if (result == gruvCols().end())
	{
		std::cout << "Error: could not find gruv colour \"" << gruvName << "\"" << std::endl;
		this->alpha = 1;
		this->red = 1;
		this->green = 0;
		this->blue = 1;
	}
	*this = result->second;
}

Colour& Colour::operator*=(const float val)
{
	red *= val;
	green *= val;
	blue *= val;
	return *this;
}

Colour::Colour(uint32_t col)
{
	this->alpha = (col >> 24) & 0xFF;
	this->red = (col >> 16) & 0xFF;
	this->green = (col >> 8) & 0xFF;
	this->blue = col & 0xFF;

	alpha = alpha / 0xFF;
	red = red / 0xFF;
	green = green / 0xFF;
	blue = blue / 0xFF;
}

Colour operator*(const Colour& c1, const float val) 
{
	Colour cr(c1.red * val, c1.green * val, c1.blue * val, c1.alpha);
	return cr;
}

uint32_t Colour::raw() const
{
	return	((uint32_t)(0xFF * this->alpha) << 24) |
			((uint32_t)(0xFF * this->red) << 16) |
			((uint32_t)(0xFF * this->green) << 8)|
			((uint32_t)(0xFF * this->blue));
}