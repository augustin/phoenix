/*
 * (C) 2015 Augustin Cavalier
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "Parser.h"

#include "util/FSUtil.h"
#include "util/StringUtil.h"
#include "Object.h"
#include "ObjectMap.h"
#include "Stack.h"

#include <cmath>
#include <iostream>
#include <fstream>
#include <vector>

using std::string;
using std::vector;

namespace Language {

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

#define IS_WHITESPACE(THING) ( \
	THING == ' ' || \
	THING == '#' || \
	THING == '\t' || \
	THING == '\n' || \
	THING == '\r')

#define UNEXPECTED_EOF Exception(Exception::SyntaxError, string("unexpected end of file"))
#define UNEXPECTED_TOKEN \
	Exception(Exception::SyntaxError, \
		string("unexpected token '").append(&code[i], 1).append("'"))
#define UNEXPECTED_TOKEN_EXPECTED(THING) \
	Exception(Exception::SyntaxError, \
		string("unexpected token '").append(&code[i], 1).append("' (expected '" THING "')"))

Object ParseAndEvalExpression(Stack* stack, const string& code, uint32_t& line, string::size_type& i);

void IgnoreWhitespace(Stack* stack, const string& code, uint32_t& line, string::size_type& i)
{
	while (i < code.length()) {
		switch (code[i]) {
		case '#': // comment
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
			return;
		}
		i++;
	}
}

ASTNode EvalVariableName(Stack* stack, const string& code, uint32_t& line, string::size_type& i)
{
	// TODO: assert code[i] == '$'
	i++;

	ASTNode realRet(ASTNode::Variable);
	std::string ret = "";
	if (code[i] == '$') { // superglobal reference
		ret += '$';
		i++;
	}
	if (code[i] == '[') {
		// Dereference.
		Object result = ParseAndEvalExpression(PARSER_PARAMS);
		Language_COERCE_OR_THROW("expression return type", result, String);
		realRet.variable.push_back(ret + result.string);
		return realRet;
	} else {
		while (i < code.length()) {
			char c = code[i];
			switch (c) {
			case ALPHANUMERIC_CASES:
				ret += c;
			break;

			default:
				// end of variable name
				i--;
				realRet.variable.push_back(ret);
				return realRet;
			break;
			}
			i++;
		}
		throw UNEXPECTED_EOF;
	}
}

ASTNode ParseString(Stack* stack, const string& code, uint32_t& line, string::size_type& i,
	char starter)
{
	string ret = "";
	i++;
	bool needs_dereferencing = false;
	while (i < code.length() && code[i] != starter) {
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

		case '$':
			needs_dereferencing = true;
			// fall through
		default:
			ret += c;
		break;
		}
		i++;
	}

	needs_dereferencing = needs_dereferencing && starter != '\'';
	if (needs_dereferencing)
		return ASTNode(ASTNode::RawString, ret);
	return ASTNode(ASTNode::Literal, StringObject(ret));
}

Object ParseNumber(Stack* stack, const string& code, uint32_t& line, string::size_type& i)
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

	int ret = strtol(number.c_str(), NULL, 10);
	return IntegerObject(ret);
}

Object ParseCallAndEval(Stack* stack, const string& code, uint32_t& line, string::size_type& i)
{
	// Parse function name
	string funcName;
	bool pastEndOfFuncName = false;
	while (!pastEndOfFuncName && i < code.length()) {
		char c = code[i];
		switch (c) {
		case ALPHANUMERIC_CASES:
			funcName += c;
		break;
		case '(':
			pastEndOfFuncName = true;
		break;
		default:
			throw UNEXPECTED_TOKEN;
		break;
		}
		string::size_type iBefore = ++i;
		IgnoreWhitespace(PARSER_PARAMS);
		if (i > iBefore)
			pastEndOfFuncName = true;
	}
	if (i >= code.length())
		throw UNEXPECTED_EOF;

	// Check if this function really exists
	auto funcIter = GlobalFunctions.find(funcName);
	if (funcIter == GlobalFunctions.end()) {
		throw Exception(Exception::SyntaxError, string("attempted to call function '")
			.append(funcName).append("' which does not exist"));
	}
	Function func = funcIter->second;

	// Parse & build argument map
	ObjectMap arguments;
	bool atEOC = false;
	bool paramNum = 0;
	while (!atEOC) {
		// Parse an argument
		string paramName;

		auto commaBeforeColon = [&]() {
			string::size_type j = i;
			while (j < code.length()) {
				if (code[j] == ':')
					return false;
				if (code[j] == '(' || code[j] == ',' ||
					code[j] == '$' || code[j] == '{')
					return true;
				j++;
			}
			throw UNEXPECTED_EOF;
		};
		if (paramNum == 0 && commaBeforeColon()) {
			paramName = "0";
		} else {
			bool pastEndOfParamName = false;
			while (!pastEndOfParamName && i < code.length()) {
				char c = code[i];
				switch (c) {
				case ALPHANUMERIC_CASES: // FIXME: should begin with letter?
					paramName += c;
				break;
				case ':':
					pastEndOfParamName = true;
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
		arguments.set(paramName, ParseAndEvalExpression(PARSER_PARAMS));
		if (code[i] == ')') {
			atEOC = true;
		} else {
			i++; paramNum++;
			IgnoreWhitespace(PARSER_PARAMS);
		}
	}
	return func.call(arguments);
}

Object ParseAndEvalExpression(Stack* stack, const string& code, uint32_t& line, string::size_type& i)
{
	// Parse
	vector<ASTNode> expression;
	bool atEOE = false;
	string::size_type start = i;
	IgnoreWhitespace(PARSER_PARAMS);
	while (!atEOE && i < code.length()) {
		switch (code[i]) {
		case '[':
		case '(':
			if (i != start)
				expression.push_back(ASTNode(ASTNode::Literal, ParseAndEvalExpression(PARSER_PARAMS)));
		break;
		case ',':
		case ';':
		case ']':
		case ')': // End of expression.
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
		case ALPHABET_CASES: // Function call
			expression.push_back(ASTNode(ASTNode::Literal, ParseCallAndEval(PARSER_PARAMS)));
		break;
		case NUMERIC_CASES: // Number
			expression.push_back(ASTNode(ASTNode::Literal, ParseNumber(PARSER_PARAMS)));
		break;

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
	if (expression.size() == 0)
		return Object(); // undefined
	if (expression.size() == 1)
		return expression[0].toObject(stack);
	if (expression[expression.size() - 1].type == ASTNode::Operator)
		throw Exception(Exception::SyntaxError, string("incorrectly placed operator"));

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
		stack->set(expression[j].string, result); \
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
			stack->set(expression[j].variable[0], result); // FIXME for objects!
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
	return expression[0].toObject(stack);
}

void Run(Stack* stack, string path)
{
	string filename;
	if (FSUtil::exists(path)) {
		filename = path;
	} else if (FSUtil::exists(path = FSUtil::combinePaths({path, "Phoenixfile"}))) {
		filename = path;
	}
	if (filename.empty())
		throw Exception(Exception::FileDoesNotExist, path);
	string code; {
		std::ifstream filestream(filename);
		// extra ()s here are mandatory
		code = string((std::istreambuf_iterator<char>(filestream)),
			std::istreambuf_iterator<char>());
	}

	uint32_t line = 1;
	string::size_type i = 0;
	try {
		IgnoreWhitespace(PARSER_PARAMS);
		while (i < code.length()) {
			std::cout << ParseAndEvalExpression(PARSER_PARAMS).asString();
			i++;
			IgnoreWhitespace(PARSER_PARAMS);
		}
	} catch (Exception& e) {
		// FIXME: adapt for multifile
		e.fFile = filename;
		e.fLine = line;
		throw e;
	}
}

}
