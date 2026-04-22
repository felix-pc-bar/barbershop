#pragma once

#include <variant>
#include <functional>
#include <string>

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
using extendedValue = std::variant<int, float, bool, std::string, objectPointer>;

using builderFunction = std::function<objectPointer(std::vector<extendedValue>)>;

// This is just an class that stores the type explicitly (unimportant names)
enum class TypeData
{
	Int,
	Float,
	Bool,
	StdString
};

struct FunctionEntry
{
	builderFunction func;
	std::vector<TypeData> argTypes;
	FunctionEntry(builderFunction bf, std::vector<TypeData> td);
};
