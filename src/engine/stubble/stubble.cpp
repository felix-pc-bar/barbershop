#include <iostream>
//#include <sstream>
#include <fstream>
#include <optional>
#include <ostream>
#include <filesystem>
#include <algorithm>
#include <stdexcept>

#include "stubble.h"
#include "../helpers/stringy.h"
#include "types.h"
#include "buildermap.h"
#include "../material.h"

// Bridge between runtime std::variant data and static enum class types
bool matchesType(const extendedValue& val, TypeData t)
{
	switch (t)
	{
	case TypeData::Int:
		return std::holds_alternative<int>(val);
	case TypeData::Bool:
		return std::holds_alternative<bool>(val);
	case TypeData::Float:
		return std::holds_alternative<float>(val);
	case TypeData::StdString:
		return std::holds_alternative<std::string>(val);
	case TypeData::_Colour:
		return std::holds_alternative<Colour*>(val);
	case TypeData::_Material:
		return std::holds_alternative<Material*>(val);
	}
	return false;
}

FunctionEntry::FunctionEntry(std::vector<std::pair<std::vector<TypeData>, builderFunction>> bldLs): builders(bldLs) {}

// TODO: add support for escaped quotes (we don't atm i think)
std::string getDataToken(std::istream& stream)
{
	std::string result;
	char c;
	bool inQuotes = false; // is current char inside quotes
	while (true)
	{
		c = stream.get();
		if (c == EOF) { break; }
		if (c == '\"') { inQuotes = !inQuotes; }
		if (std::string("(),").find_first_of(c) != std::string::npos && !inQuotes)
		{
			break;
		}
		result += static_cast<char>(c);
	}
	stream.unget();
	return result;
}

// We store data and token type- this is duplicitous for the punctuators, as the type always corresponds
// to the same data, but this way we are futureproofed a bit for writing out a file.
StubbleParser::Token::Token(TokenType tt, unsigned int linenum, std::string d): ttype(tt), lineNumber(linenum), data(d) 
{
	switch (tt)
	{
	case TokenType::brOpen:
		data = "(";
		break;
	
	case TokenType::brClose:
		data = ")";
		break;
	
	case TokenType::comma:
		data = ",";
		break;
	
	default:
		break;
	}
}

void StubbleParser::Token::unexpected()
{
	std::cout << "Unexpected token \"" + data + "\" at line " + std::to_string(lineNumber) << std::endl;
	return;
}

StubbleParser::TokenStream::TokenStream(): streamLocation(-1) {} // We init streamloc to -1 because we incr it before reading (trust bro)

std::optional<StubbleParser::Token*> StubbleParser::TokenStream::consume(std::vector<StubbleParser::TokenType> validTypes)
{
	streamLocation++;
	if (std::find(validTypes.begin(), validTypes.end(), tokens[streamLocation].ttype) != validTypes.end())
	{
		return &tokens.at(streamLocation); // throws std::out_of_range
	}
	else // Token is not of valid type
	{
		tokens[streamLocation].unexpected();
		return std::nullopt;
	}
}

StubbleParser::Token* StubbleParser::TokenStream::peek()
{
	if (streamLocation == tokens.size()) { throw std::out_of_range("Pointer is at end of vector"); }
	return &tokens.at(streamLocation + 1);
}

