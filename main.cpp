#include "Assembler.h"
#include "Virtual_Machine.h"
#include "Lexer.h"
#include "Parser.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

int main() {

	std::ifstream file("code.txt");

	try {
		if (!file.is_open()) throw std::runtime_error("Failed to open file!");
		std::stringstream buf;
		buf << file.rdbuf();
		std::string src = buf.str();

		Lexer lexer(src);
		Parser parser(lexer.tokenize());
		parser.parse();

		Assembler assembler(parser.getBytecode());

		Virtual_Machine vm(assembler.Compile());
		vm.run();
	}
	catch (const std::exception& ex) {
		std::cerr << ex.what() << '\n';
		return -1;
	}

	return 0;
}