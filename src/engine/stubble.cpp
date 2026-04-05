#include <iostream>
#include <sstream>
#include <fstream>
#include <filesystem>

#include "stubble.h"

void StubbleParser::parse(std::string filepath)
{
   	if (!std::filesystem::exists(filepath))
	{
       std::cout << "Error: " << filepath << " does not exist." << std::endl; 
       return;
	}

    std::ifstream stbstream(filepath); // read file into ifstream
    // Convert to std::string (Konrad Rudolph)
    std::ostringstream sstr;
    sstr << stbstream.rdbuf();
    std::string stbtext = sstr.str();
    // Strip whitespace (Rob Cusimano)
    const char* whitespace = " \t\n\r\f\v"; // All whitespace chars to remove
    size_t begin = stbtext.find_first_not_of(whitespace);
    if (begin == std::string::npos) {
        std::cout << "Error: " << filepath << " is empty." << std::endl;
        return;
    }
    size_t end = stbtext.find_last_not_of(whitespace);
    std::string stripped{stbtext.substr(begin, end - begin + 1)};
    std::cout << stripped << std::endl;
}