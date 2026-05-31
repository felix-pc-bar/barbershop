#pragma once
#include "builders.h"
#include "types.h"
#include <vector>


static std::unordered_map<std::string, FunctionEntry> builderLookup =
{
	{
		"Colour",
		FunctionEntry
		{
			{
				{
					{ TypesEnum::Float, TypesEnum::Float, TypesEnum::Float},
					_colRGB
				},
				{
					{ TypesEnum::Float, TypesEnum::Float, TypesEnum::Float, TypesEnum::Float },
					_colRGBA
				}
			}
		}
	},
	{
		"Material",
		FunctionEntry
		{
			{
				{
					{ TypesEnum::_Colour, TypesEnum::Int, TypesEnum::Bool},
					_matColPtShd
				}
			}
		}
	},
	{
		"Object3D",
		FunctionEntry
		{
			{
				{
					{ TypesEnum::StdString, TypesEnum::_Material},
					_objPathpp
				}
			}
		}
	}
};
// FunctionEntry(std::vector<std::pair<std::vector<TypeData>, builderFunction>> bldLs);
