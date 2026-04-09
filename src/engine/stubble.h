#include <variant>
#include <unordered_map>
#include <string>
#include <functional>
#include <optional>

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
        Material*,
        Colour*
    >;
    using extendedValue = std::variant<baseValue, objectPointer>;
    using builderFunction = std::function<objectPointer(std::vector<extendedValue>)>;

    // This is just an class that stores the type explicitly (unimportant names)
    enum class TypeData
    {
        Int,
        Float,
        Bool,
        StdString
    };

    enum class TokenType
    {
        obTypename,
        brOpen,
        brClose,
        comma,
        baseTypeData
    };

    struct Token
    {
        TokenType ttype;
        std::optional<std::string> data;
    };

    // Stores function pointer alongside arg types it wants
    struct FunctionEntry
    {
        builderFunction func;
        std::vector<TypeData> argTypes;
        FunctionEntry() = default;
    };

    std::optional<extendedValue> import(std::string filepath);

    extendedValue parse(std::string filepath);
private:
    extendedValue parseFrag(std::string token);
};

// using translatorFunc = std::function<baseValue(std::string)>;

// std::unordered_map<std::string, translatorFunc> translatorMap;