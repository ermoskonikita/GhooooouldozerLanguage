#include "Parser.h"
#include <stdexcept>
#include <iostream>

Parser::Parser(std::vector<Token> t_vec) : token_vec(std::move(t_vec)),target_bytecode(&main_bytecode) {}

Token Parser::current() { return token_vec[pos]; }
void Parser::advance() { pos++; }

void Parser::expect(TokenType type, const std::string& error_msg) {
	if (current().type == type) advance();
	else throw std::runtime_error(error_msg);
}

void Parser::parseFactor() {
	if (current().type == TokenType::Not) {
		advance();
		parseFactor();
		target_bytecode->push_back("NOT");
	}
	else if (current().type == TokenType::BNot) {
		advance();
		parseFactor();
		target_bytecode->push_back("BNOT");
	}
	else if (current().type == TokenType::Number) {
		target_bytecode->push_back("PUSH " + current().text);
		advance();
	}
	else if (current().type == TokenType::Identifier) {
		Token id_name = current();
		advance();

		if (current().type == TokenType::LParen) {
			advance();
			expect(TokenType::RParen, "Ожидалась )!");
			target_bytecode->push_back("CALL " + id_name.text);
		}
		else if (current().type == TokenType::LBracket) {
			advance();
			parseOr();
			expect(TokenType::RBracket, "Ожидалась ]!");
			target_bytecode->push_back("LOAD_INDEX " + id_name.text);
		}
		else target_bytecode->push_back("LOAD " + id_name.text);
	}
	else if (current().type == TokenType::LParen) {
		advance();
		parseOr();
		expect(TokenType::RParen, "Ожидалась закрывающаяся скобка!");
	}
	else throw std::runtime_error("Ожидалось число или выражение в скобках!");
}

void Parser::parseTerm() {
	parseFactor();
	while (current().type == TokenType::Star || current().type == TokenType::Slash) {
		Token t = current();
		advance();
		parseFactor();
		if (t.type == TokenType::Star) {
			target_bytecode->push_back("MUL");
		}
		else {
			target_bytecode->push_back("DIV");
		}
	}
}

void Parser::parseExpression() {
	parseTerm();
	while (current().type == TokenType::Plus || current().type == TokenType::Minus) {
		Token t = current();
		advance();
		parseTerm();
		if (t.type == TokenType::Plus) {
			target_bytecode->push_back("ADD");
		}
		else {
			target_bytecode->push_back("SUB");
		}
	}
}

void Parser::parseComparison() {
	parseExpression();
	while (current().type == TokenType::Less || current().type == TokenType::Greater
		  || current().type == TokenType::Equal || current().type == TokenType::LessEqual
		  || current().type == TokenType::GreaterEqual || current().type == TokenType::NotEqual) {
		Token t = current();
		advance();
		parseExpression();
		if (t.type == TokenType::Less) {
			target_bytecode->push_back("LT");
		}
		else if (t.type == TokenType::LessEqual) {
			target_bytecode->push_back("LE");
		}
		else if (t.type == TokenType::Greater) {
			target_bytecode->push_back("GT");
		}
		else if (t.type == TokenType::GreaterEqual) {
			target_bytecode->push_back("GE");
		}
		else if(t.type == TokenType::Equal){
			target_bytecode->push_back("EQ");
		}
		else target_bytecode->push_back("NE");
	}
}

void Parser::parseBAnd() {
	parseComparison();
	while (current().type == TokenType::BAnd) {
		advance();
		parseComparison();
		target_bytecode->push_back("BAND");
	}
}

void Parser::parseBXor() {
	parseBAnd();
	while (current().type == TokenType::BXor) {
		advance();
		parseBAnd();
		target_bytecode->push_back("BXOR");
	}
}

void Parser::parseBOr() {
	parseBXor();
	while (current().type == TokenType::BOr) {
		advance();
		parseBXor();
		target_bytecode->push_back("BOR");
	}
}

void Parser::parseAnd() {
	parseBOr();
	while (current().type == TokenType::And) {
		advance();
		parseBOr();
		target_bytecode->push_back("AND");
	}
}

void Parser::parseOr() {
	parseAnd();
	while (current().type == TokenType::Or) {
		advance();
		parseAnd();
		target_bytecode->push_back("OR");
	}
}

