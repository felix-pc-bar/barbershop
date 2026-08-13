#pragma once
#include "builders.h"
#include "types.h"
#include <vector>


static std::unordered_map<std::string, builderFunctionEntry> builderLookup =
{
	{
		"Colour",
		builderFunctionEntry
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
		builderFunctionEntry
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
		builderFunctionEntry
		{
			{
				{
					{ TypesEnum::StdString, TypesEnum::_Material},
					_objPathpp
				}
			}
		}
	},
	{
		"pixelBuffer",
		builderFunctionEntry
		{
			{
				{
					{ TypesEnum::Int, TypesEnum::StdString },
					_pxbufWidthData
				}
			}
		}
	},
	{
		"bmpGlyph",
		builderFunctionEntry
		{
			{
				{
					{ TypesEnum::_pixelBuffer, TypesEnum::Int, TypesEnum::Int, TypesEnum::Bool },
					_bmpglyphBufXYPrintable
				}
			}
		}
	},
	{
		"bmpFont",
		builderFunctionEntry
		{
			{
				{
					{ TypesEnum::StdString, TypesEnum::Int, TypesEnum::Int, TypesEnum::_Pyjama },
					_bmpfontNameSizeKernPJGlyph
				}
			}
		}
	}
};
// builderFunctionEntry(std::vector<std::pair<std::vector<TypeData>, builderFunction>> bldLs);
