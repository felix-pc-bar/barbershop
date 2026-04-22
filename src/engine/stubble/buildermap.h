#include "builders.h"
#include "types.h"

static std::unordered_map<std::string, FunctionEntry> builderLookup =
{
	{"Colour", {colourBuilder, {TypeData::Int, TypeData::Int, TypeData::Int}}}
};
