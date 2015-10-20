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
#define ALPHANUMERIC_CASES \
	'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H': \
	case 'I': case 'J': case 'K': case 'L': case 'M': case 'N': case 'O': case 'P': \
	case 'Q': case 'R': case 'S': case 'T': case 'U': case 'V': case 'W': case 'X': \
	case 'Y': case 'Z': case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': \
	case 'g': case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n': \
	case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u': case 'v': \
	case 'w': case 'x': case 'y': case 'z': case NUMERIC_CASES: case '_'
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

string EvalVariableName(Stack* stack, const string& code, uint32_t& line, string::size_type& i)
{
	// TODO: assert code[i] == '$'
	i++;

	string ret = "";
	if (code[i] == '$') { // superglobal reference
		ret += '$';
		i++;
	}
	if (code[i] == '[') {
		// Dereference.
		Object result = ParseAndEvalExpression(PARSER_PARAMS);
		Language_COERCE_OR_THROW("expression return type", result, String);
		return ret + *result.string;
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
				return ret;
			break;
			}
			i++;
		}
		throw UNEXPECTED_EOF;
	}
}

Object ParseString(Stack* stack, const string& code, uint32_t& line, string::size_type& i,
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
	Object str = StringObject(ret);
	str.string_dereferenced = !needs_dereferencing && starter != '\'';
	return str;
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

	// FIXME: locale
	int ret = strtol(number.c_str(), NULL, 10);
	return IntegerObject(ret);
}

Object ParseCallAndEval(Stack* stack, const string& code, uint32_t& line, string::size_type& i)
{
	string funcname;
	ObjectMap arguments;
	bool past_end = false, past_paren = false;

	// Parse function name
	while (!past_paren && i < code.length()) {
		char c = code[i];
		switch (c) {
		case ALPHANUMERIC_CASES:
			if (past_end)
				throw UNEXPECTED_TOKEN_EXPECTED("(");
			funcname += c;
		break;

		case '#': // comment
			// Ignore all following characters until next newline
			while (code[i] != '\n' && i < code.length())
				i++;
			// fall through
		case '\n':
			line++;
			// fall through
		case ' ':
		case '\t':
		case '\r':
			past_end = true;
		break;

		case '(':
			past_paren = true;
		break;

		default:
			throw UNEXPECTED_EOF;
		break;
		}
		i++;
	}
	if (i >= code.length()) {
		throw Exception(Exception::SyntaxError, string("unexpected end of file"));
	}

	// Check if this function really exists
	if (GlobalFunctions.find(funcname) == GlobalFunctions.end()) {
		throw Exception(Exception::SyntaxError, string("attempted to call function '")
			.append(funcname).append("' which does not exist"));
	}
	throw Exception(Exception::SyntaxError, funcname);

	// Parse & build argument map
}

