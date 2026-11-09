#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "Token.h"

class Lexer {
private:
	std::string src;
	size_t pos = 0;
	std::unordered_map<std::string,TokenType> keywords;

	Token GetNextToken();
public:
	Lexer(std::string str);
	void clearComments();
	std::vector<Token> tokenize();
};