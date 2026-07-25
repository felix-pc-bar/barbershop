#pragma once
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
		sqBrOpen,
		sqBrClose,
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

        void unexpected();
    };

	class TokenStream
	{
	public:
		std::vector<Token> tokens;
		int streamLocation;

		TokenStream();
		std::optional<Token> consume(std::vector<TokenType> validTypes);
		Token* peek();
	};

    // AST branch object.
    // if no children, this branch is a "leaf" and 'data' stores the string to be converted
    // otherwise, 'data' is an identifier for the class that needs to be constructed with the parameters stored in chidren
    class SyntacticalBranch
    {
    public:
        SyntacticalBranch() = default;
		SyntacticalBranch(std::string d); // construct a leaf

		std::string serialise(); // convert self to string form

        std::string data;
		bool isVector; // is this branch a vector (requires different translation rules)
        std::vector<SyntacticalBranch> children;
        unsigned int lineNumber; // Line number of data tag
    };

	// If we're doing a vector, we want to pass a branch name and not be looking for one before the children list
	static std::optional<SyntacticalBranch> graftFrag(TokenStream& ts, bool parent = true, std::optional<Token> idToken = std::nullopt);

	static std::optional<extendedValue> translateTree(SyntacticalBranch& ast);
	static std::optional<Pyjama*> translateVector(SyntacticalBranch& ast);

	static std::optional<objectPointer> getBuiltObject(std::string typeName, std::vector<extendedValue> params);

    // Stores function pointer alongside arg types it wants

    static std::optional<extendedValue> import(std::string filepath);

	// Write object to stubble file
	static void stbExport(extendedValue ob, std::string filepath);

    static extendedValue parse(std::string filepath);
private:
    extendedValue parseFrag(std::string token);
};

// using translatorFunc = std::function<baseValue(std::string)>;

// std::unordered_map<std::string, translatorFunc> translatorMap;
using writerFunction = std::function<void(extendedValue, StubbleParser::SyntacticalBranch*)>;
