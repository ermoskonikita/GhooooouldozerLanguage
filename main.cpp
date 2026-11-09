#include "Assembler.h"
#include "Virtual_Machine.h"
#include "Lexer.h"
#include "Parser.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <Windows.h>

int main() {
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);

	std::ifstream file("code.txt");

	try {
		if (!file.is_open()) throw std::runtime_error("Не удалось открыть файл!");
		std::stringstream buf;
		buf << file.rdbuf();
		std::string src = buf.str();

		Lexer lexer(src);
		Parser parser(lexer.tokenize());
		parser.parse();

		Assembler assembler(parser.getBytecode());

		Virtual_Machine vm;
		vm.SetBytecode(assembler.Compile());
		vm.run();

		std::cout << "VM finished execution.\n";
	}
	catch (std::exception& ex) {
		std::cerr << ex.what();
		return -1;
	}

	return 0;
}