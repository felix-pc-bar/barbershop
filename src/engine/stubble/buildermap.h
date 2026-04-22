#pragma once
#include "builders.h"
#include "types.h"
#include <vector>


static std::unordered_map<std::string, FunctionEntry> builderLookup =
{
{
		"Colour", 
		FunctionEntry{
	{
			{
					{ TypeData::Float, TypeData::Float, TypeData::Float},
					_colRGB
				},
			{
					{ TypeData::Float, TypeData::Float, TypeData::Float, TypeData::Float },
					_colRGBA
				}
			}
		}
	},
{
		"Material", 
		FunctionEntry{
	{
			{
					{ TypeData::_Colour, TypeData::Int, TypeData::Bool},
					_matColPtShd
				}
			}
		}
	}
};
// FunctionEntry(std::vector<std::pair<std::vector<TypeData>, builderFunction>> bldLs);
