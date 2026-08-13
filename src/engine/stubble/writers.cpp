#include <stdexcept>
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

// Hey future me!
// okay so here's how this mess should work
// as seen in stbExport(), writeObToAST() is called on some extendedValue ob.
// the result SyntacticalBranch AST is created here, and assigned it's data local
// on the basis of whether it's a leaf or a branch.
// however! in the case we have a Pyjama, _Pyjama() is a special writer function 
// in that it overwrites the data value of the AST it is passed.
// normally, a writer function should only touch the children of the AST they're passed!! (important)

// To allow for indefinite recursion, the writer functions call writeASTRecurseKernel(),
// in order to add extendedValues to the children of the AST they've been passed as SyntacticalBranch.
// In writeASTRecurseKernel(), a new child AST is made, and the extendedValue sent by the writerFunction
// is passed to writeObToAST() to be put into said child AST, and the circle is complete.
// after this, the complete child AST is appended to the children vector of the AST passed by the writer func.
// -- that AST, of course, being the same one passed to the writer function by the call in writeObToAST().

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
		// std::cout << std::get<std::string>(ob) << std::endl;
		AST->data = "\"" + std::get<std::string>(ob) + "\"";
		return;
	}
	AST->data = tname;
	// TODO: can we not skip per-wf arg validation because of this step \/
	try
	{
		writerFunction wf = writerLookup.at(tname);
		wf(ob, AST);
	} catch (std::out_of_range) {
		std::cout << "Error: no writer function found for " << tname << std::endl;
	}
	return;
}

// it's messy, but this is the step where recursion occurs
// really just shorthand to avoid having to write this out every time
void writeASTRecurseKernel(extendedValue ob, StubbleParser::SyntacticalBranch* AST) // catchy eh
{
	// Be carefull: must ensure childAST remains valid for as long as we need the tree root
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
	// Vector branch: data must store typename
	// We take it on faith that pj.v contains extendedValues of all the same type
	AST->data = getTypeName(pj.v[0]);
	for (extendedValue thing : pj.v)
	{
		writeASTRecurseKernel(thing, AST);
	}
	return;
}

void _bmpGlyph(extendedValue ob, StubbleParser::SyntacticalBranch* AST)
{
	if (!std::holds_alternative<bmpGlyph*>(ob)) { return; }
	bmpGlyph glyph = *std::get<bmpGlyph*>(ob);
	writeASTRecurseKernel(glyph.bitmap, AST);
	writeASTRecurseKernel(glyph.placementX, AST);
	writeASTRecurseKernel(glyph.placementY, AST);
	writeASTRecurseKernel(glyph.isPrintable, AST);
	return;
}

void _bmpFont(extendedValue ob, StubbleParser::SyntacticalBranch* AST)
{
	if (!std::holds_alternative<bmpFont*>(ob)) { return; }
	bmpFont font = *std::get<bmpFont*>(ob);
	writeASTRecurseKernel(font.name, AST);
	writeASTRecurseKernel(font.sizepx, AST);
	writeASTRecurseKernel(font.defaultKerning, AST);
	// We need to convert the std::array for glyphs into a vector, so it can pack into a pyjama object
	Pyjama* glyphs = new Pyjama;
	glyphs->v = std::vector<extendedValue>(font.glyphs.begin(), font.glyphs.end());
	writeASTRecurseKernel(glyphs, AST);
}