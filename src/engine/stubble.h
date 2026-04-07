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
    using extendedValue = std::variant<baseValue, objectPointer>;

    extendedValue parse(std::string filepath);
private:
    extendedValue parseFrag(std::string token);
};



// using translatorFunc = std::function<baseValue(std::string)>;

// std::unordered_map<std::string, translatorFunc> translatorMap;