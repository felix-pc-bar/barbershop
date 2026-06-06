#pragma once

#include "stubble.h"

void writeObToAST(extendedValue ob, StubbleParser::SyntacticalBranch* AST);

void _buffer(extendedValue ob, StubbleParser::SyntacticalBranch* AST);
