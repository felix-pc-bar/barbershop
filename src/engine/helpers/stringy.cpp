#include "stringy.h"

std::vector<std::string> split(const std::string& s, char delim) {
    std::vector<std::string> out;
    size_t start = 0;

    for (size_t i = 0; i <= s.size(); ++i) {
        if (i == s.size() || s[i] == delim) {
            out.emplace_back(s.substr(start, i - start));
            start = i + 1;
        }
    }
    return out;
}

// (C) Rob Cusimano (MIT)
std::string stripws(const std::string& s) {
  // Whitespace is one of: space, tab, carriage return,
  // line feed, form feed, or vertical tab.
  // Additionally strip brackets.
  const char* whitespace = " \t\n\r\f\v()";
  size_t begin = s.find_first_not_of(whitespace);
  if (begin == std::string::npos) {
    return std::string{};
  }
  size_t end = s.find_last_not_of(whitespace);
  return std::string{s.substr(begin, end - begin + 1)};
}