#include <cstdint>
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

	LayoutElement() = default;
	LayoutElement(std::string _name, frac2d _anchor, frac2d _relPos, frac2d _relSize, Point2d _offsetPx = {0,0}, Point2d _sizeOffsetPx = {0,0});
	virtual ~LayoutElement() = default;

	// if parent == nullptr, this is the top element;
	void draw(LayoutElement* parent, cRenderer* renderer);
	virtual void _drawSelf(cRenderer* renderer);
	void deleteChild(int index);
	// unique_ptr provides exclusive ownership but doesn't force the class extensions back into LayoutElement form
	std::vector<std::unique_ptr<LayoutElement>> children;
};

class Rectangle : public LayoutElement
{
public:
	Rectangle(std::string _name, frac2d _anchor, frac2d _relPos, frac2d _relSize, Colour _fillColour, Point2d _offsetPx = {0,0}, Point2d _sizeOffsetPx = {0,0});
	void _drawSelf(cRenderer* renderer) override;
	uint32_t fillColour;
};
