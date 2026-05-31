#include "stringy.h"
#include <iostream>
#include <charconv>

std::vector<std::string> splitRespectingBkts(const std::string& s, char delim) {
    std::vector<std::string> out;
    size_t start = 0;
    int depth = 0;

    for (size_t i = 0; i <= s.size(); ++i) {
        if (i < s.size()) {
            if (s[i] == '(') depth++;
            else if (s[i] == ')') depth--;
			if (depth < 0) { break; }
        }

        if (i == s.size() || (s[i] == delim && depth == 0)) {
            out.emplace_back(s.substr(start, i - start));
            start = i + 1;
        }
    }

	if (depth != 0) 
	{
		std::cout << "Error: malformed brackets in string \"" << s << "\"" << std::endl;
	}
    return out;
}

// (C) Rob Cusimano (MIT)
std::string stripws(const std::string& s, bool includeBrackets) {
	// Whitespace is one of: space, tab, carriage return,
	// line feed, form feed, or vertical tab.
	// Additionally strip brackets.
	const char* whitespace;
	if (includeBrackets)
	{
		whitespace = " \t\n\r\f\v()";
	}
	else
	{
		whitespace = " \t\n\r\f\v";
	}
	size_t begin = s.find_first_not_of(whitespace);
	if (begin == std::string::npos) {
		return std::string{};
	}
	size_t end = s.find_last_not_of(whitespace);
	return std::string{s.substr(begin, end - begin + 1)};
}

inline bool isValidDescriptorChar(char c) {
    return (c >= 'A' && c <= 'Z') ||
           (c >= 'a' && c <= 'z') ||
           (c >= '0' && c <= '9') ||
           c == '-' || c == '_';
}

std::string readUntil(std::istream& in, std::function<bool(char)> stopCondition)
{
    std::string result;

    while (true) {
        int c = in.peek();               // look ahead
        if (c == EOF) break;

        if (stopCondition(static_cast<char>(c)))
            break;

        result += static_cast<char>(in.get()); // consume
    }

    return result;
}

// Source - https://stackoverflow.com/a/8889045
// Posted by Blastfurnace
// Retrieved 2026-04-22, License - CC BY-SA 3.0
bool isDigits(const std::string &str)
{
    return str.find_first_not_of("0123456789") == std::string::npos;
}

std::optional<float> getFloatLiteral(std::string_view s)
{
	if (s.empty()) return false;

	// Handle optional suffix
	char last = s.back();
	bool has_suffix = (last == 'f' || last == 'F' || last == 'l' || last == 'L');
	if (has_suffix)
	{
		s.remove_suffix(1);
		if (s.empty()) return false; // "f" alone is invalid
	}

	// Parse using from_chars
	float value;
	auto result = std::from_chars(s.data(), s.data() + s.size(), value,
	                             std::chars_format::general);

	// Valid if:
	// - no error
	// - entire string consumed
	if (result.ec == std::errc() && result.ptr == s.data() + s.size())
	{
		return value;
	}
	else { return std::nullopt; }
}

std::optional<std::string> getStringLiteral(std::string_view s)
{
	if (s.size() < 2) return std::nullopt;
	if (s.front() != '"' || s.back() != '"') return std::nullopt;

	std::string out;
	out.reserve(s.size() - 2);

	for (size_t i = 1; i + 1 < s.size(); ++i)
	{
		char c = s[i];

		if (c == '"')
		{
			return std::nullopt; // illegal unescaped quote
		}

		if (c == '\\')
		{
			if (i + 1 >= s.size() - 1) return std::nullopt;

			char n = s[++i];

			switch (n)
			{
				case '\\': case '"': case '?':
				case 'n': case 't': case 'r':
				case 'a': case 'b': case 'f': case 'v':
					out.push_back('\\');
					out.push_back(n);
					break;

				default:
					return std::nullopt;
			}
		}
		else
		{
			out.push_back(c);
		}
	}

	return out;
}
