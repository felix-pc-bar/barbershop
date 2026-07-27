#pragma once

#include "stubble.h"
#include "writers.h"

static std::unordered_map<std::string, writerFunction> writerLookup =
{
	{
		"pixelBuffer",
		_buffer
	},
	{
		"Pyjama",
		_Pyjama
	},
	{
		"bmpGlyph",
		_bmpGlyph
	},
	{
		"bmpFont",
		_bmpFont
	}
};

// writerFunctionEntry(std::vector<std::pair<std::vector<TypeData>, writerFunction>> wrtLs);
