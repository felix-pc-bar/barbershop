#include <string>
#include <vector>
#include <functional>
#include <string_view>
#include <optional>

// chatgpt
// This ignores commas inside brackets!
std::vector<std::string> splitRespectingBkts(const std::string& s, char delim);

// (C) Rob Cusimano (MIT)
std::string stripws(const std::string& s, bool stripBrackets = false);

bool isValidDescriptorChar(char c); // is alpha, digit, hyphen or underscore

// chatgpt
// We take istream ptr and a bool lambda as a condition
std::string readUntil(std::istream& in, std::function<bool(char)> stopCondition);

// Used to translate base types in the stubble translator
bool isDigits(const std::string &str);

// chatgpt
std::optional<float> getFloatLiteral(std::string_view s);

//chatgpt
std::optional<std::string> getStringLiteral(std::string_view s);
