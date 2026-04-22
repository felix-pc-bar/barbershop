#pragma once

#include <compare>
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

using extendedValue = std::variant<
	int,
	float,
	bool,
	std::string,
	Material*,
	Colour*
>;

using builderFunction = std::function<objectPointer(std::vector<extendedValue>)>;

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
