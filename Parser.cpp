#include "Parser.h"
#include <stdexcept>
#include <iostream>

Parser::Parser(std::vector<Token> t_vec) : token_vec(std::move(t_vec)),target_bytecode(&main_bytecode) {}

const Token& Parser::current() { return token_vec[pos]; }
void Parser::advance() { pos++; }

void Parser::expect(TokenType type, const std::string& error_msg) {
	if (current().type == type) advance();
	else throw std::runtime_error(error_msg);
}

size_t Parser::getVar(const std::string& str, SymbolType t) {
	if (auto it = symbol_table.find(str); it == symbol_table.end()) {
		throw std::runtime_error("Undeclared variable!");
	}
	else if(it->second.type == t) return it->second.address;
	throw std::runtime_error("Invalid variable type!");
}

size_t Parser::declare_or_getVar(const std::string& str, SymbolType t) {
	auto it = symbol_table.find(str);
	if (it == symbol_table.end()) {
		size_t addr = next_free_address++;
		symbol_table.emplace(str, Symbol{t,addr});
		return addr;
	}
	else if (t == SymbolType::Scalar && it->second.type == t) return it->second.address;
	else if (t == SymbolType::Array && it->second.type == t) throw std::runtime_error("Cannot redefine array!");
	throw std::runtime_error("Invalid variable type!");
}

void Parser::parseFactor() {
	TokenType cur = current().type;

	if (cur == TokenType::Not) {
		advance();
		parseFactor();
		target_bytecode->push_back("NOT");
	}
	else if (cur == TokenType::BNot) {
		advance();
		parseFactor();
		target_bytecode->push_back("BNOT");
	}
	else if (cur == TokenType::PlusPlus || cur == TokenType::MinusMinus) {
		std::string in_or_dePush;
		if (cur == TokenType::PlusPlus) in_or_dePush = "PUSH 1";
		else in_or_dePush = "PUSH -1";

		if (pos + 2 < token_vec.size() && token_vec[pos + 1].type == TokenType::Identifier
			&& token_vec[pos + 2].type != TokenType::LParen) {
			advance();

			const std::string& id_name = current().text;
			advance();
			if (current().type != TokenType::LBracket) {

				size_t addr = getVar(id_name);
				target_bytecode->push_back("PUSH " + std::to_string(addr));
				target_bytecode->push_back("LOAD");

				target_bytecode->push_back(std::move(in_or_dePush));
				target_bytecode->push_back("ADD");
				target_bytecode->push_back("DUP");

				target_bytecode->push_back("PUSH " + std::to_string(addr));
				target_bytecode->push_back("STORE");
			}
			else {
				advance();

				parseAssign();
				target_bytecode->push_back("DUP");

				expect(TokenType::RBracket, "Expected ]!");

				size_t addr = getVar(id_name, SymbolType::Array);
				target_bytecode->push_back("PUSH " + std::to_string(addr));
				target_bytecode->push_back("LOAD_INDEX");

				target_bytecode->push_back(std::move(in_or_dePush));
				target_bytecode->push_back("ADD");
				target_bytecode->push_back("DUP");
				target_bytecode->push_back("SWAP3");

				target_bytecode->push_back("PUSH " + std::to_string(addr));
				target_bytecode->push_back("STORE_INDEX");
			}
		}
		else if (pos + 2 >= token_vec.size()) throw std::runtime_error("Expected expression or semicolon!");
		else throw std::runtime_error("Expected variable name!");
	}
	else if (cur == TokenType::Number) {
		target_bytecode->push_back("PUSH " + current().text);
		advance();
	}
	else if (cur == TokenType::Identifier) {
		const std::string& id_name = current().text;
		advance();

		TokenType cur_in_id = current().type;

		if (cur_in_id == TokenType::LParen) {
			advance();
			expect(TokenType::RParen, "Expected )!");
			target_bytecode->push_back("CALL " + id_name);
		}
		else if (cur_in_id == TokenType::LBracket) {
			advance();

			parseAssign();
			expect(TokenType::RBracket, "Expected ]!");

			size_t addr = getVar(id_name, SymbolType::Array);
			if (current().type == TokenType::PlusPlus || current().type == TokenType::MinusMinus) {
				target_bytecode->push_back("DUP");

				std::string in_or_dePush;
				if (current().type == TokenType::PlusPlus) in_or_dePush = "PUSH 1";
				else in_or_dePush = "PUSH -1";

				advance();

				target_bytecode->push_back("PUSH " + std::to_string(addr));
				target_bytecode->push_back("LOAD_INDEX");

				target_bytecode->push_back("DUP");
				target_bytecode->push_back(std::move(in_or_dePush));
				target_bytecode->push_back("ADD");

				target_bytecode->push_back("SWAP");
				target_bytecode->push_back("SWAP3");

				target_bytecode->push_back("PUSH " + std::to_string(addr));
				target_bytecode->push_back("STORE_INDEX");
			}
			else {
				target_bytecode->push_back("PUSH " + std::to_string(addr));
				target_bytecode->push_back("LOAD_INDEX");
			}
		}
		else if (cur_in_id == TokenType::PlusPlus || cur_in_id == TokenType::MinusMinus) {
			std::string in_or_dePush;
			if (cur_in_id == TokenType::PlusPlus) in_or_dePush = "PUSH 1";
			else in_or_dePush = "PUSH -1";

			advance();

			size_t addr = getVar(id_name);
			target_bytecode->push_back("PUSH " + std::to_string(addr));
			target_bytecode->push_back("LOAD");

			target_bytecode->push_back("DUP");
			target_bytecode->push_back(std::move(in_or_dePush));
			target_bytecode->push_back("ADD");

			target_bytecode->push_back("PUSH " + std::to_string(addr));
			target_bytecode->push_back("STORE");
		}
		else {
			size_t addr = getVar(id_name);
			target_bytecode->push_back("PUSH " + std::to_string(addr));
			target_bytecode->push_back("LOAD");
		}
	}
	else if (cur == TokenType::LParen) {
		advance();
		parseAssign();
		expect(TokenType::RParen, "Expected )!");
	}
	else throw std::runtime_error("Expected expression or number!");
}