Object ParseAndEvalExpression(Stack* stack, const string& code, uint32_t& line, string::size_type& i)
{
	// Parse
	vector<Object> expression;
	bool atEOE = false;
	string::size_type start = i;
	IgnoreWhitespace(PARSER_PARAMS);
	while (!atEOE && i < code.length()) {
		switch (code[i]) {
		case '[':
		case '(':
			if (i != start)
				expression.push_back(ParseAndEvalExpression(PARSER_PARAMS));
		break;
		case ';':
		case ']':
		case ')': // End of expression.
			atEOE = true;
		break;

		case '$': // Variable reference.
			expression.push_back(VariableObject(EvalVariableName(PARSER_PARAMS)));
		break;
		case '"': // String
			expression.push_back(ParseString(PARSER_PARAMS, '"'));
		break;
		case '\'': // String literal
			expression.push_back(ParseString(PARSER_PARAMS, '\''));
		break;
		case NUMERIC_CASES: // Number
			expression.push_back(ParseNumber(PARSER_PARAMS));
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
				expression[expression.size() - 1].type() == Type::Operator) {
				throw Exception(Exception::SyntaxError, string("incorrectly placed operator"));
			}
			expression.push_back(OperatorObject(oper));
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
		return Object();
	if (expression[expression.size() - 1].type() == Type::Operator)
		throw Exception(Exception::SyntaxError, string("incorrectly placed operator"));
	if (expression.size() == 1)
		return Language_POSSIBLY_DEREFERENCE(expression[0]);

#define GET_OPERATOR_OR_CONTINUE \
	const Object& obj = expression[j]; \
	if (obj.type() != Type::Operator) \
		continue; \
	const string& oper = *obj.string
#define UNKNOWN_OPERATOR \
	Exception(Exception::SyntaxError, \
		string("unknown operator '").append(oper).append("'"));
#define IMPLEMENT_OPERATOR(OP_NAME, TOKEN, TOKEQ_ALLOWED) { \
	j--; \
	Object result = Object::op_##OP_NAME(Language_POSSIBLY_DEREFERENCE(expression[j]), \
		Language_POSSIBLY_DEREFERENCE(expression[j + 2])); \
	if (TOKEQ_ALLOWED && oper == (#TOKEN "=")) { \
		if (expression[j].type() != Type::Variable) \
			throw Exception(Exception::TypeError, string("the left-hand side of '" #TOKEN \
				"=' must be a variable")); \
		stack->set(*expression[j].string, result); \
	} else if (oper == (#TOKEN "=") && oper.length() > 1) \
		throw UNKNOWN_OPERATOR; \
	/* Now update the expression vector */  \
	expression[j] = result; \
	expression.erase(expression.begin() + j + 1, expression.begin() + j + 3); }

	// Pass 1: /, *
	for (vector<Object>::size_type j = 0; j < expression.size(); j++) {
		GET_OPERATOR_OR_CONTINUE;
		if (oper[0] == '/')
			IMPLEMENT_OPERATOR(/* operator name */ div,
							   /* token */ 	       /,
							   /* "TOKEN=" */      true)
		else if (oper[0] == '*')
			IMPLEMENT_OPERATOR(/* operator name */ mult,
							   /* token */ 	       *,
							   /* "TOKEN=" */      true)
	}
	// Pass 2: +, -. TODO: ++, --
	for (vector<Object>::size_type j = 0; j < expression.size(); j++) {
		GET_OPERATOR_OR_CONTINUE;
		if (oper[0] == '-')
			IMPLEMENT_OPERATOR(/* operator name */ subt,
							   /* token */ 	       -,
							   /* "TOKEN=" */      true)
		else if (oper[0] == '+')
			IMPLEMENT_OPERATOR(/* operator name */ add,
							   /* token */ 	       +,
							   /* "TOKEN=" */      true)
	}
	// Pass 3: ==, !=
	for (vector<Object>::size_type j = 0; j < expression.size(); j++) {
		GET_OPERATOR_OR_CONTINUE;
		if (oper == "==")
			IMPLEMENT_OPERATOR(/* operator name */ eq,
							   /* token */ 	       ==,
							   /* "TOKEN=" */      false)
		else if (oper == "!=")
			IMPLEMENT_OPERATOR(/* operator name */ neq,
							   /* token */ 	       !=,
							   /* "TOKEN=" */      false)
	}
	// Pass 4: =
	for (vector<Object>::size_type j = 0; j < expression.size(); j++) {
		GET_OPERATOR_OR_CONTINUE;
		if (oper == "=") {
			j--;
			if (expression[j].type() != Type::Variable)
				throw Exception(Exception::TypeError,
					string("the left-hand side of '=' must be a variable"));
			Object result = Language_POSSIBLY_DEREFERENCE(expression[j + 2]);
			stack->set(*expression[j].string, result);
			/* Now update the expression vector */
			expression[j] = result;
			expression.erase(expression.begin() + j + 1, expression.begin() + j + 3);
		}
	}
	// Pass 5: &&, ||
	for (vector<Object>::size_type j = 0; j < expression.size(); j++) {
		GET_OPERATOR_OR_CONTINUE;
		if (oper == "&&")
			IMPLEMENT_OPERATOR(/* operator name */ and,
							   /* token */ 	       &&,
							   /* "TOKEN=" */      false)
		else if (oper == "||")
			IMPLEMENT_OPERATOR(/* operator name */ or,
							   /* token */ 	       ||,
							   /* "TOKEN=" */      false)
	}
	// Pass 6: throw if there are remaining operators
	for (vector<Object>::size_type j = 0; j < expression.size(); j++) {
		GET_OPERATOR_OR_CONTINUE;
		throw UNKNOWN_OPERATOR;
	}
	if (expression.size() != 1)
		throw Exception(Exception::TypeError,
			string("evaluated expression does not have 1 return value"));
	return expression[0];
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
