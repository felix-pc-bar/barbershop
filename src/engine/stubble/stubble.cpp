#include <iostream>
//#include <sstream>
#include <fstream>
#include <optional>
#include <ostream>
#include <filesystem>
#include <algorithm>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

#include "stubble.h"
#include "typemap.h"
#include "../helpers/stringy.h"
#include "types.h"
#include "buildermap.h"
#include "writers.h"
#include "../material.h"
#include "../general3d.h"

std::function<bool(const extendedValue&)> getTypeMatcher(TypeData t)
{
	switch (t.type)
	{
	case TypesEnum::Int:
		return [](const extendedValue& v)
		{
			return std::holds_alternative<int>(v);
		};
	case TypesEnum::Bool:
		return [](const extendedValue& v)
		{
			return std::holds_alternative<bool>(v);
		};
	case TypesEnum::Float:
		return [](const extendedValue& v)
		{
			return std::holds_alternative<float>(v);
		};
	case TypesEnum::StdString:
		return [](const extendedValue& v)
		{
			return std::holds_alternative<std::string>(v);
		};
	case TypesEnum::_Colour:
		return [](const extendedValue& v)
		{
			return std::holds_alternative<Colour*>(v);
		};
	case TypesEnum::_Material:
		return [](const extendedValue& v)
		{
			return std::holds_alternative<Material*>(v);
		};
	case TypesEnum::_Object3D:
		return [](const extendedValue& v)
		{
			return std::holds_alternative<Object3D*>(v);
		};
	case TypesEnum::_Position3D:
		return [](const extendedValue& v)
		{
			return std::holds_alternative<Position3d*>(v);
		};
	case TypesEnum::_Quaternion:
		return [](const extendedValue& v)
		{
			return std::holds_alternative<Quaternion*>(v);
		};
	case TypesEnum::_Camera:
		return [](const extendedValue& v)
		{
			return std::holds_alternative<Camera*>(v);
		};
	case TypesEnum::_Scene:
		return [](const extendedValue& v)
		{
			return std::holds_alternative<Scene*>(v);
		};
	default:
		return [](const extendedValue& v)
		{
			return false; // Nothing matches an unknown type (best-effort, avoid abort)
		};
	}
}

// Bridge between runtime std::variant data and static enum class types
bool matchesType(const extendedValue& val, TypeData t)
{
	auto matcher = getTypeMatcher(t);
	if (!t.isVector)
	{
		return matcher(val);
	}
	else
	{
		if (!std::holds_alternative<Pyjama*>(val)) { return false; } // not a vector as stipulated
		auto pj = std::get<Pyjama*>(val);
		for (auto x : pj->v)
		{
			if (!matcher(x)) { return false; } // an item doesn't match
		}
		// made it thru all items
		return true;
	}
}

std::string_view getTypeName(const extendedValue& ob)
{
	return std::visit([]<typename T>(const T&)
	{
		return TypeInfo<T>::name;
	}, ob);
}

builderFunctionEntry::builderFunctionEntry(std::vector<std::pair<std::vector<TypeData>, builderFunction>> bldLs): builders(bldLs) {}

TypeData::TypeData(TypesEnum t, bool isvec): type(t), isVector(isvec) {}

StubbleParser::SyntacticalBranch::SyntacticalBranch(std::string d): data(d) { }

std::string StubbleParser::SyntacticalBranch::serialise()
{
	std::string result;
	result.append(this->data);
	if (this->children.size() != 0)
	{
		result.append("(");
		for (auto child : this->children)
		{
			result.append(child.serialise());
			result.append(", ");
		}
		result.resize(result.size() - 2); // remove the last comma+space
	
		result.append(")");
	}

	return result;
}

