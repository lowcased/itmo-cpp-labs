#pragma once
#include "parser.h"
#include <istream>
#include <ostream>

class Interpreter {
public:
	explicit Interpreter(Parser parser);
	Value interpret();
private:
	Parser parser;
};

bool interpret(std::string& input);
bool interpret(std::istream& in, std::ostream&);