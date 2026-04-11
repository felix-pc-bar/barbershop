#include <iostream>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <algorithm>

#include "stubble.h"
#include "helpers/stringy.h"
// #include "material.h"

// static std::unordered_map<std::string, StubbleParser::FunctionEntry> builderLookup = 
// {
// 	{"Colour", {colourBuilder, {StubbleParser::TypeData::Int, StubbleParser::TypeData::Int, StubbleParser::TypeData::Int}}}
// };


StubbleParser::Token::Token(TokenType tt, std::optional<std::string> d): ttype(tt), data(d) {}

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
		return (std::string(" \n\t\r\f\v").find_first_of(c) == std::string::npos);
	};
	auto isControlChar = [](char c) 
	{
		return (std::string("(),").find_first_of(c) != std::string::npos);
	};

	// ==== Tokenise ====
	// make a flat vector of tokens representing the file.
	// no error checking here- that's later

	std::string currentData; // stores characters up to next control char
	char c;
	for (;;)
	{
		readUntil(stbstream, isntWhitespace); // Skip whitespace
		if (stbstream.peek() == EOF) { break; }
		currentData = readUntil(stbstream, isControlChar);
		if (currentData.empty())
		{
			c = stbstream.get();	
			switch (c)
			{
			case '(':
				tokens.emplace_back(TokenType::brOpen);
				break;
			
			case ')':
				tokens.emplace_back(TokenType::brClose);
				break;
			
			case ',':
				tokens.emplace_back(TokenType::comma);
				break;
			}
		}
		else
		{
			tokens.emplace_back(TokenType::data, currentData);
		}
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