// TODO: add support for escaped quotes (we don't atm i think)
std::string getDataToken(std::istream& stream)
{
	std::string result;
	char c;
	bool inQuotes = false; // is current char inside quotes
	while (true)
	{
		c = stream.get();
		if (c == EOF) { break; }
		if (c == '\"') { inQuotes = !inQuotes; }
		if (std::string("()[],").find_first_of(c) != std::string::npos && !inQuotes)
		{
			break;
		}
		result += static_cast<char>(c);
	}
	stream.unget();
	return result;
}

// We store data and token type- this is duplicitous for the punctuators, as the type always corresponds
// to the same data, but this way we are futureproofed a bit for writing out a file.
StubbleParser::Token::Token(TokenType tt, unsigned int linenum, std::string d): ttype(tt), lineNumber(linenum), data(d)
{
	switch (tt)
	{
	case TokenType::brOpen:
		data = "(";
		break;
	
	case TokenType::brClose:
		data = ")";
		break;
	
	case TokenType::sqBrOpen:
		data = "[";
		break;
	
	case TokenType::sqBrClose:
		data = "]";
		break;

	case TokenType::comma:
		data = ",";
		break;
	
	default:
		break;
	}
}

void StubbleParser::Token::unexpected()
{
	std::cout << "Unexpected token \"" + data + "\" at line " + std::to_string(lineNumber) << std::endl;
	return;
}

StubbleParser::TokenStream::TokenStream(): streamLocation(-1) {} // We init streamloc to -1 because we incr it before reading (trust bro)

std::optional<StubbleParser::Token> StubbleParser::TokenStream::consume(std::vector<StubbleParser::TokenType> validTypes)
{
	streamLocation++;
	if (std::find(validTypes.begin(), validTypes.end(), tokens[streamLocation].ttype) != validTypes.end())
	{
		return tokens.at(streamLocation); // throws std::out_of_range
	}
	else // Token is not of valid type
	{
		tokens[streamLocation].unexpected();
		return std::nullopt;
	}
}

StubbleParser::Token* StubbleParser::TokenStream::peek()
{
	if (streamLocation == tokens.size()) { throw std::out_of_range("Pointer is at end of vector"); }
	return &tokens.at(streamLocation + 1);
}

