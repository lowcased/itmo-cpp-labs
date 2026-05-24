#include <iostream>
#include <fstream>
#include "interpreter.h"

int main(int argc, char** argv) {
	if (argc > 1) {
		std::ifstream in(argv[1]);
		if (!in) {
			std::cerr << "error: cannot open file '" << argv[1] << "'\n";
			return 1;
		}
		return interpret(in, std::cout) ? 0 : 1;
	}
	return interpret(std::cin, std::cout) ? 0 : 1;
}
