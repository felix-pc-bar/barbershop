#pragma once

#include <variant>
#include <functional>
#include <string>
#include <utility>
#include <vector>

#include "../material.h"
#include "../general3d.h"

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
	Scene*
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
	Scene*
>;

using builderFunction = std::function<objectPointer(std::vector<extendedValue>)>;

// This class is simply a wrapper for vector. (gedit?)
// We do this to allow for recursive vectors
class Pyjama
{
public:
	std::vector<extendedValue> v;
	Pyjama() = default;
};

enum class TypesEnum
{
	Int,
	Float,
	Bool,
	StdString,
	_Colour,
	_Material,
	_Object3D,
	_Position3D,
	_Quaternion,
	_Camera,
	_Scene
};

struct TypeData
{
	TypesEnum type;
	bool isVector;
	TypeData(TypesEnum t, bool isvec = false);
};

std::function<bool(const extendedValue&)> getTypeMatcher(TypeData t);

bool matchesType(const extendedValue& val, TypesEnum t);

struct FunctionEntry
{
	// Vector of arglist-function pairs
	std::vector<std::pair<std::vector<TypeData>, builderFunction>> builders;
	FunctionEntry(std::vector<std::pair<std::vector<TypeData>, builderFunction>> bldLs);
};
