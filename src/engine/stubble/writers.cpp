#include <string>
#include <unistd.h>
#include <variant>
#include <iostream>

#include "writers.h"
#include "writermap.h"
#include "types.h"

#include "../buffer.h"
#include "stubble.h"

void writeObToAST(extendedValue ob, StubbleParser::SyntacticalBranch* AST)
{
	std::string tname(getTypeName(ob));
	if (tname == "int")
	{
		AST->children.emplace_back(std::to_string(std::get<int>(ob)));
		return;
	}
	if (tname == "float")
	{
		AST->children.emplace_back(std::to_string(std::get<float>(ob)) + "f");
		return;
	}
	if (tname == "bool")
	{
		AST->children.emplace_back(std::get<bool>(ob) ? "true" : "false");
		return;
	}
	if (tname == "std::string")
	{
		std::cout << std::get<std::string>(ob) << std::endl;
		AST->children.emplace_back("\"" + std::get<std::string>(ob) + "\"");
		return;
	}
	AST->data = getTypeName(ob);
	writerFunction wf = writerLookup.at(std::string(getTypeName(ob)));
	wf(ob, AST);
	return;
}

void _buffer(extendedValue ob, StubbleParser::SyntacticalBranch* AST)
{
	if (!std::holds_alternative<pixelBuffer*>(ob)) { return; }
	pixelBuffer pxbuf = *std::get<pixelBuffer*>(ob);
	// AST->children.emplace_back(std::to_string(pxbuf.width));
	writeObToAST(pxbuf.width, AST);
	// AST->children.emplace_back(pxbuf.bufB64());
	writeObToAST(pxbuf.bufB64(), AST);
	return;
}
