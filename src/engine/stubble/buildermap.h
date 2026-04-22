#pragma once
#include "builders.h"
#include "types.h"

static std::unordered_map<std::string, FunctionEntry> builderLookup =
{
	{"Colour", {colourBuilder, 4}}
};
