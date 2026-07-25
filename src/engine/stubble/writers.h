#pragma once

#include "stubble.h"

void writeObToAST(extendedValue ob, StubbleParser::SyntacticalBranch* AST);

void _buffer(extendedValue ob, StubbleParser::SyntacticalBranch* AST);

void _Pyjama(extendedValue ob, StubbleParser::SyntacticalBranch* AST);

void _bmpGlyph(extendedValue ob, StubbleParser::SyntacticalBranch* AST);

void _bmpFont(extendedValue ob, StubbleParser::SyntacticalBranch* AST);
