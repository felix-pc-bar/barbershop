#pragma once

#include <variant>
#include <functional>
#include <string>
#include <utility>
#include <vector>

#include "../material.h"
#include "../general3d.h"
#include "../buffer.h"
#include "../render/components/bmpfont.h"

using baseValue = std::variant<
    int,
    float,
    bool,
    std::string
>;

using objectPointer = std::variant<
    Material*,
    Colour*,
	Object3D*,
	Scene*,
	pixelBuffer*,
	bmpGlyph*,
	bmpFont*
>;

class Pyjama;

using extendedValue = std::variant<
	int,
	float,
	bool,
	std::string,
	Pyjama*,
	Colour*,
	Material*,
	Object3D*,
	Position3d*,
	Quaternion*,
	Camera*,
	Scene*,
	pixelBuffer*,
	bmpGlyph*,
	bmpFont*
>;

enum class TypesEnum
{
	Int,
	Float,
	Bool,
	StdString,
	_Pyjama,
	_Colour,
	_Material,
	_Object3D,
	_Position3D,
	_Quaternion,
	_Camera,
	_Scene,
	_pixelBuffer,
	_bmpGlyph,
	_bmpFont
};

using builderFunction = std::function<objectPointer(std::vector<extendedValue>)>;

// This class is simply a wrapper for vector. (gedit?)
// We do this to allow for recursive vectors
class Pyjama
{
public:
	std::vector<extendedValue> v;
	Pyjama() = default;
};

struct TypeData
{
	TypesEnum type;
	bool isVector;
	TypeData(TypesEnum t, bool isvec = false);
};

std::function<bool(const extendedValue&)> getTypeMatcher(TypeData t);

bool matchesType(const extendedValue& val, TypeData t);

std::string_view getTypeName(const extendedValue& ob);

struct builderFunctionEntry
{
	// Vector of arglist-function pairs
	std::vector<std::pair<std::vector<TypeData>, builderFunction>> builders;
	builderFunctionEntry(std::vector<std::pair<std::vector<TypeData>, builderFunction>> bldLs);
};
