#include "builders.h"
#include "types.h"

#include "../import3d.h"
#include "../render/components/bmpfont.h"

Colour* _colRGB(std::vector<extendedValue> args)
{
	return new Colour(std::get<float>(args[0]),
			   std::get<float>(args[1]),
			   std::get<float>(args[2])
			   );
}

Colour* _colRGBA(std::vector<extendedValue> args)
{
	return new Colour(std::get<float>(args[0]),
		   std::get<float>(args[1]),
		   std::get<float>(args[2]),
		   std::get<float>(args[3])
		   );
}

Material* _matColPtShd(std::vector<extendedValue> args)
{
	return new Material(*std::get<Colour*>(args[0]), std::get<int>(args[1]), std::get<bool>(args[2]));
}

Object3D* _objPathpp(std::vector<extendedValue> args)
{
	Object3D* result = new Object3D(importObj(std::get<std::string>(args[0])));
	result->materials[0] = *std::get<Material*>(args[1]);
	return result;
}

Position3d* _posXYZf(std::vector<extendedValue> args);

Quaternion* _quatAngleAxis(std::vector<extendedValue> args);

Camera* _camPosQuatFov(std::vector<extendedValue> args);

Scene* _scnFull(std::vector<extendedValue> args);

pixelBuffer* _pxbufWidthData(std::vector<extendedValue> args)
{
	pixelBuffer* result = new pixelBuffer(std::get<int>(args[0]), std::get<std::string>(args[1]));
	return result;
}

// bmpGlyph from pixelBuffer, x placement, y placement (ints), printable (bool)
bmpGlyph* _bmpglyphBufXYPrintable(std::vector<extendedValue> args)
{
	bmpGlyph* result = new bmpGlyph();
	result->bitmap = std::get<pixelBuffer*>(args[0]);
	result->placementX = std::get<int>(args[1]);
	result->placementY = std::get<int>(args[2]);
	result->isPrintable = std::get<bool>(args[3]);
	return result;
}

// bmpFont from name (std::string), size in px (int), and a pyjama of bmpGlyph*
bmpFont* _bmpfontNameSizeKernPJGlyph(std::vector<extendedValue> args)
{
	bmpFont* result = new bmpFont();
	result->name = std::get<std::string>(args[0]);
	result->sizepx = std::get<int>(args[1]);
	result->defaultKerning = std::get<int>(args[2]);
	Pyjama* glpyhpj = std::get<Pyjama*>(args[3]);
	for (int i = 0; i < std::max(128, static_cast<int>(glpyhpj->v.size())); i++)
	{
		bmpGlyph* ptr = std::get<bmpGlyph*>(glpyhpj->v[i]);
		result->glyphs[i] = ptr;
	}
	return result;
}
