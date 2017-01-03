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

Object ExprNode::toObject(Stack* stack)
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
				if (string[i + 1] != '{' && string[i + 2] != '{') {
					str += c;
					continue;
				}
				// Yes, this is how Phoenix actually has eval().
				// Please do not try this at home.
				// See the "string-deref-abuse" test if your curiosity is insatiable.
				try {
					str += EvalVariableName(stack, string, line, i).toObject(stack)->asStringRaw();
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
		std::string("illegal conversion from ExprNode to Object, please file a bug!"));
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
	case '%': case '<': case '>'
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

ExprNode EvalVariableName(Stack* stack, const string& code, uint32_t& line, string::size_type& i)
{
	assert(code[i] == '$');
	i++;

	ExprNode realRet(ExprNode::Variable);
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
			i++;
			Object result = ParseAndEvalExpression(PARSER_PARAMS);
			realRet.variable.push_back(result->asStringRaw());
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
			i++; // Cancels out the i-- below.
		// fall through
		default:
			i--;
			if (ret.size() > 0)
				realRet.variable.push_back(ret);
			return realRet;
		}
		i++;
	}
	i--; // so we're 1 before the last in the str
	return realRet;
}

ExprNode ParseString(Stack*, const string& code, uint32_t& line, string::size_type& i)
{
	string ret = "";
	const char endChar = code[i];
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
		return ExprNode(ExprNode::RawString, ret);
	return ExprNode(ExprNode::Literal, StringObject(ret));
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
		func = *o->function;
	} else {
		auto funcIter = stack->GlobalFunctions.find(funcRef[0]);
		if (funcIter == stack->GlobalFunctions.end()) {
			throw Exception(Exception::SyntaxError, string("attempted to call function '")
				.append(funcRef[0]).append("' which does not exist"));
		}
		func = funcIter->second;
	}

	// Parse & build argument map
	assert(code[i] == '(');
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
					code[j] == '$' || code[j] == '{' || code[j] == ')')
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
						paramName = ParseString(PARSER_PARAMS).toObject(stack)->string;
						pastEndOfParamName = true;
					} else
						throw UNEXPECTED_TOKEN;
				break;

				case ')':
					i--;
					// fallthrough
				case ',':
					treatAsTrueBoolean = true;
					// fallthrough
				case ':':
					pastColon = true;
				break;

				default:
					throw UNEXPECTED_TOKEN;
				}
				i++;
			}
		}
		if (i >= code.length())
			throw UNEXPECTED_EOF;

		if (paramName == "false" || paramName == "true" || paramName == "undefined" ||
			paramName == "return" || paramName == "if") {
			throw Exception(Exception::SyntaxError, "'" + paramName + "' is an illegal parameter name");
		}

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

	Object context = nullptr;
	if (funcRef.size() > 1) {
		vector<string> parentRef = funcRef;
		parentRef.pop_back();
		context = stack->get_ptr(parentRef);
	}
	return func.call(stack, context, arguments);
}

