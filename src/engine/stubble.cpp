#include <iostream>
#include <sstream>
#include <fstream>
#include <ostream>
#include <filesystem>
#include <algorithm>

#include "stubble.h"
#include "helpers/stringy.h"
// #include "material.h"

// static std::unordered_map<std::string, StubbleParser::FunctionEntry> builderLookup = 
// {
// 	{"Colour", {colourBuilder, {StubbleParser::TypeData::Int, StubbleParser::TypeData::Int, StubbleParser::TypeData::Int}}}
// };


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

StubbleParser::extendedValue StubbleParser::parse(std::string filepath)
{
 	if (!std::filesystem::exists(filepath))
	{
	    std::cout << "Error: " << filepath << " does not exist." << std::endl; 
	    return "";
	}

    std::ifstream stbstream(filepath); // read file into ifstream
    std::ostringstream sstr;
    sstr << stbstream.rdbuf();
    std::string stbtext = stripws(sstr.str());
	return this->parseFrag(stbtext);
}

std::optional<StubbleParser::extendedValue> StubbleParser::import(std::string filepath)
{
	if (!std::filesystem::exists(filepath))
	{
	    std::cout << "Error: file does not exist." << std::endl; 
	    return std::nullopt;
	}

    std::ifstream stbstream(filepath); // read file into ifstream
	std::vector<Token> tokens;

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
		currentData = readUntil(stbstream, isPunctuator);
		if (currentData.empty())
		{
			c = stbstream.get();	
			switch (c)
			{
			case '(':
				tokens.emplace_back(TokenType::brOpen, currentline);
				break;
			
			case ')':
				tokens.emplace_back(TokenType::brClose, currentline);
				break;
			
			case ',':
				tokens.emplace_back(TokenType::comma, currentline);
				break;
			}
		}
		else
		{
			tokens.emplace_back(TokenType::data, currentline, currentData);
		}
	}

	// ==== Build AST ====
	// Next, we build a heirachical structure from the lexemes/tokens 
	// This is where structural error checking is done

	if (tokens.front().ttype != TokenType::data)
	{
		std::cout << tokens.front().unexpected() << std::endl;
		return std::nullopt;
	}
	SyntacticalBranch result;
	result.data = tokens.front().data;

	if (tokens[1].ttype != TokenType::brOpen)
	{
		std::cout << tokens[1].unexpected() << std::endl;
		return std::nullopt;
	}
	for (size_t i = 2; i < tokens.size(); i++)
	{
		
	}

	return extendedValue();	
}

StubbleParser::extendedValue StubbleParser::parseFrag(std::string token)
{
    std::size_t openPrnthIndex = token.find_first_of("("); // Index of first open bracket (npos if N/A)

    if (openPrnthIndex == std::string::npos) // No open bracket in string
    {
      // ====Base value translation====
	    // std::cout << "Processing base type token: {" << token << "}\n";
	    StubbleParser::baseValue result;
    	if (token.back() == 'f') { result = std::stof(token.substr(0, token.length() - 1)); } // case float
    	else if (std::all_of(token.begin(), token.end(), ::isdigit)) { result = std::stoi(token); } // check for all digits (https://stackoverflow.com/questions/8888748/)
    	else { std::cout << "Error: The token \"" << token << "\" could not be matched to a known base type." << std::endl;}
    	return result;
    }
    else
    {
    	std::string objectName = token.substr(0, openPrnthIndex);

		// Clunky way of getting what's inside the brackets
		// TODO: clean up
    	std::string args = token.substr(openPrnthIndex, token.length());
		args = stripws(args).substr(1, args.length() - 2);
    	// std::cout << args << std::endl;

		std::vector<std::string> substrings = splitRespectingBkts(args, ',');
    	// std::cout << "Processing object of type " << objectName << std::endl;
		std::vector<StubbleParser::extendedValue> results;
		for (std::string& substr : substrings)
		{
			results.emplace_back(stripws(substr));
		}


    	return StubbleParser::baseValue();
    }
}
