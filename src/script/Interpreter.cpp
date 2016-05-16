/*
 * (C) 2015-2016 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "Interpreter.h"

#include "util/FSUtil.h"
#include "util/StringUtil.h"
#include "Object.h"
#include "Stack.h"

#include <cassert>
#include <vector>

using std::string;
using std::vector;

namespace Script {

// AST
Object ASTNode::toObject(Stack* stack)
{
	if (type == Literal)
		return literal;
	if (type == Variable)
		return stack->get(variable);
	if (type == RawString) {
		std::string str;
		uint32_t line = 0;
		for (std::string::size_type i = 0; i < string.size(); i++) {
			char c = string[i];
			switch (c) {
			case '$':
				// FIXME/TODO: there's probably a way to implement eval() using this
				// but this is a build system, so screw it
				try {
					str += EvalVariableName(stack, string, line, i).toObject(stack).asStringRaw();
				} catch (Exception& e) {
					if (e.fLine == 0) {
						e.fFile = "(string dereferencing)";
						e.fLine = line;
					}
					throw e;
				}
			break;

			case '\n':
				line++;
				str += c;
				break;
			break;

			case '\\':
				i++;
				c = string[i];
				// fall through
			default:
				str += c;
			break;
			}
		}
		return StringObject(str);
	}
	throw Exception(Exception::InternalError,
		std::string("illegal conversion from ASTNode to Object, please file a bug!"));
}

// Parser

#define PARSER_PARAMS stack, code, line, i

#define NUMERIC_CASES \
	'0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': \
	case '8': case '9'
#define ALPHABET_CASES \
	'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H': \
	case 'I': case 'J': case 'K': case 'L': case 'M': case 'N': case 'O': case 'P': \
	case 'Q': case 'R': case 'S': case 'T': case 'U': case 'V': case 'W': case 'X': \
	case 'Y': case 'Z': case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': \
	case 'g': case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n': \
	case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u': case 'v': \
	case 'w': case 'x': case 'y': case 'z': case '_'
#define ALPHANUMERIC_CASES \
	ALPHABET_CASES: case NUMERIC_CASES
#define OPERS_CASES \
	'=': case '!': case '&': case '|': case '+': case '-': case '*': case '/': \
	case '%'
#define WHITESPACE_CASES \
	' ': case '#': case '\t': case '\n': case '\r'

#define UNEXPECTED_EOF Exception(Exception::SyntaxError, string("unexpected end of file"))
#define UNEXPECTED_TOKEN \
	Exception(Exception::SyntaxError, \
		string("unexpected token '").append(&code[i], 1).append("'"))
#define UNEXPECTED_TOKEN_EXPECTED(THING) \
	Exception(Exception::SyntaxError, \
		string("unexpected token '").append(&code[i], 1).append("' (expected '" THING "')"))

Object ParseAndEvalExpression(Stack* stack, const string& code, uint32_t& line, string::size_type& i);

// Returns whether or not it ignored whitespace
bool IgnoreWhitespace(Stack*, const string& code, uint32_t& line, string::size_type& i,
	bool ignoreComments = true)
{
	const string::size_type oldi = i;
	while (i < code.length()) {
		switch (code[i]) {
		case '#': // comment
			if (!ignoreComments)
				return oldi < i;
			// Ignore all following characters until next newline
			while (code[i] != '\n' && i < code.length())
				i++;
			// fall through
		case '\n':
			line++;
		break;
		case ' ':
		case '\t':
		case '\r':
			// Ignore.
		break;
		default:
			return oldi < i;
		}
		i++;
	}
	return oldi < i;
}

ASTNode EvalVariableName(Stack* stack, const string& code, uint32_t& line, string::size_type& i)
{
	assert(code[i] == '$');
	i++;

	ASTNode realRet(ASTNode::Variable);
	std::string ret = "";
	if (code[i] == '$') { // superglobal reference
		ret += '$';
		i++;
	}
	if (code[i] == '{') { // reference guard
		i++;
	}
	bool atEOE = false;
	while (!atEOE && i < code.length()) {
		char c = code[i];
		switch (c) {
		case ALPHANUMERIC_CASES:
			ret += c;
		break;

		case '[': {
			if (ret.size() > 0) {
				realRet.variable.push_back(ret);
				ret = "";
			}
			Object result = ParseAndEvalExpression(PARSER_PARAMS);
			CoerceOrThrow("expression return type", result, Type::String);
			realRet.variable.push_back(result.string);
		} break;

		case '.':
		case WHITESPACE_CASES: {
			if (ret.size() > 0) {
				realRet.variable.push_back(ret);
				ret = "";
			}
			if (IgnoreWhitespace(PARSER_PARAMS, false))
				i--;
		} break;

		case '}':
			i++;
			// fall through
		default:
			i--;
			if (ret.size() > 0)
				realRet.variable.push_back(ret);
			return realRet;
		break;
		}
		i++;
	}
	i--; // so we're 1 before the last in the str
	return realRet;
}

ASTNode ParseString(Stack*, const string& code, uint32_t& line, string::size_type& i,
	char endChar)
{
	string ret = "";
	i++;
	bool needs_dereferencing = false;
	while (i < code.length() && code[i] != endChar) {
		char c = code[i];
		switch (c) {
		case '\\':
			i++;
			c = code[i];
			switch (c) {
			case 'r': ret += '\r'; break;
			case 'n': ret += '\n'; break;
			case 't': ret += '\t'; break;
			default: ret += c; break;
			}
		break;

		case '\n':
			line++;
			ret += c;
		break;

		case '$':
			needs_dereferencing = true;
			ret += c;
		break;

		default:
			ret += c;
		break;
		}
		i++;
	}

	needs_dereferencing = needs_dereferencing && endChar != '\'';
	if (needs_dereferencing)
		return ASTNode(ASTNode::RawString, ret);
	return ASTNode(ASTNode::Literal, StringObject(ret));
}

Object ParseNumber(Stack*, const string& code, uint32_t&, string::size_type& i)
{
	string number = "";
	bool atEnd = false;
	number += code[i];
	while (!atEnd && i < code.length()) {
		i++;
		char c = code[i];
		switch (c) {
		case '-':
		case ALPHANUMERIC_CASES:
			number += c;
		break;
		case '.':
			throw Exception(Exception::TypeError, string("floating-point unsupported"));
		break;

		default:
			// End of number.
			atEnd = true;
			i--;
		break;
		}
	}

	int ret = std::stoi(number, nullptr, 10);
	return IntegerObject(ret);
}

Object ParseCallAndEval(Stack* stack, const string& code, uint32_t& line, string::size_type& i,
	const vector<string>& funcRef, bool variable = false)
{
	Function func;
	// Try to find the function
	if (variable) {
		Object o = stack->get(funcRef);
		CoerceOrThrow("referenced variable", o, Type::Function);
		func = *o.function;
	} else {
		auto funcIter = GlobalFunctions.find(funcRef[0]);
		if (funcIter == GlobalFunctions.end()) {
			throw Exception(Exception::SyntaxError, string("attempted to call function '")
				.append(funcRef[0]).append("' which does not exist"));
		}
		func = funcIter->second;
	}

	// Parse & build argument map
	assert(i != '(');
	i++;
	ObjectMap arguments;
	bool atEOC = false;
	int paramNum = 0;
	while (!atEOC) {
		// Parse an argument
		string paramName;

		auto commaBeforeColon = [&]() {
			string::size_type j = i;
			while (j < code.length()) {
				if (code[j] == ':')
					return false;
				if (code[j] == '(' || code[j] == ',' ||	code[j] == '[' ||
					code[j] == '$' || code[j] == '{')
					return true;
				j++;
			}
			throw UNEXPECTED_EOF;
		};

		bool treatAsTrueBoolean = false;
		if (paramNum == 0 && commaBeforeColon()) {
			paramName = "0";
			i--;
		} else {
			bool pastColon = false, pastEndOfParamName = false;
			while (!pastColon && i < code.length()) {
				if (IgnoreWhitespace(PARSER_PARAMS) && !paramName.empty())
					pastEndOfParamName = true;
				char c = code[i];
				switch (c) {
				case ALPHANUMERIC_CASES:
					if (pastEndOfParamName)
						throw UNEXPECTED_TOKEN;
					paramName += c;
				break;

				case '\'':
				case '"':
					if (pastEndOfParamName)
						throw UNEXPECTED_TOKEN;
					if (paramName.length() == 0) {
						paramName = ParseString(PARSER_PARAMS, c).toObject(stack).string;
						pastEndOfParamName = true;
					} else
						throw UNEXPECTED_TOKEN;
				break;

				case ')':
					i--;
				case ',':
					treatAsTrueBoolean = true;
					// fallthrough
				case ':':
					pastColon = true;
				break;

				default:
					throw UNEXPECTED_TOKEN;
				break;
				}
				i++;
			}
		}
		if (i >= code.length())
			throw UNEXPECTED_EOF;

		// TODO: any other keywords?
		if (paramName == "false" || paramName == "true" || paramName == "undefined")
			throw Exception(Exception::SyntaxError, "'" + paramName + "' is an illegal parameter name");

		if (!treatAsTrueBoolean)
			arguments.set(paramName, ParseAndEvalExpression(PARSER_PARAMS));
		else
			arguments.set(paramName, BooleanObject(true));

		if (code[i] == ')') {
			atEOC = true;
		} else {
			i++; paramNum++;
			IgnoreWhitespace(PARSER_PARAMS);
		}
	}

	Object* context = nullptr;
	if (funcRef.size() > 1) {
		vector<string> parentRef = funcRef;
		parentRef.pop_back();
		context = stack->get_ptr(parentRef);
	}
	return func.call(stack, context, arguments);
}

Object ParseList(Stack* stack, const string& code, uint32_t& line, string::size_type& i)
{
	assert(i != '[');
	i++;

	vector<Object>* ret = new vector<Object>;
	bool pastEndOfEntry = false, atEndOfList = false;
	while (!atEndOfList && i < code.length()) {
		IgnoreWhitespace(PARSER_PARAMS);
		char c = code[i];
		switch (c) {
		case '\'':
		case '"':
			if (pastEndOfEntry)
				throw UNEXPECTED_TOKEN_EXPECTED(",' or ']");
			ret->push_back(ParseString(PARSER_PARAMS, c).toObject(stack));
			pastEndOfEntry = true;
		break;

		case NUMERIC_CASES:
			if (pastEndOfEntry)
				throw UNEXPECTED_TOKEN_EXPECTED(",' or ']");
			ret->push_back(ParseNumber(PARSER_PARAMS));
			pastEndOfEntry = true;
		break;

		case 'u':
			if (code.substr(i, 9) == "undefined")
				ret->push_back(Object());
			else
				throw UNEXPECTED_TOKEN;
			i += 8;
		break;
		case 't':
			if (code.substr(i, 4) == "true")
				ret->push_back(BooleanObject(true));
			else
				throw UNEXPECTED_TOKEN;
			i += 3;
		break;
		case 'f':
			if (code.substr(i, 5) == "false")
				ret->push_back(BooleanObject(false));
			else
				throw UNEXPECTED_TOKEN;
			i += 4;
		break;

		case ',':
			pastEndOfEntry = false;
		break;

		case ']':
			atEndOfList = true;
			i--;
		break;

		default:
			throw UNEXPECTED_TOKEN;
		break;
		}
		i++;
	}
	if (i >= code.length())
		throw UNEXPECTED_EOF;
	return ListObject(ret);
}


class ReturnValue : public std::exception
{
public:
	ReturnValue(Object val) { value = val; }
	virtual ~ReturnValue() throw() {}
	virtual const char* what() const throw() { return "ReturnValue"; }

	Object value;
};

Object ParseAndEvalExpression(Stack* stack, const string& code, uint32_t& line, string::size_type& i)
{
	// Parse
	vector<ASTNode> expression;
	bool atEOE = false, isReturn = false;
	string::size_type start = i;

	IgnoreWhitespace(PARSER_PARAMS);
	while (!atEOE && i < code.length()) {
		switch (code[i]) {
		case '(':
			if (i == start)
				break;
			if (expression.size() > 0 && expression[expression.size() - 1].type == ASTNode::Variable)
				expression[expression.size() - 1] =
					ASTNode(ASTNode::Literal, ParseCallAndEval(PARSER_PARAMS,
						expression[expression.size() - 1].variable, true));
			else
				expression.push_back(ASTNode(ASTNode::Literal, ParseAndEvalExpression(PARSER_PARAMS)));
		break;
		case '[':
			if (i == start)
				break;
			// Assume list
			expression.push_back(ASTNode(ASTNode::Literal, ParseList(PARSER_PARAMS)));
		break;
		case ',':
		case ';':
		case ']':
		case ')':
			// End of expression.
			atEOE = true;
		break;

		case '$': // Variable reference.
			expression.push_back(EvalVariableName(PARSER_PARAMS));
		break;
		case '"': // String
			expression.push_back(ParseString(PARSER_PARAMS, '"'));
		break;
		case '\'': // String literal
			expression.push_back(ParseString(PARSER_PARAMS, '\''));
		break;
		case NUMERIC_CASES: // Number
			expression.push_back(ASTNode(ASTNode::Literal, ParseNumber(PARSER_PARAMS)));
		break;
		case ALPHABET_CASES: { // something else
			string thing;
			bool pastEndOfThing = false;
			while (!pastEndOfThing && i < code.length()) {
				char c = code[i];
				switch (c) {
				case ALPHANUMERIC_CASES:
					thing += c;
				break;

				case '(':
				default:
					pastEndOfThing = true;
				break;
				}
				i++;
			}
			i--;
			i--; // to get us back to last character of thing
			if (thing == "true")
				expression.push_back(ASTNode(ASTNode::Literal, BooleanObject(true)));
			else if (thing == "false")
				expression.push_back(ASTNode(ASTNode::Literal, BooleanObject(false)));
			else if (thing == "undefined")
				expression.push_back(ASTNode(ASTNode::Literal, Object()));
			else if (thing == "return") {
				if (expression.size() != 0)
					throw Exception(Exception::SyntaxError, string("incorrectly placed 'return'"));
				isReturn = true;
			} else { // assume function call
				i++;
				expression.push_back(ASTNode(ASTNode::Literal, ParseCallAndEval(PARSER_PARAMS, {thing})));
			}
		} break;

		case OPERS_CASES: {
			string oper = "";
			oper += code[i];
			bool atOperEnd = false;
			while (!atOperEnd && i < code.length()) {
				i++;
				switch (code[i]) {
				case OPERS_CASES:
					oper += code[i];
				break;
				default:
					atOperEnd = true;
					i--;
				break;
				}
			}
			if (expression.size() == 0 ||
				expression[expression.size() - 1].type == ASTNode::Operator) {
				throw Exception(Exception::SyntaxError, string("incorrectly placed operator"));
			}
			if (oper == "++" || oper == "--") {
				expression.push_back(ASTNode(ASTNode::Operator, string(&oper[0], 1).append("=")));
				expression.push_back(ASTNode(ASTNode::Literal, IntegerObject(1)));
			} else
				expression.push_back(ASTNode(ASTNode::Operator, oper));
		} break;

		default:
			throw UNEXPECTED_TOKEN;
		break;
		}
		if (!atEOE) {
			i++;
			IgnoreWhitespace(PARSER_PARAMS);
		}
	}

#define RETURN(x) if (isReturn) { throw ReturnValue(x); } else { return x; }
	if (expression.size() == 0) {
		RETURN(Object()); // undefined
	}
	if (expression.size() == 1) {
		RETURN(expression[0].toObject(stack));
	}
	if (expression[expression.size() - 1].type == ASTNode::Operator)
		throw Exception(Exception::SyntaxError, string("incorrectly placed operator"));

	// Evaluate
#define GET_OPERATOR_OR_CONTINUE \
	const ASTNode& node = expression[j]; \
	if (node.type != ASTNode::Operator) \
		continue; \
	const string& oper = node.string
#define UNKNOWN_OPERATOR \
	Exception(Exception::SyntaxError, \
		string("unknown operator '").append(oper).append("'"));
#define IMPLEMENT_OPERATOR(OP_NAME, TOKEN, TOKEQ_ALLOWED) { \
	j--; \
	Object result = Object::op_##OP_NAME(expression[j].toObject(stack), \
		expression[j + 2].toObject(stack)); \
	if (TOKEQ_ALLOWED && oper == (#TOKEN "=")) { \
		if (expression[j].type != ASTNode::Variable) \
			throw Exception(Exception::TypeError, string("the left-hand side of '" #TOKEN \
				"=' must be a variable")); \
		stack->set(expression[j].variable, result); \
	} else if (oper == (#TOKEN "=") && oper.length() > 1) \
		throw UNKNOWN_OPERATOR; \
	/* Now update the expression vector */  \
	expression[j] = ASTNode(ASTNode::Literal, result); \
	expression.erase(expression.begin() + j + 1, expression.begin() + j + 3); }

	// Pass 1: /, *
	for (vector<Object>::size_type j = 0; j < expression.size(); j++) {
		GET_OPERATOR_OR_CONTINUE;
		if (oper[0] == '/')
			IMPLEMENT_OPERATOR(/* operator name */ div,
							   /* token */ 	       /,
							   /* "TOKEN="? */     true)
		else if (oper[0] == '*')
			IMPLEMENT_OPERATOR(/* operator name */ mult,
							   /* token */ 	       *,
							   /* "TOKEN="? */     true)
	}
	// Pass 2: +, -
	for (vector<Object>::size_type j = 0; j < expression.size(); j++) {
		GET_OPERATOR_OR_CONTINUE;
		if (oper[0] == '-')
			IMPLEMENT_OPERATOR(/* operator name */ subt,
							   /* token */ 	       -,
							   /* "TOKEN="? */     true)
		else if (oper[0] == '+')
			IMPLEMENT_OPERATOR(/* operator name */ add,
							   /* token */ 	       +,
							   /* "TOKEN="? */     true)
	}
	// Pass 3: ==, !=
	for (vector<Object>::size_type j = 0; j < expression.size(); j++) {
		GET_OPERATOR_OR_CONTINUE;
		if (oper == "==")
			IMPLEMENT_OPERATOR(/* operator name */ eq,
							   /* token */ 	       ==,
							   /* "TOKEN="? */     false)
		else if (oper == "!=")
			IMPLEMENT_OPERATOR(/* operator name */ neq,
							   /* token */ 	       !=,
							   /* "TOKEN="? */     false)
	}
	// Pass 4: =
	for (vector<Object>::size_type j = 0; j < expression.size(); j++) {
		GET_OPERATOR_OR_CONTINUE;
		if (oper == "=") {
			j--;
			if (expression[j].type != ASTNode::Variable)
				throw Exception(Exception::TypeError,
					string("the left-hand side of '=' must be a variable"));
			Object result = expression[j + 2].toObject(stack);
			stack->set(expression[j].variable, result);
			/* Now update the expression vector */
			expression[j] = ASTNode(ASTNode::Literal, result);
			expression.erase(expression.begin() + j + 1, expression.begin() + j + 3);
		}
	}
	// Pass 5: &&, ||
	for (vector<Object>::size_type j = 0; j < expression.size(); j++) {
		GET_OPERATOR_OR_CONTINUE;
		if (oper == "&&")
			IMPLEMENT_OPERATOR(/* operator name */ and,
							   /* token */ 	       &&,
							   /* "TOKEN="? */     false)
		else if (oper == "||")
			IMPLEMENT_OPERATOR(/* operator name */ or,
							   /* token */ 	       ||,
							   /* "TOKEN="? */     false)
	}
	// Pass 6: throw if there are remaining operators
	for (vector<Object>::size_type j = 0; j < expression.size(); j++) {
		GET_OPERATOR_OR_CONTINUE;
		throw UNKNOWN_OPERATOR;
	}
	if (expression.size() != 1)
		throw Exception(Exception::TypeError,
			string("evaluated expression does not have 1 return value"));
	RETURN(expression[0].toObject(stack));
#undef RETURN
}

Object Run(Stack* stack, string path)
{
	string filename;
	if (FSUtil::isFile(path))
		filename = path;
	else if (FSUtil::isFile(path = FSUtil::combinePaths({path, "Phoenixfile.phnx"})))
		filename = path;
	if (filename.empty())
		throw Exception(Exception::FileDoesNotExist, path);
	string code = FSUtil::getContents(filename);
	stack->pushDir(FSUtil::parentDirectory(filename));

	uint32_t line = 1;
	string::size_type i = 0;
	try {
		IgnoreWhitespace(PARSER_PARAMS);
		while (i < code.length()) {
			ParseAndEvalExpression(PARSER_PARAMS);
			i++;
			IgnoreWhitespace(PARSER_PARAMS);
		}
	} catch (ReturnValue& e) {
		stack->popDir();
		return e.value;
	} catch (Exception& e) {
		stack->popDir();
		if (e.fLine == 0) {
			e.fFile = filename;
			e.fLine = line;
		}
		throw e;
	}
	stack->popDir();
	return Object(); // undefined
}

}
