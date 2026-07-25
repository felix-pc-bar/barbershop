#include <string>
#include <unistd.h>
#include <variant>
#include <iostream>

#include "writers.h"
#include "builders.h"
#include "writermap.h"
#include "types.h"

#include "../buffer.h"
#include "stubble.h"
#include "../render/components/bmpfont.h"

void writeObToAST(extendedValue ob, StubbleParser::SyntacticalBranch* AST)
{
	std::string tname(getTypeName(ob));
	if (tname == "int")
	{
		AST->data = std::to_string(std::get<int>(ob));
		return;
	}
	if (tname == "float")
	{
		AST->data = std::to_string(std::get<float>(ob)) + "f";
		return;
	}
	if (tname == "bool")
	{
		AST->data = std::get<bool>(ob) ? "true" : "false";
		return;
	}
	if (tname == "std::string")
	{
		std::cout << std::get<std::string>(ob) << std::endl;
		AST->data = "\"" + std::get<std::string>(ob) + "\"";
		return;
	}
	AST->data = tname;
	// TODO: can we not skip per-wf arg validation because of this step \/
	writerFunction wf = writerLookup.at(std::string(getTypeName(ob)));
	wf(ob, AST);
	return;
}

// it's messy, but this is the step where recursion occurs
// really just shorthand to avoid having to write this out every time
void writeASTRecurseKernel(extendedValue ob, StubbleParser::SyntacticalBranch* AST) // catchy eh
{
	StubbleParser::SyntacticalBranch* childAST = new StubbleParser::SyntacticalBranch();
	writeObToAST(ob, childAST);
	AST->children.emplace_back(*childAST);
	return;
}

void _buffer(extendedValue ob, StubbleParser::SyntacticalBranch* AST)
{
	if (!std::holds_alternative<pixelBuffer*>(ob)) { return; }
	pixelBuffer pxbuf = *std::get<pixelBuffer*>(ob);
	writeASTRecurseKernel(pxbuf.width, AST);
	writeASTRecurseKernel(pxbuf.bufB64(), AST);
	return;
}

void _Pyjama(extendedValue ob, StubbleParser::SyntacticalBranch* AST)
{
	if (!std::holds_alternative<Pyjama*>(ob)) { return; }
	Pyjama pj = *std::get<Pyjama*>(ob);
	AST->isVector = true;
}

void _bmpGlyph(extendedValue ob, StubbleParser::SyntacticalBranch* AST)
{
	if (!std::holds_alternative<bmpGlyph*>(ob)) { return; }
	bmpGlyph glyph = *std::get<bmpGlyph*>(ob);
	writeASTRecurseKernel(glyph.bitmap, AST);
	writeASTRecurseKernel(glyph.placementX, AST);
	writeASTRecurseKernel(glyph.placementY, AST);
	return;
}

void _bmpFont(extendedValue ob, StubbleParser::SyntacticalBranch* AST)
{
	if (!std::holds_alternative<bmpFont*>(ob)) { return; }
	bmpFont font = *std::get<bmpFont*>(ob);
	writeASTRecurseKernel(font.name, AST);
	writeASTRecurseKernel(font.sizepx, AST);
	// We need to convert the std::array for glyphs into a vector, so it can pack into a pyjama object
	Pyjama* glyphs;
	glyphs->v = std::vector<extendedValue>(font.glyphs.begin(), font.glyphs.end());
	writeASTRecurseKernel(glyphs, AST);
}
