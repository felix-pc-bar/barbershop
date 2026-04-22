//#include <variant>
//#include <unordered_map>
#include <string>
//#include <functional>
#include <optional>
#include <vector>

#include "types.h"

std::string getDataToken(std::istream& stream);

class StubbleParser
{
public:
    enum class TokenType
    {
        data,
        brOpen,
        brClose,
        comma
    };

    // Tokens are either data, or delimiter. We don't differentiate between base type data and object type names yet.
    struct Token
    {
        TokenType ttype;
        std::string data;
        unsigned int lineNumber; // Line number token was read on, 0 for none specced

        Token() = default;
        Token(TokenType tt, unsigned int linenum = 0, std::string d = "");

        std::string unexpected();
    };

	class TokenStream
	{
	public:
		std::vector<Token> tokens;
		int streamLocation;

		TokenStream();
		std::optional<Token*> consume(std::vector<TokenType> validTypes);
		Token* peek();
	};

    // AST branch object.
    // if no children, this branch is a "leaf" and 'data' stores the string to be converted
    // otherwise, 'data' is an identifier for the class that needs to be constructed with the parameters stored in chidren
    class SyntacticalBranch
    {
    public:
        SyntacticalBranch() = default;
        std::string data;
        std::vector<SyntacticalBranch> children;
    };
	
	std::optional<SyntacticalBranch> graftFrag(TokenStream& ts);

	std::optional<extendedValue> translateTree(SyntacticalBranch& ast);

	std::optional<objectPointer> getBuiltObject(std::string typeName, std::vector<extendedValue> params);

    // Stores function pointer alongside arg types it wants

    std::optional<extendedValue> import(std::string filepath);

    extendedValue parse(std::string filepath);
private:
    extendedValue parseFrag(std::string token);
};

// using translatorFunc = std::function<baseValue(std::string)>;

// std::unordered_map<std::string, translatorFunc> translatorMap;