Object ParseList(Stack* stack, const string& code, uint32_t& line, string::size_type& i)
{
	assert(code[i] == '[');
	i++;

	ObjectList* ret = new ObjectList;
	bool atEndOfList = false;
	while (!atEndOfList && i < code.length()) {
		ret->push_back(ParseAndEvalExpression(PARSER_PARAMS));
		switch (code[i]) {
		case ']':
			atEndOfList = true;
			i--;
			break;

		case ',':
		default:
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
	ReturnValue(Object val) : value(val) {}
	virtual ~ReturnValue() noexcept {}
	virtual const char* what() const noexcept { return "ReturnValue"; }

	Object value;
};
class BreakStatement : public std::exception
{
public:
	BreakStatement() {}
	virtual ~BreakStatement() noexcept {}
	virtual const char* what() const noexcept { return "Break"; }
};
class ContinueStatement : public std::exception
{
public:
	ContinueStatement() {}
	virtual ~ContinueStatement() noexcept {}
	virtual const char* what() const noexcept { return "Continue"; }
};

string::size_type LocateEndOfScope(Stack*, const string& code, uint32_t&, const string::size_type& i)
{
	string scope;
	if (code[i] == '{' || code[i] == '(')
		scope.append(&code[i], 1);
	else
		scope = "_"; // Must be a one-liner
	string::size_type ret = i;
	ret++;
	while (scope.size()) {
		ret++;
		char c = code[ret];
		switch (c) {
		case '(':
		case '[':
		case '{':
			scope += c;
			break;

		case ')':
			if (scope[scope.size() - 1] != '(')
				throw UNEXPECTED_TOKEN;
			scope.resize(scope.size() - 1);
			break;
		case ']':
			if (scope[scope.size() - 1] != '[')
				throw UNEXPECTED_TOKEN;
			scope.resize(scope.size() - 1);
			break;
		case '}':
			if (scope[scope.size() - 1] != '{')
				throw UNEXPECTED_TOKEN;
			scope.resize(scope.size() - 1);
			break;

		case ';':
			if (scope == "_")
				scope.resize(scope.size() - 1);
			break;

		default:
			break;
		}
	}
	return ret;
}
inline void JumpToPosition(const string::size_type& pos, Stack*, const string& code, uint32_t& line,
	string::size_type& i)
{
	while (i < pos) {
		if (code[i] == '\n')
			line++;
		i++;
	}
}

void ConditionalBranchHandler(vector<ExprNode> expression, string thing, Stack* stack,
	const string& code, uint32_t& line, string::size_type& i)
{
	if (expression.size() != 0)
		throw Exception(Exception::SyntaxError, string("incorrectly placed '").append(thing).append("'"));
	i++;
	IgnoreWhitespace(PARSER_PARAMS);
	if (code[i] != '(')
		throw UNEXPECTED_TOKEN_EXPECTED("(");
	string::size_type endOfBlock = string::npos;

	auto CBH_Inner = [&](bool noExpression = false) -> bool {
		bool exec = true;
		if (!noExpression) {
			exec = ParseAndEvalExpression(PARSER_PARAMS)->coerceToBoolean();
			i++;
		}
		IgnoreWhitespace(PARSER_PARAMS);
		if (endOfBlock == string::npos)
			endOfBlock = LocateEndOfScope(PARSER_PARAMS);
		if (exec) {
			bool oneliner = code[i] != '{';
			if (!oneliner)
				i++;
			stack->push();
			while (i < endOfBlock) {
				ParseAndEvalExpression(PARSER_PARAMS);
				i++;
				IgnoreWhitespace(PARSER_PARAMS);
			}
			if (oneliner)
				i--;
			stack->pop();
		} else {
			// Skip the block.
			JumpToPosition(endOfBlock, PARSER_PARAMS);
		}
		return exec;
	};

	if (thing == "while") {
		uint32_t old_line = line;
		string::size_type old_i = i;
		try {
			bool result = true;
			while (result) {
				line = old_line;
				i = old_i;
				try {
					result = CBH_Inner();
				} catch (ContinueStatement) {
					result = true;
				}
			}
		} catch (BreakStatement) {
			// Skip the rest of the block.
			JumpToPosition(endOfBlock, PARSER_PARAMS);
		}
	} else if (thing == "if") {
		bool done = false, didExec = false;
		while (!done) {
			endOfBlock = string::npos;
			if (!didExec)
				didExec = CBH_Inner();

			const uint32_t oldLine = line;
			const string::size_type oldI = i;
			if (code[i] == ';' || code[i] == '}')
				i++;
			IgnoreWhitespace(PARSER_PARAMS);
			if (code.substr(i, 4) == "else") {
				i += 4;
				IgnoreWhitespace(PARSER_PARAMS);
				if (code.substr(i, 2) == "if" && !didExec) {
					i += 2;
					continue;
				} else if (!didExec) {
					didExec = CBH_Inner(true);
					continue;
				} else {
					if (code.substr(i, 2) == "if") {
						i += 2;
						IgnoreWhitespace(PARSER_PARAMS);
						JumpToPosition(LocateEndOfScope(PARSER_PARAMS), PARSER_PARAMS);
						i++;
					}
					IgnoreWhitespace(PARSER_PARAMS);
					JumpToPosition(LocateEndOfScope(PARSER_PARAMS), PARSER_PARAMS);
				}
			} else {
				line = oldLine;
				i = oldI;
				done = true;
			}
		}
	}
}

Object ParseAndEvalExpression(Stack* stack, const string& code, uint32_t& line, string::size_type& i)
{
	// Parse
	vector<ExprNode> expression;
	bool atEOE = false, isReturn = false;

	IgnoreWhitespace(PARSER_PARAMS);
	string::size_type start = i;

	while (!atEOE && i < code.length()) {
		switch (code[i]) {
		case '(':
			if (i == start)
				break;
			if (expression.size() > 0 && expression[expression.size() - 1].type == ExprNode::Variable) {
				expression[expression.size() - 1] =
					ExprNode(ExprNode::Literal, ParseCallAndEval(PARSER_PARAMS,
						expression[expression.size() - 1].variable, true));
			} else {
				expression.push_back(ExprNode(ExprNode::Literal, ParseAndEvalExpression(PARSER_PARAMS)));
			}
		break;
		case '[':
			// Assume list
			expression.push_back(ExprNode(ExprNode::Literal, ParseList(PARSER_PARAMS)));
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
		case '\'': // String literal
			expression.push_back(ParseString(PARSER_PARAMS));
		break;
		case NUMERIC_CASES: // Number
			expression.push_back(ExprNode(ExprNode::Literal, ParseNumber(PARSER_PARAMS)));
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
				expression.push_back(ExprNode(ExprNode::Literal, BooleanObject(true)));
			else if (thing == "false")
				expression.push_back(ExprNode(ExprNode::Literal, BooleanObject(false)));
			else if (thing == "undefined")
				expression.push_back(ExprNode(ExprNode::Literal, UndefinedObject()));
			else if (thing == "return") {
				if (expression.size() != 0)
					throw Exception(Exception::SyntaxError, string("incorrectly placed 'return'"));
				isReturn = true;
			} else if (thing == "break") {
				if (expression.size() != 0)
					throw Exception(Exception::SyntaxError, string("incorrectly placed 'break'"));
				throw BreakStatement();
			} else if (thing == "continue") {
				if (expression.size() != 0)
					throw Exception(Exception::SyntaxError, string("incorrectly placed 'continue'"));
				throw ContinueStatement();
			} else if (thing == "if" || thing == "while") {
				ConditionalBranchHandler(expression, thing, PARSER_PARAMS);
				atEOE = true;
			} else if (thing == "function") {
				i++;
				IgnoreWhitespace(PARSER_PARAMS);
				if (code[i] != '(') {
					throw Exception(Exception::SyntaxError,
						"function must formally begin with '()'");
				} else
					i++;
				IgnoreWhitespace(PARSER_PARAMS);
				if (code[i] == ')')
					i++;
				else {
					throw Exception(Exception::SyntaxError,
						"function must formally begin with '()'");
				}
				IgnoreWhitespace(PARSER_PARAMS);

				string::size_type funcEnd = LocateEndOfScope(PARSER_PARAMS);
				i++;
				string func = code.substr(i, funcEnd - i);
				expression.push_back(ExprNode(ExprNode::Literal,
					FunctionObject(new Function(func, stack->currentInputFile(), line))));
				i = funcEnd;
			} else if (thing == "subdirectory") {
				i++;
				IgnoreWhitespace(PARSER_PARAMS);
				if (code[i] == '"' || code[i] == '\'') {
					string path = ParseString(PARSER_PARAMS).toObject(stack)->asStringRaw();
					expression.push_back(ExprNode(ExprNode::Literal,
						Run(stack, FSUtil::combinePaths({stack->currentDir(), path}))));
				}
			} else { // This better be a function call
				i++;
				IgnoreWhitespace(PARSER_PARAMS);
				if (code[i] != '(') {
					throw Exception(Exception::SyntaxError,
						string("unrecognized keyword '").append(thing).append("'"));
				}
				expression.push_back(ExprNode(ExprNode::Literal, ParseCallAndEval(PARSER_PARAMS, {thing})));
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
			expression.push_back(ExprNode(ExprNode::Operator, oper));
		} break;

		default:
			throw UNEXPECTED_TOKEN;
		}
		if (!atEOE) {
			i++;
			IgnoreWhitespace(PARSER_PARAMS);
		}
	}

#define RETURN(x) if (isReturn) { throw ReturnValue(x); } else { return x; }
	if (expression.size() == 0) {
		RETURN(UndefinedObject()); // undefined
	}
	if (expression.size() == 1) {
		RETURN(expression[0].toObject(stack));
	}

	// Evaluate
#define GET_OPERATOR_OR_CONTINUE \
	const ExprNode& node = expression[j]; \
	if (node.type != ExprNode::Operator) \
		continue; \
	const string& oper = node.string
#define UNKNOWN_OPERATOR \
	Exception(Exception::SyntaxError, \
		string("unknown operator '").append(oper).append("'"));
#define IMPLEMENT_OPERATOR(OP_NAME, TOKEN, DOING_TOKEQ) { \
	j--; \
	Object result = CObject::op_##OP_NAME(expression[j].toObject(stack), \
		expression[j + 2].toObject(stack)); \
	if (DOING_TOKEQ && oper == (#TOKEN "=")) { \
		if (expression[j].type != ExprNode::Variable) \
			throw Exception(Exception::TypeError, string("the left-hand side of '" #TOKEN \
				"=' must be a variable")); \
		stack->set(expression[j].variable, result); \
	} \
	/* Now update the expression vector */  \
	expression[j] = ExprNode(ExprNode::Literal, result); \
	expression.erase(expression.begin() + j + 1, expression.begin() + j + 3); }

	// Pass 1: !, !!, ++, -- (no macro, they only affect one side)
	for (vector<ExprNode>::size_type j = 0; j < expression.size(); j++) {
		GET_OPERATOR_OR_CONTINUE;
		if (oper == "!") {
			Object result = BooleanObject(!(expression[j + 1].toObject(stack)->coerceToBoolean()));
			expression[j] = ExprNode(ExprNode::Literal, result);
			expression.erase(expression.begin() + j + 1, expression.begin() + j + 2);
		} else if (oper == "!!") {
			Object result = BooleanObject(expression[j + 1].toObject(stack)->coerceToBoolean());
			expression[j] = ExprNode(ExprNode::Literal, result);
			expression.erase(expression.begin() + j + 1, expression.begin() + j + 2);
		} else if (oper == "++") {
			j--;
			Object result = expression[j].toObject(stack);
			result->integer++;
			expression[j] = ExprNode(ExprNode::Literal, result);
			expression.erase(expression.begin() + j + 1, expression.begin() + j + 2);
		} else if (oper == "--") {
			j--;
			Object result = expression[j].toObject(stack);
			result->integer--;
			expression[j] = ExprNode(ExprNode::Literal, result);
			expression.erase(expression.begin() + j + 1, expression.begin() + j + 2);
		}
	}
	// Pass 2: /, *, %
	for (vector<ExprNode>::size_type j = 0; j < expression.size(); j++) {
		GET_OPERATOR_OR_CONTINUE;
		if (oper == "/")
			IMPLEMENT_OPERATOR(/* operator name */ div,
							   /* token */ 	       /,
							   /* "TOKEN="? */     false)
		else if (oper == "*")
			IMPLEMENT_OPERATOR(/* operator name */ mult,
							   /* token */ 	       *,
							   /* "TOKEN="? */     false)
		else if (oper == "%")
			IMPLEMENT_OPERATOR(/* operator name */ modulo,
							   /* token */ 	       %,
							   /* "TOKEN="? */     false)
	}
	// Pass 3: +, -
	for (vector<ExprNode>::size_type j = 0; j < expression.size(); j++) {
		GET_OPERATOR_OR_CONTINUE;
		if (oper == "-")
			IMPLEMENT_OPERATOR(/* operator name */ subt,
							   /* token */ 	       -,
							   /* "TOKEN="? */     false)
		else if (oper == "+")
			IMPLEMENT_OPERATOR(/* operator name */ add,
							   /* token */ 	       +,
							   /* "TOKEN="? */     false)
	}
	// Pass 4: ==, !=, <, >, <=, >=
	for (vector<ExprNode>::size_type j = 0; j < expression.size(); j++) {
		GET_OPERATOR_OR_CONTINUE;
		if (oper == "==")
			IMPLEMENT_OPERATOR(/* operator name */ eq,
							   /* token */ 	       ==,
							   /* "TOKEN="? */     false)
		else if (oper == "!=")
			IMPLEMENT_OPERATOR(/* operator name */ neq,
							   /* token */ 	       !=,
							   /* "TOKEN="? */     false)
		else if (oper == "<")
			IMPLEMENT_OPERATOR(/* operator name */ lt,
							   /* token */ 	       <,
							   /* "TOKEN="? */     false)
		else if (oper == ">")
			IMPLEMENT_OPERATOR(/* operator name */ gt,
							   /* token */ 	       >,
							   /* "TOKEN="? */     false)
		else if (oper == "<=")
			IMPLEMENT_OPERATOR(/* operator name */ lteq,
							   /* token */ 	       <=,
							   /* "TOKEN="? */     false)
		else if (oper == ">=")
			IMPLEMENT_OPERATOR(/* operator name */ gteq,
							   /* token */ 	       >=,
							   /* "TOKEN="? */     false)
	}
	// Pass 5: /=, *=
	for (vector<ExprNode>::size_type j = 0; j < expression.size(); j++) {
		GET_OPERATOR_OR_CONTINUE;
		if (oper == "/=")
			IMPLEMENT_OPERATOR(/* operator name */ div,
							   /* token */ 	       /,
							   /* "TOKEN="? */     true)
		else if (oper == "*=")
			IMPLEMENT_OPERATOR(/* operator name */ mult,
							   /* token */ 	       *,
							   /* "TOKEN="? */     true)
	}
	// Pass 6: +=, -=
	for (vector<ExprNode>::size_type j = 0; j < expression.size(); j++) {
		GET_OPERATOR_OR_CONTINUE;
		if (oper == "-=")
			IMPLEMENT_OPERATOR(/* operator name */ subt,
							   /* token */ 	       -,
							   /* "TOKEN="? */     true)
		else if (oper == "+=")
			IMPLEMENT_OPERATOR(/* operator name */ add,
							   /* token */ 	       +,
							   /* "TOKEN="? */     true)
	}
	// Pass 7: =
	for (vector<ExprNode>::size_type j = 0; j < expression.size(); j++) {
		GET_OPERATOR_OR_CONTINUE;
		if (oper == "=") {
			j--;
			if (expression[j].type != ExprNode::Variable)
				throw Exception(Exception::TypeError,
					string("the left-hand side of '=' must be a variable"));
			if (expression[j + 2].type == ExprNode::Variable &&
				expression[j + 2].variable[0][0] == '$')
				throw Exception(Exception::TypeError,
					string("superglobals cannot be copied"));
			Object result = expression[j + 2].toObject(stack);
			stack->set(expression[j].variable, result);
			/* Now update the expression vector */
			expression[j] = ExprNode(ExprNode::Literal, result);
			expression.erase(expression.begin() + j + 1, expression.begin() + j + 3);
		}
	}
	// Pass 8: &&, ||
	for (vector<ExprNode>::size_type j = 0; j < expression.size(); j++) {
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
	// Pass 9: throw if there are remaining operators
	for (vector<ExprNode>::size_type j = 0; j < expression.size(); j++) {
		GET_OPERATOR_OR_CONTINUE;
		throw UNKNOWN_OPERATOR;
	}
	if (expression.size() != 1)
		throw Exception(Exception::TypeError,
			string("evaluated expression does not have 1 return value"));
	RETURN(expression[0].toObject(stack));
#undef RETURN
}

Object EvalString(Stack* stack, const string& code, string fromPath, const uint32_t fromLine, bool popDirs)
{
	uint32_t line = fromLine;
	string::size_type i = 0;
	try {
		IgnoreWhitespace(PARSER_PARAMS);
		while (i < code.length()) {
			ParseAndEvalExpression(PARSER_PARAMS);
			i++;
			IgnoreWhitespace(PARSER_PARAMS);
		}
	} catch (ReturnValue& e) {
		if (popDirs)
			stack->popDir();
		return e.value;
	} catch (BreakStatement) {
		if (popDirs)
			stack->popDir();
	   throw Exception(Exception::SyntaxError, "unexpected 'break'", fromPath, line);
	} catch (ContinueStatement) {
		if (popDirs)
			stack->popDir();
	   throw Exception(Exception::SyntaxError, "unexpected 'continue'", fromPath, line);
	} catch (Exception& e) {
		if (popDirs)
			stack->popDir();
		if (e.fLine == 0) {
			e.fFile = fromPath;
			e.fLine = line;
		}
		throw e;
	}
	if (popDirs)
		stack->popDir();
	return UndefinedObject(); // undefined
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
	stack->appendInputFile(filename);

	return EvalString(stack, code, filename, 1, true);
}

}
