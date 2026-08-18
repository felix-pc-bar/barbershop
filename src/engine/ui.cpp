#include "ui.h"
#include "general2d.h"
#include "render/render.h"
#include <cstdint>

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
	sizeOffsetPx(_sizeOffsetPx) {}

void LayoutElement::draw(LayoutElement* parent, cRenderer* renderer)
{
	if (parent == nullptr) // Top of tree; set renderer dimensions
	{
		this->_bottomLeft = {0,0};
		this->_topRight = {renderer->width, renderer->height};
		this->_sizePx = {renderer->width, renderer->height};
	}
	else
	{
		this->_sizePx = ((parent->_sizePx * relSize) + sizeOffsetPx).abs();
		Point2d anchorOffset = this->_sizePx * this->anchor;
		this->_bottomLeft = ((parent->_sizePx * relPos) + parent->_bottomLeft) - anchorOffset + offsetPx - (sizeOffsetPx / 2);
		this->_topRight = _bottomLeft + _sizePx;
		// this->_topRight = {_bottomLeft.x + _sizePx.x, _bottomLeft.y = _sizePx.y};
	}
	if (_bottomLeft.x > _topRight.x) // flip if wrong way round
	{
		Point2d temp = _bottomLeft;
		this->_bottomLeft = _topRight;
		this->_topRight = temp;
	}
	this->_drawSelf(renderer);
	for (auto& child : children)
	{
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

Rectangle::Rectangle(std::string _name, frac2d _anchor, frac2d _relPos, frac2d _relSize, Colour _fillColour, Point2d _offsetPx, Point2d _sizeOffsetPx)
: LayoutElement(_name, _anchor, _relPos, _relSize, _offsetPx, _sizeOffsetPx),
fillColour(_fillColour.raw()) {}

void Rectangle::_drawSelf(cRenderer* renderer)
{
	for (int y = this->_bottomLeft.y; y <= this->_topRight.y; y++)
	{
		int rowOffset = renderer->width * y;
		std::fill(renderer->bufScreen.begin() + rowOffset + _bottomLeft.x, renderer->bufScreen.begin() + rowOffset + _topRight.x, fillColour);
	}
	return;
}
