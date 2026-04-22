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
	}
	return false;
}

// FunctionEntry::FunctionEntry(builderFunction bf, std::vector<TypeData> td) : func{bf}, argTypes(td) {}

FunctionEntry::FunctionEntry(builderFunction bf, uint numargs) : func{bf}, numArgs(numargs) {}

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

std::string StubbleParser::Token::unexpected()
{
	return "Unexpected token \"" + data + "\" at line " + std::to_string(lineNumber);
}

StubbleParser::TokenStream::TokenStream(): streamLocation(-1) {} // We init streamloc to -1 because we incr it before reading (trust bro)

std::optional<StubbleParser::Token*> StubbleParser::TokenStream::consume(std::vector<StubbleParser::TokenType> validTypes)
{
	streamLocation++;
	if (std::find(validTypes.begin(), validTypes.end(), tokens[streamLocation].ttype) != validTypes.end())
	{
		return &tokens[streamLocation];
	}
	else // Token is not of valid type
	{
		tokens[streamLocation].unexpected();
		return std::nullopt; // Empty string signifies error reading
	}
}

StubbleParser::Token* StubbleParser::TokenStream::peek()
{
	return &tokens[streamLocation + 1];
}

std::optional<StubbleParser::SyntacticalBranch> StubbleParser::graftFrag(StubbleParser::TokenStream& ts)
{
	StubbleParser::SyntacticalBranch result;
	auto returnedToken = ts.consume({StubbleParser::TokenType::data});
	if (!returnedToken.has_value()) { return std::nullopt; }
	result.data = returnedToken.value()->data;

	auto cc = ts.consume({StubbleParser::TokenType::brOpen, StubbleParser::TokenType::brClose, StubbleParser::TokenType::comma});
	if (!cc.has_value()) { return std::nullopt; }

	switch (cc.value()->ttype)
	{
		case TokenType::brOpen: { // Curly braces to declare a new scope for the int here
			// std::cout << "Starting composite branch- " << result.data << std::endl;
			int unmatchedBrackets = 1;
			do 
			{
				std::optional<StubbleParser::SyntacticalBranch> returnedBranch = graftFrag(ts);
				if (returnedBranch.has_value())
				{
					result.children.emplace_back(returnedBranch.value());
				}
				else
				{
					// std::cout << "Carrying error" << std::endl;
				    return std::nullopt;
				}

	 			auto next = ts.peek();
				if (next->ttype == TokenType::brOpen) { unmatchedBrackets++; }
	 			if (next->ttype == TokenType::brClose) { unmatchedBrackets--; }
			} while (unmatchedBrackets != 0);
		// std::cout << "successfully done composite branch " << result.data << std::endl;
		// we need to advance the pointer by 2,
		// because after peeking the close bracket, the pointer is on the close bkt
		// and we need to skip past that _and_ the following comma
		ts.streamLocation += 2;
		return result;
		}

		case TokenType::comma:
			// std::cout << "Returning leaf branch of value " << result.data << std::endl;
			return result;

		case TokenType::brClose:
			// std::cout << "Reached the end of a composite list, ungetting and returning leaf branch of value " << result.data << std::endl;
			ts.streamLocation--;
			return result;

		default:
		    std::cout << "Fatal error: invalid control character. Cannot continue parsing file." << std::endl;
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
		std::cout << "Error: leaf branch \"" << ast.data << "\" couldn't be matched to a base type." << std::endl;
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
		auto funcEntry = builderLookup.at(typeName);
		if (params.size() != funcEntry.numArgs)
		{
			throw std::invalid_argument("param");
		}
		auto result = funcEntry.func(params);
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
	
	// for (auto token : ts.tokens)
	// {
	// 	std::cout << token.data << std::endl;
	// }

	std::optional<SyntacticalBranch> AST = graftFrag(ts);

	if (!AST.has_value())
	{
		return std::nullopt;
	}

	auto resultantObject = translateTree(AST.value());

	return resultantObject;
}