// This function takes a TokenStream.
// It wants the first token in pops to be a data-
// either because it's a leaf, and the data is... the data
// or because it's the identifier for an object.
// If the parse is successful (doesn't return std::nullopt),
// it will return leaving the next token to be popped being a comma, a close bkt, or EOF.
// otherwise, no promises.
// If branchName is passed, we will try to return either a leaf, or a branch with the passed identifier
std::optional<StubbleParser::SyntacticalBranch> StubbleParser::graftFrag(StubbleParser::TokenStream& ts, bool parent, std::optional<StubbleParser::Token> idt)
{
	std::optional<StubbleParser::Token> idToken = idt;
	bool isVectorInner = true; // this branch is a direct child of a vector- treat differently
	StubbleParser::SyntacticalBranch result;
	if (!idToken.has_value())
	{
		isVectorInner = false;
		idToken = ts.consume({StubbleParser::TokenType::data}); // Pop a data
		if (!idToken.has_value()) { return std::nullopt; }
	}

	result.data = idToken.value().data;
	result.lineNumber = idToken.value().lineNumber;
	result.isVector = false;

	try
	{
		auto cc = ts.peek();

		switch (cc->ttype)
		{
			case TokenType::data:
			{
				if (isVectorInner)
				{
					// inner vector leaf
					ts.streamLocation++; // we didn't consume the data
					result.data = cc->data;
					return result;
				}
				else
				{
					cc->unexpected();
					return std::nullopt;
				}
			}

			case TokenType::comma:
				// Non-final leaf case
				// We peeked the comma- return and let parent see/handle it
				return result;

			case TokenType::sqBrClose:
			case TokenType::brClose:
				// Final leaf case
				// Likewise, let parent deal with close bkt
				return result;

			case TokenType::brOpen: { // Curly braces to declare a new scope for the int here
				ts.streamLocation++; // We peeked the open bracket, consume it
				for (;;) // Broken out of
				{
					// Since we skipped the peeked open bracket, we should have a data next up
					std::optional<StubbleParser::SyntacticalBranch> returnedBranch = graftFrag(ts, false);
					if (returnedBranch.has_value())
					{
						result.children.emplace_back(returnedBranch.value());
					}
					else
					{
						// Propogate
					    return std::nullopt;
					}

					// Valid tokens after a child branch:
					// Comma -> continue reading children
					// Close bracket -> end of children list
					// (Nominally we don't exit pointing to any other tokens)
					auto next = ts.consume({ TokenType::comma, TokenType::brClose});
					if (!next.has_value())
					{
						// Error: probably there was an error when parsing the last child branch
						return std::nullopt;
					}

					// Break out of child-getting loop
					if (next.value().ttype == TokenType::brClose) { break; }
					// Don't do anything for comma, we just wanted to consume it
				}
				if (parent && ts.streamLocation + 1 != ts.tokens.size())
				{
					std::cout << "Warning: Trailing token(s) found after final closing bracket. Only one object may be imported at a time." << std::endl;
				}
				return result;
			}

			case TokenType::sqBrOpen: { // Curly braces to declare a new scope
				ts.streamLocation++; // We peeked the open bracket, consume it
				result.isVector = true;
				for (;;) // Broken out of
				{
					// Since we have a vector, we should be straight into either leaf data or children lists
					// We pass the ID token of this branch along
					std::optional<StubbleParser::SyntacticalBranch> returnedBranch = graftFrag(ts, false, idToken);
					if (returnedBranch.has_value())
					{
						result.children.emplace_back(returnedBranch.value());
					}
					else
					{
						// Propogate
					    return std::nullopt;
					}

					// Valid tokens after a child branch:
					// Comma -> continue reading children
					// Close bracket -> end of children list
					// (Nominally we don't exit pointing to any other tokens)
					auto next = ts.consume({ TokenType::comma, TokenType::sqBrClose});
					if (!next.has_value())
					{
						// Error: probably there was an error when parsing the last child branch
						return std::nullopt;
					}

					// Break out of child-getting loop
					if (next.value().ttype == TokenType::sqBrClose) { break; }
					// Don't do anything for comma, we just wanted to consume it
				}
				if (parent && ts.streamLocation + 1 != ts.tokens.size())
				{
					std::cout << "Warning: Trailing token(s) found after final closing bracket. Only one object may be imported at a time." << std::endl;
				}
				return result;
			}
			default:
			    std::cout << "Fatal error: invalid control character. Cannot continue parsing file." << std::endl;
			    return std::nullopt;
		}
	// ts.peek() and ts.consume() throw exceptions if we read too far
	} catch ( std::out_of_range ) {
		std::cout << "Error: hit EOF early. You may be missing trailing punctuators." << std::endl;
		return std::nullopt;
	}
}

std::optional<extendedValue> StubbleParser::translateTree(StubbleParser::SyntacticalBranch& ast)
{
	if (ast.isVector)
	{
		return translateVector(ast);
	}

	if (ast.children.size() == 0) // leaf
	{
		if (ast.data == "true") { return true; }
		if (ast.data == "false") { return false; }
		if (helpers::isDigits(ast.data))
		{
			return stoi(ast.data);
		}
		auto fpResult = helpers::getFloatLiteral(ast.data);
		if (fpResult.has_value())
		{
			return fpResult.value();
		}
		auto stResult = helpers::getStringLiteral(ast.data);
		if (stResult.has_value())
		{
			return stResult.value();
		}
		// Couldn't match to any base type
		std::cout << "Error: leaf branch \"" << ast.data << "\" at line " << ast.lineNumber << " couldn't be matched to a base type." << std::endl;
		return std::nullopt;
	}
	else
	{
		std::vector<extendedValue> translatedChildren;
		for (auto branch : ast.children)
		{
			auto br = translateTree(branch);
			if (!br.has_value())
			{
				// Propograte error
				return std::nullopt;
			}
			translatedChildren.emplace_back(br.value());
		}
		auto builtOb = StubbleParser::getBuiltObject(ast.data, translatedChildren);
		if (!builtOb.has_value())
		{
			// Propogate error
			std::cout << "Error: Couldn't build object \"" << ast.data << "\" at line " << ast.lineNumber << std::endl;
			return std::nullopt;
		}

		auto obptr = builtOb.value();
		//convert to extendedValue
		extendedValue result = std::visit([](auto&& value) -> extendedValue {
			return value;
		}, obptr);

		return result;
	}
}

