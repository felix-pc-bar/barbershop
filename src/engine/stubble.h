#include <variant>
#include <unordered_map>
#include <string>
#include <functional>

#include "material.h"

class StubbleParser
{
public:
    using baseValue = std::variant<
        int,
        float,
        bool,
        std::string
    >;
    using objectPointer = std::variant<
        Colour*
    >;

    void parse(std::string filepath);
};

// using translatorFunc = std::function<baseValue(std::string)>;

// std::unordered_map<std::string, translatorFunc> translatorMap;