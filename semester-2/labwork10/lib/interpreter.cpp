#include <sstream>
#include <iostream>
#include "interpreter.h"

std::vector<std::string> call_stack;
std::unordered_map<std::string, Value> variables;
std::vector<std::unordered_map<std::string, Value>*> scopes{ &variables };

Interpreter::Interpreter(Parser parser) : parser(parser) {}

Value Interpreter::interpret() {
    std::unique_ptr<AST> tree = parser.parse();
    return tree->visit();
}

bool interpret(std::string& input) {
	std::istringstream in(input);
	std::ostringstream out;
	bool ok = interpret(in, out);
	std::cout << out.str();
	return ok;
}

bool interpret(std::istream& input, std::ostream& output) {
	std::string src((std::istreambuf_iterator<char>(input)),
		std::istreambuf_iterator<char>());
	Lexer lexer(src);
	Parser parser(lexer);
	Interpreter interp(parser);
    std::ostream* old = itmo_out;
    itmo_out = &output;
	Value result = interp.interpret();
    itmo_out = old;
	return true;
}