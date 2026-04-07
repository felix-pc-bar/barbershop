#include <iostream>
#include <sstream>
#include <fstream>
#include <filesystem>

#include "stubble.h"
#include "helpers/stringy.h"

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

StubbleParser::extendedValue StubbleParser::parseFrag(std::string token)
{
    std::size_t openPrnthIndex = token.find_first_of("("); // Index of first open bracket (npos if N/A)

    if (openPrnthIndex == std::string::npos) // No open bracket in string
    {
      // ====Base value translation====
      StubbleParser::baseValue result;
    	if (token.back() == 'f') { result = std::stof(token.substr(0, token.length() - 1)); } // case float
    	else if (std::all_of(token.begin(), token.end(), ::isdigit)) { result = std::stoi(token); } // check for all digits (https://stackoverflow.com/questions/8888748/)
    	else { std::cout << "Error: The token \"" << token << "\" could not be matched to a known base type." << std::endl;}
    	return result;
    }
    else
    {
    	std::string typeName = token.substr(0, openPrnthIndex);
    	std::string args = stripws(token.substr(openPrnthIndex, token.length()));
    	std::cout << typeName << std::endl;
    	std::cout << args << std::endl;
    	return StubbleParser::baseValue();
    }
}