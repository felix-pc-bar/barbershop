#include <string>
#include <vector>
#include <functional>

// chatgpt
// This ignores commas inside brackets!
std::vector<std::string> splitRespectingBkts(const std::string& s, char delim);

// (C) Rob Cusimano (MIT)
std::string stripws(const std::string& s, bool stripBrackets = false);

bool isValidDescriptorChar(char c); // is alpha, digit, hyphen or underscore

// chatgpt, again
// We take istream ptr and a bool lambda as a condition
std::string readUntil(std::istream& in, std::function<bool(char)> stopCondition);