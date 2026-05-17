#pragma once

#include <variant>
#include <functional>
#include <string>
#include <utility>
#include <vector>

#include "../material.h"

using baseValue = std::variant<
    int,
    float,
    bool,
    std::string
>;

using objectPointer = std::variant<
    Material*,
    Colour*
>;

class Pyjama;

using extendedValue = std::variant<
	int,
	float,
	bool,
	std::string,
	Pyjama*,
	Material*,
	Colour*
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

// This is just an class that stores the type explicitly (unimportant names)
enum class baseTypeData
{
	Int,
	Float,
	Bool,
	StdString
};

enum class TypeData
{
	Int,
	Float,
	Bool,
	StdString,
	_Colour,
	_Material
};

bool matchesType(const extendedValue& val, TypeData t);

struct FunctionEntry
{
	// Vector of arglist-function pairs
	std::vector<std::pair<std::vector<TypeData>, builderFunction>> builders;
	FunctionEntry(std::vector<std::pair<std::vector<TypeData>, builderFunction>> bldLs);
};