void Parser::parseStatement() {
	if (current().type == TokenType::Print) {
		advance();

		Token print_expr = current();
		if (print_expr.type == TokenType::String) {
			for (unsigned char sym : print_expr.text) {
				target_bytecode->push_back("PUSH " + std::to_string(sym));
				target_bytecode->push_back("PRINT_CHAR");
			}
			advance();
			expect(TokenType::Semicolon, "Ожидалась точка с запятой!");
		}
		else {
			parseOr();
			expect(TokenType::Semicolon, "Ожидалась точка с запятой!");
			target_bytecode->push_back("PRINT");
		}
	}
	else if (current().type == TokenType::Return) {
		advance();
		parseOr();
		expect(TokenType::Semicolon, "Ожидалась точка с запятой!");
		target_bytecode->push_back("RET");
	}
	else if (current().type == TokenType::If) {
		advance();
		expect(TokenType::LParen, "Ожидалась (!");
		parseOr();
		expect(TokenType::RParen, "Ожидалась )!");

		target_bytecode->push_back("JZ ");
		size_t if_else_index = target_bytecode->size() - 1;

		expect(TokenType::LBrace, "Ожидалась {!");
		parseBlock();
		expect(TokenType::RBrace, "Ожидалась }!");
		target_bytecode->push_back(std::string("JMP ") + "end_if_" + std::to_string(if_counter));

		if (current().type == TokenType::Else) {
			(*target_bytecode)[if_else_index] += ("else_" + std::to_string(if_counter));

			target_bytecode->push_back("else_" + std::to_string(if_counter) + ':');
			advance();
			expect(TokenType::LBrace, "Ожидалась {!");
			parseBlock();
			expect(TokenType::RBrace, "Ожидалась }!");
		}
		else (*target_bytecode)[if_else_index] += ("end_if_" + std::to_string(if_counter));

		target_bytecode->push_back("end_if_" + std::to_string(if_counter) + ':');

		if_counter++;
	}
	else if (current().type == TokenType::While) {
		advance();

		target_bytecode->push_back("while_" + std::to_string(while_counter) + ':');
		expect(TokenType::LParen, "Ожидалась (!");
		parseOr();
		expect(TokenType::RParen, "Ожидалась )!");
		target_bytecode->push_back(std::string("JZ ") + "end_while_" + std::to_string(while_counter));

		expect(TokenType::LBrace, "Ожидалась {!");
		parseBlock();
		expect(TokenType::RBrace, "Ожидалась }!");
		target_bytecode->push_back(std::string("JMP ") + "while_" + std::to_string(while_counter));

		target_bytecode->push_back("end_while_" + std::to_string(while_counter) + ':');
		while_counter++;
	}
	else if (current().type == TokenType::Identifier && pos + 1 < token_vec.size()
		&& token_vec[pos + 1].type == TokenType::Assign) {
		Token id_name = current();
		advance();
		advance();

		parseOr();
		expect(TokenType::Semicolon, "Ожидалась точка с запятой!");

		target_bytecode->push_back("STORE " + id_name.text);
	}
	else if (current().type == TokenType::Identifier && pos + 4 < token_vec.size()
		&& token_vec[pos + 1].type == TokenType::LBracket
		&& (token_vec[pos + 2].type == TokenType::Number
		|| token_vec[pos + 2].type == TokenType::Identifier)
		&& token_vec[pos + 3].type == TokenType::RBracket
		&& token_vec[pos + 4].type == TokenType::Semicolon) {

		Token id_name = current();
		advance();
		advance();
		std::string size_of_arr = current().text;

		advance();
		advance();
		expect(TokenType::Semicolon, "Ожидалась точка с запятой!");

		target_bytecode->push_back("ALLOC " + id_name.text + ' ' + size_of_arr);
	}
	else if (current().type == TokenType::Identifier && pos + 1 < token_vec.size()
		&& token_vec[pos + 1].type == TokenType::LBracket) {

		size_t rollback_pos = pos;
		std::vector<std::string>* copy_ptr = target_bytecode;
		std::vector<std::string> check_vector;
		target_bytecode = &check_vector;

		Token id_name = current();
		advance();
		advance();

		parseOr();

		expect(TokenType::RBracket, "Ожидалась ]!");

		target_bytecode = copy_ptr;
		if (current().type == TokenType::Assign) {
			target_bytecode->insert(target_bytecode->end(),check_vector.begin(),check_vector.end());

			advance();
			parseOr();
			expect(TokenType::Semicolon, "Ожидалась точка с запятой!");

			target_bytecode->push_back("SWAP");
			target_bytecode->push_back("STORE_INDEX " + id_name.text);
		}
		else {
			pos = rollback_pos;
			parseOr();
			expect(TokenType::Semicolon, "Ожидалась точка с запятой!");
			target_bytecode->push_back("POP");
		}
	}
	else {
		parseOr();
		expect(TokenType::Semicolon, "Ожидалась точка с запятой!");
		target_bytecode->push_back("POP");
	}
}

void Parser::parseBlock() {
	while (current().type != TokenType::RBrace && current().type != TokenType::End)
		parseStatement();
}

void Parser::parseFunctionDecl() {
	advance();
	Token func_name = current();
	expect(TokenType::Identifier, "Ожидалось имя функции!");

	target_bytecode = &func_bytecode;

	target_bytecode->push_back(func_name.text + ':');

	expect(TokenType::LParen, "Ожидалась (!");
	expect(TokenType::RParen, "Ожидалась )!");
	expect(TokenType::LBrace, "Ожидалась {!");
	parseBlock();
	expect(TokenType::RBrace, "Ожидалась }!");

	target_bytecode->push_back("PUSH 0");
	target_bytecode->push_back("RET");
	target_bytecode = &main_bytecode;
}

void Parser::parse() {
	while (current().type != TokenType::End) {
		if (current().type == TokenType::Func) parseFunctionDecl();
		else parseStatement();
	}

	main_bytecode.push_back("HALT");
	final_bytecode = std::move(main_bytecode);
	final_bytecode.insert(final_bytecode.end(), func_bytecode.begin(), func_bytecode.end());

	//debug
	std::cout << "Parser:\n";
	for (const auto& str : final_bytecode) std::cout << str << '\n'; 
}

const std::vector<std::string>& Parser::getBytecode() {
	return final_bytecode;
}