std::optional<objectPointer> StubbleParser::getBuiltObject(std::string typeName, std::vector<extendedValue> params)
{
	try
	{
		// ==== Match parameters to a builder function ====

		auto funcEntry = builderLookup.at(typeName);

		builderFunction bf = nullptr;

		for (size_t i = 0; i < funcEntry.builders.size(); i++)
		{
			// No point checking if individual params match if there's different numbers of them
			if (params.size() == funcEntry.builders[i].first.size())
			{
				auto wantedParamTypeList = funcEntry.builders[i].first;
				bool match = true;

				for (size_t j= 0; j < wantedParamTypeList.size(); j++)
				{
					if (!matchesType(params[j], wantedParamTypeList[j]))
					{
						match = false;
						break;
					}
				} // loop of each parameter

				if (match == true)
				{
					// Loop ws completed with all params matching
					// Break out of main loop with the function we found
					bf = funcEntry.builders[i].second;
					break;
				}
			}
		} // </loop of potential builders>

		if (bf == nullptr) // None of the options got all the way thru without finding a false type
		{
			std::cout << "Error: Couldn't match parameters for construction of \"" << typeName << "\" to any builder" << std::endl;
			std::string paramerr = "Parameters seen: ";
			for (auto param : params)
			{
				paramerr += std::string(getTypeName(param)) + std::string(", ");
			}
			paramerr.resize(paramerr.size() - 1); // remove trailing comma
			std::cout << paramerr << std::endl;
			return std::nullopt;
		}

		auto result = bf(params);
		return result;
	}
	catch (std::out_of_range)
	{
		std::cout << "Error: type \"" << typeName << "\" is not supported." << std::endl;
		return std::nullopt;
	}
	catch (std::invalid_argument)
	{
		std::cout << "Error: passed invalid arguments in construction of \"" << typeName << "\"" << std::endl;
		// TODO: Add reporting for wanted/given parameter types
		return std::nullopt;
	}
	catch ( ... )
	{
		std::cout << "Error: Could not import \"" << typeName << "\"" << std::endl;
		return std::nullopt;
	}
}

std::optional<Pyjama*> StubbleParser::translateVector(SyntacticalBranch& ast)
{
	std::vector<extendedValue> result;
	if (ast.data == "int")
	{
		for (auto branch : ast.children)
		{
			if (!helpers::isDigits(branch.data) || branch.children.size() != 0)
			{
				std::cout << "Error: encountered non-int type when translating vector<int> on line " << ast.lineNumber << std::endl;
				return std::nullopt;
			}
			result.emplace_back(stoi(branch.data));
		}
	}
	else if (ast.data == "float")
	{
		for (auto branch : ast.children)
		{
			auto rf = helpers::getFloatLiteral(branch.data);
			if (rf == std::nullopt || branch.children.size() != 0)
			{
				std::cout << "Error: encountered non-float type when translating vector<float> on line " << ast.lineNumber << std::endl;
				return std::nullopt;
			}
			result.emplace_back(rf.value());
		}
	}
	else if (ast.data == "bool")
	{
		for (auto branch : ast.children)
		{
			if ((branch.data != "true" && branch.data != "false") || branch.children.size() != 0)
			{
				std::cout << "Error: encountered non-bool type when translating vector<bool> on line " << ast.lineNumber << std::endl;
				return std::nullopt;
			}
			result.emplace_back(branch.data == "true" ? true : false);
		}
	}
	else if (ast.data == "string")
	{
		for (auto branch : ast.children)
		{
			if (branch.children.size() != 0)
			{
				std::cout << "Error: encountered non-string type when translating vector<string> on line " << ast.lineNumber << std::endl;
				return std::nullopt;
			}
			result.emplace_back(branch.data);
		}
	}
	else
	{
		for (auto branch : ast.children)
		{
			auto ro = translateTree(branch);
			if (ro == std::nullopt)
			{
				return std::nullopt;
			}
			result.emplace_back(ro.value());
		}
	}
	Pyjama* pj = new Pyjama();
	pj->v = result;
	return pj;
}