// This function takes a TokenStream.
// It wants the first token in pops to be a data-
// either because it's a leaf, and the data is... the data
// or because it's the identifier for an object.
// If the parse is successful (doesn't return std::nullopt), 
// it will return leaving the next token to be popped being a comma, a close bkt, or EOF.
// otherwise, no promises.
std::optional<StubbleParser::SyntacticalBranch> StubbleParser::graftFrag(StubbleParser::TokenStream& ts, bool parent)
{
	StubbleParser::SyntacticalBranch result;
	auto returnedToken = ts.consume({StubbleParser::TokenType::data}); // Pop a data
	if (!returnedToken.has_value()) { return std::nullopt; }

	result.data = returnedToken.value()->data;
	result.lineNumber = returnedToken.value()->lineNumber;

	try 
	{
		auto cc = ts.peek();

		switch (cc->ttype)
		{
			case TokenType::data:
			{
				cc->unexpected();
				return std::nullopt;
			}

			case TokenType::comma:
				// Non-final leaf case
				// We peeked the comma- return and let parent see/handle it
				return result;

			case TokenType::brClose:
				// Final leaf case
				// Likewise, let parent deal with close bkt
				return result;

			case TokenType::brOpen: { // Curly braces to declare a new scope for the int here
				ts.streamLocation++; // We peeked the open bracket, consume it
				for (;;) // Broken out of
				{
					// Since we skipped the peeked open bracket, we should have a data next up
					std::optional<StubbleParser::SyntacticalBranch> returnedBranch = graftFrag(ts, false);
					if (returnedBranch.has_value())
					{
						result.children.emplace_back(returnedBranch.value());
					}
					else
					{
						// Propogate
					    return std::nullopt;
					}

					// Valid tokens after a child branch:
					// Comma -> continue reading children
					// Close bracket -> end of children list
					// (Nominally we don't exit pointing to any other tokens)
					auto next = ts.consume({ TokenType::comma, TokenType::brClose});
					if (!next.has_value())
					{
						// Error: probably there was an error when parsing the last child branch
						return std::nullopt;
					}

					// Break out of child-getting loop
					if (next.value()->ttype == TokenType::brClose) { break; }
					// Don't do anything for comma we just wanted to consume it
				}
				if (parent && ts.streamLocation + 1 != ts.tokens.size())
				{
					std::cout << "Warning: Trailing token(s) found after final closing bracket. Only one object may be imported at a time." << std::endl;
				}
				return result;
			}

			default:
			    std::cout << "Fatal error: invalid control character. Cannot continue parsing file." << std::endl;
			    return std::nullopt;
		}
	// ts.peek() and ts.consume() throw exceptions if we read too far
	} catch ( std::out_of_range ) {
		std::cout << "Error: hit EOF early. You may be missing trailing punctuators." << std::endl;
		return std::nullopt;
	}
}

std::optional<extendedValue> StubbleParser::translateTree(StubbleParser::SyntacticalBranch& ast)
{
	if (ast.children.size() == 0) // leaf
	{
		if (ast.data == "true") { return true; }
		if (ast.data == "false") { return false; }
		if (isDigits(ast.data))
		{
			return stoi(ast.data);
		}
		auto fpResult = getFloatLiteral(ast.data);
		if (fpResult.has_value())
		{
			return fpResult.value();
		}
		auto stResult = getStringLiteral(ast.data);
		if (stResult.has_value())
		{
			return stResult.value();
		}
		// Couldn't match to any base type
		std::cout << "Error: leaf branch \"" << ast.data << "\" at line " << ast.lineNumber << " couldn't be matched to a base type." << std::endl;
		return std::nullopt;
	}
	else
	{
		std::vector<extendedValue> translatedChildren;
		for (auto branch : ast.children)
		{
			auto br = translateTree(branch);
			if (!br.has_value())
			{
				// Propograte error
				return std::nullopt;
			}
			translatedChildren.emplace_back(br.value());
		}
		auto builtOb = StubbleParser::getBuiltObject(ast.data, translatedChildren);
		if (!builtOb.has_value())
		{
			// Propogate error
			std::cout << "Error: Couldn't build object \"" << ast.data << "\" at line " << ast.lineNumber << std::endl;
			return std::nullopt;
		}

		auto obptr = builtOb.value();
		//convert to extendedValue
		extendedValue result = std::visit([](auto&& value) -> extendedValue {
			return value;
		}, obptr);

		return result;
	}
}

