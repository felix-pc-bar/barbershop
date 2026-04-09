#include <string>
#include <vector>

// chatgpt
// This ignores commas inside brackets!
std::vector<std::string> splitRespectingBkts(const std::string& s, char delim);

// (C) Rob Cusimano (MIT)
std::string stripws(const std::string& s, bool includeBrackets = false);