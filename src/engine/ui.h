#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>
#include <memory>
#include <string>

#include "general2d.h"
#include "render/render.h"
#include "material.h"

struct frac2d
{
	float X;
	float Y;
};

Point2d operator*(Point2d const& lhs, frac2d const& rhs);

class LayoutElement
{
protected:
	// Derived from the public dimensions at draw-time
	Point2d _bottomLeft, _topRight;
public:
	std::string name;
	frac2d anchor; // Where the anchor is on this object
	frac2d relPos; // Where to put the anchor, as fraction of the parent
	Point2d offsetPx;
	frac2d relSize; // as a fraction of the parent
	Point2d sizeOffsetPx;

	// Derived from the public dimensions at draw-time
	Point2d _sizePx;

	// unique_ptr provides exclusive ownership but doesn't force the class extensions back into LayoutElement form
	std::vector<std::unique_ptr<LayoutElement>> children;

	// We store notable elements in the phoneBook, with names/ids and a map to get to them from this object; i.e. [0,2,5,4] is children[0].children[2].etc...
	std::unordered_map<std::string, std::vector<int>> phoneBook;

	LayoutElement() = default;
	LayoutElement(std::string _name, frac2d _anchor, frac2d _relPos, frac2d _relSize, Point2d _offsetPx = {0,0}, Point2d _sizeOffsetPx = {0,0});
	virtual ~LayoutElement() = default;

	// Copy constructor and assign
	LayoutElement(LayoutElement&&) = default;
	LayoutElement& operator=(LayoutElement&&) = default;

	// if parent == nullptr, this is the top element;
	void draw(LayoutElement* parent, cRenderer* renderer);
	virtual void _drawSelf(cRenderer* renderer);
	void deleteChild(int index);
};

// TODO:
// - LayoutPosition won't update the _realPosition; it's never drawn
// - Split draw and position calculations: maybe a set() to allow for interrupt-ish recalculation
// - Finish Text
class LayoutPosition
{
protected:
	// Derived from the public dimensions at draw-time
	Point2d _realPosition;
public:
	std::string name;
	frac2d relPos; // Where to put the anchor, as fraction of the parent
	Point2d offsetPx;

	LayoutPosition() = default;
	LayoutPosition(std::string _name, frac2d _anchor, Point2d _offsetPx = {0,0});
	virtual ~LayoutPosition() = default;

	// Copy constructor and assign
	LayoutPosition(LayoutPosition&&) = default;
	LayoutPosition& operator=(LayoutPosition&&) = default;
};

class Rectangle : public LayoutElement
{
public:
	uint32_t fillColour;

	Rectangle(std::string _name, frac2d _anchor, frac2d _relPos, frac2d _relSize, Colour _fillColour, Point2d _offsetPx = {0,0}, Point2d _sizeOffsetPx = {0,0});
	void _drawSelf(cRenderer* renderer) override;
};

class Text : public LayoutElement
{
public:
	std::string text;

	Text(std::string _name, frac2d _anchor, frac2d _relPos, std::string _text, Point2d _offsetPx = {0,0});
	void _drawSelf(cRenderer* renderer) override;
};
