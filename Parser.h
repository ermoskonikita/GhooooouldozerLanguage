#pragma once
#include "Token.h"
#include <vector>
#include <string>

class Parser{
public:
	Parser(std::vector<Token> t_vec);
	void parse();
	const std::vector<std::string>& getBytecode();
private:
	std::vector<Token> token_vec;
	size_t pos = 0;
	std::vector<std::string> main_bytecode;
	std::vector<std::string> func_bytecode;
	std::vector<std::string> final_bytecode;
	std::vector<std::string>* target_bytecode;

	size_t if_counter = 0;
	size_t while_counter = 0;

	Token current();
	void advance();
	void expect(TokenType type, const std::string& error_msg);

	void parseFactor();
	void parseTerm();
	void parseExpression();
	void parseComparison();
	void parseBAnd();
	void parseBXor();
	void parseBOr();
	void parseAnd();
	void parseOr();
	void parseStatement();
	void parseBlock();
	void parseFunctionDecl();
};