void Parser::parseTerm() {
	parseFactor();
	while (current().type == TokenType::Star || current().type == TokenType::Slash) {
		TokenType cur = current().type;
		advance();
		parseFactor();
		if (cur == TokenType::Star) {
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
		TokenType cur = current().type;
		advance();
		parseTerm();
		if (cur == TokenType::Plus) {
			target_bytecode->push_back("ADD");
		}
		else {
			target_bytecode->push_back("SUB");
		}
	}
}

void Parser::parseComparison() {
	parseExpression();
	TokenType cur = current().type;
	while (cur == TokenType::Less || cur == TokenType::Greater
		  || cur == TokenType::Equal || cur == TokenType::LessEqual
		  || cur == TokenType::GreaterEqual || cur == TokenType::NotEqual) {
		advance();
		parseExpression();
		if (cur == TokenType::Less) {
			target_bytecode->push_back("LT");
		}
		else if (cur == TokenType::LessEqual) {
			target_bytecode->push_back("LE");
		}
		else if (cur == TokenType::Greater) {
			target_bytecode->push_back("GT");
		}
		else if (cur == TokenType::GreaterEqual) {
			target_bytecode->push_back("GE");
		}
		else if(cur == TokenType::Equal){
			target_bytecode->push_back("EQ");
		}
		else target_bytecode->push_back("NE");

		cur = current().type;
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

void Parser::parseAssign() {
	if (current().type == TokenType::Identifier && pos + 1 < token_vec.size() &&
		(token_vec[pos + 1].type == TokenType::Assign ||
		token_vec[pos + 1].type == TokenType::PlusAssign ||
		token_vec[pos + 1].type == TokenType::MinusAssign || 
		token_vec[pos + 1].type == TokenType::StarAssign ||
		token_vec[pos + 1].type == TokenType::SlashAssign ||
		token_vec[pos + 1].type == TokenType::BAndAssign || 
		token_vec[pos + 1].type == TokenType::BOrAssign || 
		token_vec[pos + 1].type == TokenType::BXorAssign)) {

		std::string id_name = current().text;
		advance();
		TokenType op = current().type;
		advance();

		if (op == TokenType::Assign) {
			parseAssign();
			size_t addr = declare_or_getVar(id_name);
			target_bytecode->push_back("DUP");
			target_bytecode->push_back("PUSH " + std::to_string(addr));
			target_bytecode->push_back("STORE");
		}
		else {
			size_t addr = getVar(id_name);
			target_bytecode->push_back("PUSH " + std::to_string(addr));
			target_bytecode->push_back("LOAD");

			parseAssign();
			target_bytecode->push_back(assignType(op));
			target_bytecode->push_back("DUP");

			target_bytecode->push_back("PUSH " + std::to_string(addr));
			target_bytecode->push_back("STORE");
		}
	}
	else if (current().type == TokenType::Identifier && pos + 1 < token_vec.size() &&
		token_vec[pos + 1].type == TokenType::LBracket) {

		size_t rollback_pos = pos;
		std::vector<std::string>* copy_ptr = target_bytecode;
		std::vector<std::string> check_vector;
		target_bytecode = &check_vector;

		std::string id_name = current().text;
		advance();
		advance();

		parseAssign();

		expect(TokenType::RBracket, "Expected ]!");

		target_bytecode = copy_ptr;
		TokenType cur = current().type;
		if (cur == TokenType::Assign) {

			advance();
			parseAssign();
			target_bytecode->push_back("DUP");

			target_bytecode->insert(target_bytecode->end(), check_vector.begin(), check_vector.end());
			
			size_t addr = getVar(id_name, SymbolType::Array);
			target_bytecode->push_back("PUSH " + std::to_string(addr));
			target_bytecode->push_back("STORE_INDEX");
		}
		else if (cur == TokenType::PlusAssign ||
			cur == TokenType::MinusAssign ||
			cur == TokenType::StarAssign ||
			cur == TokenType::SlashAssign ||
			cur == TokenType::BAndAssign ||
			cur == TokenType::BOrAssign ||
			cur == TokenType::BXorAssign ) {

			advance();

			target_bytecode->insert(target_bytecode->end(), check_vector.begin(), check_vector.end());
			target_bytecode->push_back("DUP");

			size_t addr = getVar(id_name, SymbolType::Array);
			target_bytecode->push_back("PUSH " + std::to_string(addr));
			target_bytecode->push_back("LOAD_INDEX");

			parseAssign();
			target_bytecode->push_back(assignType(cur));
			target_bytecode->push_back("DUP");
			target_bytecode->push_back("SWAP3");

			target_bytecode->push_back("PUSH " + std::to_string(addr));
			target_bytecode->push_back("STORE_INDEX");
		}
		else {
			pos = rollback_pos;
			parseOr();
		}
	}
	else {
		parseOr();
	}
}

void Parser::parseStatement() {
	TokenType cur = current().type;

	if (cur == TokenType::Print) {
		advance();

		const Token& print_expr = current();
		if (print_expr.type == TokenType::String) {
			target_bytecode->reserve(target_bytecode->size() + print_expr.text.size() * 2);
			for (unsigned char sym : print_expr.text) {
				target_bytecode->push_back("PUSH " + std::to_string(sym));
				target_bytecode->push_back("PRINT_CHAR");
			}
			advance();
			expect(TokenType::Semicolon, "Expected semicolon!");
		}
		else {
			parseAssign();
			expect(TokenType::Semicolon, "Expected semicolon!");
			target_bytecode->push_back("PRINT");
		}
	}
	else if (cur == TokenType::Return) {
		advance();
		parseAssign();
		expect(TokenType::Semicolon, "Expected semicolon!");
		target_bytecode->push_back("RET");
	}
	else if (cur == TokenType::If) {
		advance();
		expect(TokenType::LParen, "Expected (!");
		parseAssign();
		expect(TokenType::RParen, "Expected )!");

		target_bytecode->push_back("JZ ");
		size_t if_else_index = target_bytecode->size() - 1;

		expect(TokenType::LBrace, "Expected {!");
		parseBlock();
		expect(TokenType::RBrace, "Expected }!");

		std::string if_number_to_str = std::to_string(if_counter++);
		target_bytecode->push_back("JMP end_if_" + if_number_to_str);

		if (current().type == TokenType::Else) {
			(*target_bytecode)[if_else_index] += ("else_" + if_number_to_str);

			target_bytecode->push_back("else_" + if_number_to_str + ':');
			advance();
			expect(TokenType::LBrace, "Expected {!");
			parseBlock();
			expect(TokenType::RBrace, "Expected }!");
		}
		else (*target_bytecode)[if_else_index] += ("end_if_" + if_number_to_str);

		target_bytecode->push_back("end_if_" + std::move(if_number_to_str) + ':');
	}
	else if (cur == TokenType::While) {
		advance();

		std::string while_number_to_str = std::to_string(while_counter++);
		target_bytecode->push_back("while_" + while_number_to_str + ':');
		expect(TokenType::LParen, "Expected (!");
		parseAssign();
		expect(TokenType::RParen, "Expected )!");
		target_bytecode->push_back("JZ end_while_" + while_number_to_str);

		expect(TokenType::LBrace, "Expected {!");
		parseBlock();
		expect(TokenType::RBrace, "Expected }!");
		target_bytecode->push_back("JMP while_" + while_number_to_str);

		target_bytecode->push_back("end_while_" + std::move(while_number_to_str) + ':');
	}
	else if (cur == TokenType::Array) {

		advance();
		const std::string& id_name = current().text;
		expect(TokenType::Identifier, "Expected array name!");
		expect(TokenType::LBracket, "Expected [!");
		parseAssign();
		expect(TokenType::RBracket, "Expected ]!");
		expect(TokenType::Semicolon, "Expected semicolon!");

		size_t addr = declare_or_getVar(id_name, SymbolType::Array);
		target_bytecode->push_back("PUSH " + std::to_string(addr));
		target_bytecode->push_back("ALLOC");
	}
	else {
		parseAssign();
		expect(TokenType::Semicolon, "Expected semicolon!");
		target_bytecode->push_back("POP");
	}
}

void Parser::parseBlock() {
	while (current().type != TokenType::RBrace && current().type != TokenType::End)
		parseStatement();
}

void Parser::parseFunctionDecl() {
	advance();
	const std::string& func_name = current().text;
	expect(TokenType::Identifier, "Expected function name!");

	target_bytecode = &func_bytecode;
	target_bytecode->push_back(func_name + ':');

	expect(TokenType::LParen, "Expected (!");
	expect(TokenType::RParen, "Expected )!");
	expect(TokenType::LBrace, "Expected {!");
	parseBlock();
	expect(TokenType::RBrace, "Expected }!");

	if (target_bytecode->empty() || target_bytecode->back() != "RET") {
		target_bytecode->push_back("PUSH 0");
		target_bytecode->push_back("RET");
	}
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

const std::vector<std::string>& Parser::getBytecode() const{
	return final_bytecode;
}

std::string Parser::assignType(TokenType op) {
	switch (op) {
	case TokenType::PlusAssign:
		return "ADD";
	case TokenType::MinusAssign:
		return "SUB";
	case TokenType::StarAssign:
		return "MUL";
	case TokenType::SlashAssign:
		return "DIV";
	case TokenType::BAndAssign:
		return "BAND";
	case TokenType::BOrAssign:
		return "BOR";
	case TokenType::BXorAssign:
		return "BXOR";
	default:
		throw std::runtime_error("Unknown symbol!");
	}
}