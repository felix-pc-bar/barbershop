#include <algorithm>
#include <functional>

#include "ui.h"
#include "general2d.h"
#include "render/render.h"

Point2d operator*(Point2d const& lhs, frac2d const& rhs)
{
	return {static_cast<int>(lhs.x * rhs.X), static_cast<int>(lhs.y * rhs.Y)};
}

LayoutElement::LayoutElement(std::string _name, frac2d _anchor, frac2d _relPos, frac2d _relSize, Point2d _offsetPx, Point2d _sizeOffsetPx)
	: name(_name),
	anchor(_anchor),
	relPos(_relPos),
	relSize(_relSize),
	offsetPx(_offsetPx),
	sizeOffsetPx(_sizeOffsetPx),
	isPoint(false),
	sizePxOverride(false) {}

LayoutElement::LayoutElement(std::string _name, frac2d _anchor, frac2d _relPos, Point2d _offsetPx)
	: name(_name),
	anchor(_anchor),
	relPos(_relPos),
	offsetPx(_offsetPx),
	isPoint(true),
	sizePxOverride(false) {}

LayoutElement::LayoutElement(frac2d _anchor, frac2d _relPos, std::string _name, Point2d _offsetPx)
	: name(_name),
	anchor(_anchor),
	relPos(_relPos),
	offsetPx(_offsetPx),
	isPoint(false),
	sizePxOverride(true) {}

void LayoutElement::draw(LayoutElement* parent, cRenderer* renderer)
{
	// if (parent == nullptr) // Top of tree; set renderer dimensions
	// {
	// 	this->_bottomLeft = {0,0};
	// 	this->_topRight = {renderer->width, renderer->height};
	// 	this->_sizePx = {renderer->width, renderer->height};
	// }
	// else
	// {
	// 	this->_sizePx = ((parent->_sizePx * relSize) + sizeOffsetPx).abs();
	// 	Point2d anchorOffset = this->_sizePx * this->anchor;
	// 	this->_bottomLeft = ((parent->_sizePx * relPos) + parent->_bottomLeft) - anchorOffset + offsetPx - (sizeOffsetPx / 2);
	// 	this->_topRight = _bottomLeft + _sizePx;
	// 	// this->_topRight = {_bottomLeft.x + _sizePx.x, _bottomLeft.y = _sizePx.y};
	// }
	// if (_bottomLeft.x > _topRight.x) // flip if wrong way round
	// {
	// 	Point2d temp = _bottomLeft;
	// 	this->_bottomLeft = _topRight;
	// 	this->_topRight = temp;
	// }
	this->_drawSelf(renderer);
	for (auto& child : children)
	{
		child->updateLiteralValues(this);
		child->draw(this, renderer);
	}
	return;
}

void LayoutElement::_drawSelf(cRenderer* renderer) { return; }

void LayoutElement::deleteChild(int index)
{
	this->children.erase(children.begin() + index);
	return;
}

void LayoutElement::updateLiteralValues(LayoutElement* parent)
{
	this->preprocessLiterals();
	// Note: if the screen dimensions change, then the root object won't reflect that; maybe bring in renderer here
	if (true) // Top of tree; set renderer dimensions
	{
		Point2d parentSize = (parent == nullptr) ? Point2d{globScreenwidth, globScreenheight} : parent->_sizePx;
		Point2d parentBottomLeft = (parent == nullptr) ? Point2d{0, 0} : parent->_bottomLeft;
		if (!this->isPoint)
		{
			if (!sizePxOverride) // if we have sizePxOverride, then sizePx is already set in the preprocessLiterals step
			{
				this->_sizePx = ((parentSize * relSize) + sizeOffsetPx).abs();
			}
			Point2d anchorOffset = this->_sizePx * this->anchor;
			this->_bottomLeft = ((parentSize * relPos) + parentBottomLeft) - anchorOffset + offsetPx;
			this->_topRight = _bottomLeft + _sizePx;
			// this->_topRight = {_bottomLeft.x + _sizePx.x, _bottomLeft.y = _sizePx.y};
			if (_bottomLeft.x > _topRight.x) // flip if wrong way round
			{
				Point2d temp = _bottomLeft;
				this->_bottomLeft = _topRight;
				this->_topRight = temp;
			}
		}
		else
		{
			this->_bottomLeft = ((parentSize * relPos) + parentBottomLeft) + offsetPx;
		}
	}
	else
	{

	}
	return;
}

void LayoutElement::preprocessLiterals() { return; }

//
// LayoutPosition::LayoutPosition(std::string _name, frac2d _relPos, Point2d _offsetPx)
// 	: name(_name),
// 	relPos(_relPos),
// 	offsetPx(_offsetPx) {}

Rectangle::Rectangle(std::string _name, frac2d _anchor, frac2d _relPos, frac2d _relSize, Colour _fillColour, Point2d _offsetPx, Point2d _sizeOffsetPx)
: LayoutElement(_name, _anchor, _relPos, _relSize, _offsetPx, _sizeOffsetPx),
fillColour(_fillColour.raw()) {}

void Rectangle::_drawSelf(cRenderer* renderer)
{
	for (int y = this->_bottomLeft.y; y <= this->_topRight.y; y++)
	{
		int rowOffset = renderer->width * ((renderer->height - y) - 1);
		std::fill(renderer->bufScreen.begin() + rowOffset + _bottomLeft.x, renderer->bufScreen.begin() + rowOffset + _topRight.x, fillColour);
	}
	return;
}

Text::Text(std::string _name, frac2d _anchor, frac2d _relPos, std::string _text, bmpFont** _font, int scaling, std::function<std::string()> _txfac, Point2d _offsetPx)
: LayoutElement(_anchor, _relPos, _name, _offsetPx),
text(_text),
font(_font),
scaling(scaling),
textFactory(_txfac) {}

// todo: figure out why text put at (0, 0) is drawn a few pixels too low.
void Text::preprocessLiterals()
{
	this->numlines = std::count(this->text.begin(), this->text.end(), '\n') + 1;
	this->_sizePx = {10, (((*font)->sizepx * scaling) + (*font)->lineSpacing) * numlines};
	if (textFactory != nullptr)
	{
		this->text = textFactory();
	}
}

void Text::_drawSelf(cRenderer* renderer)
{
	renderer->hairline->drawText(_bottomLeft + Point2d{0, ((*font)->sizepx + (*font)->lineSpacing) * this->scaling * (this->numlines - 1)}, text, *font, scaling);
}
