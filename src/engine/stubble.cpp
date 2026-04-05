#include <iostream>
#include <sstream>
#include <fstream>
#include <filesystem>

#include "stubble.h"

// (C) Rob Cusimano (MIT)
std::string stripws(const std::string& s) {
  // Whitespace is one of: space, tab, carriage return,
  // line feed, form feed, or vertical tab.
  const char* whitespace = " \t\n\r\f\v";
  size_t begin = s.find_first_not_of(whitespace);
  if (begin == std::string::npos) {
    return std::string{};
  }
  size_t end = s.find_last_not_of(whitespace);
  return std::string{s.substr(begin, end - begin + 1)};
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

    // std::cout << stbtext << std::endl;

    baseValue result;
    if (stbtext.back() == 'f') { result = std::stof(stbtext.substr(0, stbtext.length() - 1)); } // case float
    else if (std::all_of(stbtext.begin(), stbtext.end(), ::isdigit)) { result = std::stoi(stbtext); } // check for all digits (https://stackoverflow.com/questions/8888748/)
    else { std::cout << "Error: The token \"" << stbtext << "\" could not be matched." << std::endl;}

    if (std::holds_alternative<float>(result)) 
    {
        std::cout << std::get<float>(result) << std::endl;
    }
    return result;

}