std::optional<objectPointer> StubbleParser::getBuiltObject(std::string typeName, std::vector<extendedValue> params)
{
	try
	{
		// ==== Match parameters to a builder function ====

		auto funcEntry = builderLookup.at(typeName);

		builderFunction bf = nullptr;

		for (size_t i = 0; i < funcEntry.builders.size(); i++)
		{
			// No point checking if individual params match if there's different numbers of them
			if (params.size() == funcEntry.builders[i].first.size())
			{
				auto wantedParamTypeList = funcEntry.builders[i].first;
				bool match = true;

				for (size_t j= 0; j < wantedParamTypeList.size(); j++)
				{
					if (!matchesType(params[j], wantedParamTypeList[j]))
					{
						match = false;
						break;
					}
				} // loop of each parameter

				if (match == true)
				{
					// Loop was completed with all params matching
					// Break out of main loop with the function we found
					bf = funcEntry.builders[i].second;
					break;
				}
			}
		} // </loop of potential builders>

		if (bf == nullptr) // None of the options got all the way thru without finding a false type
		{
			std::cout << "Error: Couldn't match parameters for construction of \"" << typeName << "\" to any builder" << std::endl;
			return std::nullopt;
		}

		auto result = bf(params);
		return result;
	}
	catch (std::out_of_range)
	{
		std::cout << "Error: type \"" << typeName << "\" is not supported." << std::endl;
		return std::nullopt;
	}
	catch (std::invalid_argument)
	{
		std::cout << "Error: passed invalid arguments in construction of \"" << typeName << "\"" << std::endl;
		// TODO: Add reporting for wanted/given parameter types
		return std::nullopt;
	}
	catch ( ... )
	{
		std::cout << "Error: Could not import \"" << typeName << "\"" << std::endl;
		return std::nullopt;
	}
}

std::optional<extendedValue> StubbleParser::import(std::string filepath)
{
	if (!std::filesystem::exists(filepath))
	{
	    std::cout << "Error: file does not exist." << std::endl; 
	    return std::nullopt;
	}

    std::ifstream stbstream(filepath); // read file into ifstream
	TokenStream ts;

	auto isntWhitespace = [](char c) 
	{
		return (std::string(" \t\r\f\v").find_first_of(c) == std::string::npos);
	};
	auto isPunctuator = [](char c) 
	{
		return (std::string("(),").find_first_of(c) != std::string::npos);
	};

	// ==== Tokenise ====
	// Performs lexical analysis on input file.
	// make a flat vector of tokens representing the file
	// no error checking here- that's later

	std::string currentData; // stores characters up to next punctuator
	char c;
	unsigned int currentline = 1;

	for (;;)
	{
		readUntil(stbstream, isntWhitespace); // Skip whitespace
		if (stbstream.peek() == EOF) { break; }
		if (stbstream.peek() == '\n') { currentline++; stbstream.get(); } //Skip newline but incr counter
		// currentData = readUntil(stbstream, isPunctuator);
		currentData = getDataToken(stbstream);
		if (currentData.empty())
		{
			c = stbstream.get();	
			switch (c)
			{
			case '(':
				ts.tokens.emplace_back(TokenType::brOpen, currentline);
				break;
			
			case ')':
				ts.tokens.emplace_back(TokenType::brClose, currentline);
				break;
			
			case ',':
				ts.tokens.emplace_back(TokenType::comma, currentline);
				break;
			}
		}
		else
		{
			// Strip leading/trailing whitespace on individual token (this is the data path)
			auto start = std::find_if_not(currentData.begin(), currentData.end(), [](unsigned char c) {
				return std::isspace(c);
		    });
	
		    auto end = std::find_if_not(currentData.rbegin(), currentData.rend(), [](unsigned char c) {
				return std::isspace(c);
		    }).base();
		
		    if (start >= end) { continue; } // This skips adding a token, because it would be an empty data
		    else 
			{
				currentData = std::string(start, end);
			}
			ts.tokens.emplace_back(TokenType::data, currentline, currentData);
		}
	}

	// ==== Build AST ====
	// Next, we build a heirachical structure from the lexemes/tokens 
	// This is where structural error checking is done
	
	auto AST = graftFrag(ts);

	if (!AST.has_value())
	{
		std::cout << "Error when grafting \"" << filepath << "\"\n";
		return std::nullopt;
	}

	auto result = translateTree(AST.value()) ;
	if (!result.has_value())
	{
		std::cout << "Error when translating \"" << filepath << "\"\n";
	}
	return result;
}