std::optional<extendedValue> StubbleParser::import(std::string filepath)
{
	if (!std::filesystem::exists(filepath))
	{
	    std::cout << "Error: file does not exist." << std::endl; 
	    return std::nullopt;
	}

    std::ifstream stbstream(filepath); // read file into ifstream
	TokenStream ts;

	auto isntWhitespace = [](char c) 
	{
		return (std::string(" \t\r\f\v").find_first_of(c) == std::string::npos);
	};
	auto isPunctuator = [](char c) 
	{
		return (std::string("(),").find_first_of(c) != std::string::npos);
	};

	// ==== Tokenise ====
	// Performs lexical analysis on input file.
	// make a flat vector of tokens representing the file
	// no error checking here- that's later

	std::string currentData; // stores characters up to next punctuator
	char c;
	unsigned int currentline = 1;

	for (;;)
	{
		helpers::readUntil(stbstream, isntWhitespace); // Skip whitespace
		if (stbstream.peek() == EOF) { break; }
		if (stbstream.peek() == '\n') { currentline++; stbstream.get(); } //Skip newline but incr counter
		currentData = getDataToken(stbstream);
		if (currentData.empty())
		{
			c = stbstream.get();	
			switch (c)
			{
			case '(':
				ts.tokens.emplace_back(TokenType::brOpen, currentline);
				break;
			
			case ')':
				ts.tokens.emplace_back(TokenType::brClose, currentline);
				break;
			
			case '[':
				ts.tokens.emplace_back(TokenType::sqBrOpen, currentline);
				break;
			
			case ']':
				ts.tokens.emplace_back(TokenType::sqBrClose, currentline);
				break;

			case ',':
				ts.tokens.emplace_back(TokenType::comma, currentline);
				break;
			}
		}
		else
		{
			// Strip leading/trailing whitespace on individual token (this is the data path)
			auto start = std::find_if_not(currentData.begin(), currentData.end(), [](unsigned char c) {
				return std::isspace(c);
		    });
	
		    auto end = std::find_if_not(currentData.rbegin(), currentData.rend(), [](unsigned char c) {
				return std::isspace(c);
		    }).base();
		
		    if (start >= end) { continue; } // This skips adding a token, because it would be an empty data
		    else
			{
				currentData = std::string(start, end);
			}
			ts.tokens.emplace_back(TokenType::data, currentline, currentData);
		}
	}

	// ==== Build AST ====
	// Next, we build a heirachical structure from the lexemes/tokens 
	// This is where structural error checking is done
	
	auto AST = graftFrag(ts);

	if (!AST.has_value())
	{
		std::cout << "Error when grafting \"" << filepath << "\"\n";
		return std::nullopt;
	}

	auto result = translateTree(AST.value());
	if (!result.has_value())
	{
		std::cout << "Error when translating \"" << filepath << "\"\n";
	}
	return result;
}

void StubbleParser::stbExport(extendedValue ob, std::string filepath)
{
	SyntacticalBranch AST{}; // most vexing parse !1!!
	// AST.data = getTypeName(ob);
	// writerFunction wf = writerLookup.at(std::string(getTypeName(ob)));
	// wf(ob, &AST);
	writeObToAST(ob, &AST);
	std::ofstream savedFile(filepath);

	savedFile << AST.serialise();

	savedFile.close();
}
