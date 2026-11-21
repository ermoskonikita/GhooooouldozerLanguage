#include "Parser.h"
#include <stdexcept>
#include <iostream>
#include <unordered_set>

Parser::Parser(std::vector<Token> t_vec) : token_vec(std::move(t_vec)),target_bytecode(&main_bytecode) {}

const Token& Parser::current() { return token_vec[pos]; }
void Parser::advance() { pos++; }

void Parser::expect(TokenType type, const std::string& error_msg) {
	if (current().type == type) advance();
	else throw std::runtime_error(error_msg);
}

VarInfo Parser::getVar(const std::string& str, SymbolType t) {
	if (in_function) {
		if (auto it = local_symbol_table.find(str); it != local_symbol_table.end()) {
			if (it->second.type == t) return { it->second.address,true };
			else throw std::runtime_error("Invalid variable type!");
		}
	}

	if (auto it = symbol_table.find(str); it == symbol_table.end()) {
		throw std::runtime_error("Undeclared variable!");
	}
	else if (it->second.type == t) return { it->second.address,false };
	throw std::runtime_error("Invalid variable type!");
}

VarInfo Parser::declare_or_getVar(const std::string& str, SymbolType t) {
	auto global_it = symbol_table.find(str);

	if (in_function) {
		if (t == SymbolType::Array) {
			if (local_symbol_table.count(str)) {
				throw std::runtime_error("Cannot redefine array!");
			}
			size_t addr = next_free_local_address++;
			local_symbol_table.emplace(str, Symbol{ t,addr });
			return { addr,true };
		}

		auto local_it = local_symbol_table.find(str);
		if (local_it == local_symbol_table.end()) {
			if (global_it != symbol_table.end() && global_it->second.type == t) {
				return { global_it->second.address,false };
			}
			else if (global_it != symbol_table.end() && global_it->second.type != t) {
				throw std::runtime_error("Invalid variable type!");
			}
			else {
				size_t addr = next_free_local_address++;
				local_symbol_table.emplace(str, Symbol{ t,addr });
				return { addr,true };
			}
		}
		else if (t == SymbolType::Scalar && local_it->second.type == t) return { local_it->second.address,true };
		throw std::runtime_error("Invalid variable type!");
	}

	if (global_it == symbol_table.end()) {
		size_t addr = next_free_address++;
		symbol_table.emplace(str, Symbol{t,addr});
		return { addr,false };
	}
	else if (t == SymbolType::Scalar && global_it->second.type == t) return { global_it->second.address,false };
	else if (t == SymbolType::Array && global_it->second.type == t) throw std::runtime_error("Cannot redefine array!");
	throw std::runtime_error("Invalid variable type!");
}

void Parser::emitLoad(const VarInfo& var) {
	target_bytecode->push_back("PUSH " + std::to_string(var.address));
	if(var.is_local) target_bytecode->push_back("LOAD_LOCAL");
	else target_bytecode->push_back("LOAD");
}

void Parser::emitStore(const VarInfo& var) {
	target_bytecode->push_back("PUSH " + std::to_string(var.address));
	if (var.is_local) target_bytecode->push_back("STORE_LOCAL");
	else target_bytecode->push_back("STORE");
}

void Parser::emitLoadIndex(const VarInfo& var) {
	target_bytecode->push_back("PUSH " + std::to_string(var.address));
	if (var.is_local) target_bytecode->push_back("LOAD_LOCAL_INDEX");
	else target_bytecode->push_back("LOAD_INDEX");
}

void Parser::emitStoreIndex(const VarInfo& var) {
	target_bytecode->push_back("PUSH " + std::to_string(var.address));
	if (var.is_local) target_bytecode->push_back("STORE_LOCAL_INDEX");
	else target_bytecode->push_back("STORE_INDEX");
}

void Parser::emitAlloc(const VarInfo& var) {
	target_bytecode->push_back("PUSH " + std::to_string(var.address));
	if (var.is_local) target_bytecode->push_back("ALLOC_LOCAL");
	else target_bytecode->push_back("ALLOC");
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
	else if (cur == TokenType::Minus) {
		advance();
		parseFactor();
		target_bytecode->push_back("NEG");
	}
	else if (cur == TokenType::Input) {
		advance();
		target_bytecode->push_back("INPUT");
	}
	else if (cur == TokenType::Length) {
		advance();
		expect(TokenType::LParen, "Expected (!");
		const std::string& id_name = current().text;
		expect(TokenType::Identifier, "Expected variable name!");
		expect(TokenType::RParen, "Expected )!");

		VarInfo val = getVar(id_name, SymbolType::Array);
		emitLoad(val);
		target_bytecode->push_back("LEN");
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

				VarInfo var = getVar(id_name);
				emitLoad(var);

				target_bytecode->push_back(std::move(in_or_dePush));
				target_bytecode->push_back("ADD");
				target_bytecode->push_back("DUP");

				emitStore(var);
			}
			else {
				advance();

				parseAssign();
				target_bytecode->push_back("DUP");

				expect(TokenType::RBracket, "Expected ]!");

				VarInfo var = getVar(id_name,SymbolType::Array);
				emitLoadIndex(var);

				target_bytecode->push_back(std::move(in_or_dePush));
				target_bytecode->push_back("ADD");
				target_bytecode->push_back("DUP");
				target_bytecode->push_back("SWAP3");

				emitStoreIndex(var);
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

			auto it = func_table.find(id_name);
			if (it == func_table.end()) {
				throw std::runtime_error("Undeclared function!");
			}

			advance();

			const auto& func_params = it->second.param_types;
			size_t params_size = func_params.size();
			for (size_t i = 0;i < params_size;i++) {
				if (func_params[i] == SymbolType::Array) {
					const std::string& arr_name = current().text;
					expect(TokenType::Identifier, "Expected array name!");
					VarInfo arr = getVar(arr_name, SymbolType::Array);
					emitLoad(arr);
				}
				else {
					parseAssign();
				}

				if (i + 1 < params_size) expect(TokenType::Comma, "Expected comma!");
			}

			expect(TokenType::RParen, "Expected )!");
			target_bytecode->push_back("CALL " + id_name);
		}
		else if (cur_in_id == TokenType::LBracket) {
			advance();

			parseAssign();
			expect(TokenType::RBracket, "Expected ]!");

			VarInfo var = getVar(id_name, SymbolType::Array);
			if (current().type == TokenType::PlusPlus || current().type == TokenType::MinusMinus) {
				target_bytecode->push_back("DUP");

				std::string in_or_dePush;
				if (current().type == TokenType::PlusPlus) in_or_dePush = "PUSH 1";
				else in_or_dePush = "PUSH -1";

				advance();

				emitLoadIndex(var);

				target_bytecode->push_back("DUP");
				target_bytecode->push_back(std::move(in_or_dePush));
				target_bytecode->push_back("ADD");

				target_bytecode->push_back("SWAP");
				target_bytecode->push_back("SWAP3");

				emitStoreIndex(var);
			}
			else {
				emitLoadIndex(var);
			}
		}
		else if (cur_in_id == TokenType::PlusPlus || cur_in_id == TokenType::MinusMinus) {
			std::string in_or_dePush;
			if (cur_in_id == TokenType::PlusPlus) in_or_dePush = "PUSH 1";
			else in_or_dePush = "PUSH -1";

			advance();

			VarInfo var = getVar(id_name);
			emitLoad(var);

			target_bytecode->push_back("DUP");
			target_bytecode->push_back(std::move(in_or_dePush));
			target_bytecode->push_back("ADD");

			emitStore(var);
		}
		else {
			VarInfo var = getVar(id_name);
			emitLoad(var);
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
	TokenType cur = current().type;
	while (cur == TokenType::Star || cur == TokenType::Slash || cur == TokenType::Percent) {
		advance();
		parseFactor();
		if (cur == TokenType::Star) {
			target_bytecode->push_back("MUL");
		}
		else if(cur == TokenType::Slash) {
			target_bytecode->push_back("DIV");
		}
		else {
			target_bytecode->push_back("MOD");
		}

		cur = current().type;
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

		std::string and_label = "and_label_" + std::to_string(label_counter++);
		target_bytecode->push_back("DUP");
		target_bytecode->push_back("JZ " + and_label);
		parseBOr();
		target_bytecode->push_back("AND");
		target_bytecode->push_back(std::move(and_label) + ':');
	}
}

void Parser::parseOr() {
	parseAnd();
	while (current().type == TokenType::Or) {
		advance();

		std::string or_label = "or_label_" + std::to_string(label_counter++);
		target_bytecode->push_back("DUP");
		target_bytecode->push_back("JNZ " + or_label);
		parseAnd();
		target_bytecode->push_back("OR");
		target_bytecode->push_back(std::move(or_label) + ':');
		target_bytecode->push_back("NOT");
		target_bytecode->push_back("NOT");
	}
}

void Parser::parseAssign() {
	if (current().type == TokenType::Identifier && pos + 1 < token_vec.size() &&
		(token_vec[pos + 1].type == TokenType::Assign ||
		token_vec[pos + 1].type == TokenType::PlusAssign ||
		token_vec[pos + 1].type == TokenType::MinusAssign || 
		token_vec[pos + 1].type == TokenType::StarAssign ||
		token_vec[pos + 1].type == TokenType::SlashAssign ||
		token_vec[pos + 1].type == TokenType::PercentAssign ||
		token_vec[pos + 1].type == TokenType::BAndAssign || 
		token_vec[pos + 1].type == TokenType::BOrAssign || 
		token_vec[pos + 1].type == TokenType::BXorAssign)) {

		const std::string& id_name = current().text;
		advance();
		TokenType op = current().type;
		advance();

		if (op == TokenType::Assign) {
			parseAssign();

			VarInfo var = declare_or_getVar(id_name);
			target_bytecode->push_back("DUP");
			
			emitStore(var);
		}
		else {
			VarInfo var = getVar(id_name);
			emitLoad(var);

			parseAssign();
			target_bytecode->push_back(assignType(op));
			target_bytecode->push_back("DUP");

			emitStore(var);
		}
	}
	else if (current().type == TokenType::Identifier && pos + 1 < token_vec.size() &&
		token_vec[pos + 1].type == TokenType::LBracket) {

		size_t rollback_pos = pos;
		std::vector<std::string>* copy_ptr = target_bytecode;
		std::vector<std::string> check_vector;
		target_bytecode = &check_vector;

		const std::string& id_name = current().text;
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
			
			VarInfo var = getVar(id_name, SymbolType::Array);
			emitStoreIndex(var);
		}
		else if (cur == TokenType::PlusAssign ||
			cur == TokenType::MinusAssign ||
			cur == TokenType::StarAssign ||
			cur == TokenType::SlashAssign ||
			cur == TokenType::PercentAssign ||
			cur == TokenType::BAndAssign ||
			cur == TokenType::BOrAssign ||
			cur == TokenType::BXorAssign ) {

			advance();

			target_bytecode->insert(target_bytecode->end(), check_vector.begin(), check_vector.end());
			target_bytecode->push_back("DUP");

			VarInfo var = getVar(id_name, SymbolType::Array);
			emitLoadIndex(var);

			parseAssign();
			target_bytecode->push_back(assignType(cur));
			target_bytecode->push_back("DUP");
			target_bytecode->push_back("SWAP3");

			emitStoreIndex(var);
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
		if (!in_function) {
			throw std::runtime_error("Return outside of function!");
		}

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
		std::string if_number_to_str = std::to_string(if_counter++);

		expect(TokenType::LBrace, "Expected {!");
		parseBlock();
		expect(TokenType::RBrace, "Expected }!");

		target_bytecode->push_back("JMP end_if_" + if_number_to_str);

		bool find_else = false;
		bool find_else_if = false;
		if (current().type == TokenType::Else && pos + 1 < token_vec.size() &&
			token_vec[pos + 1].type == TokenType::If)  {
			advance();

			find_else_if = true;
			size_t number_of_else_if = 0;
			(*target_bytecode)[if_else_index] += ("else_if_" + if_number_to_str + "_0");

			while (current().type == TokenType::If) {
				advance();

				std::string number_of_else_if_to_str = std::to_string(number_of_else_if++);
				target_bytecode->push_back("else_if_" + if_number_to_str + '_' +
					number_of_else_if_to_str + ':');

				expect(TokenType::LParen, "Expected (!");
				parseAssign();
				expect(TokenType::RParen, "Expected )!");

				target_bytecode->push_back("JZ end_else_if_" + if_number_to_str + '_' + number_of_else_if_to_str);

				expect(TokenType::LBrace, "Expected {!");
				parseBlock();
				expect(TokenType::RBrace, "Expected }!");

				target_bytecode->push_back("JMP end_if_" + if_number_to_str);

				target_bytecode->push_back("end_else_if_" + if_number_to_str + '_' +
					number_of_else_if_to_str + ':');

				if (current().type == TokenType::Else && pos + 1 < token_vec.size() &&
					token_vec[pos + 1].type == TokenType::If) {
					advance();
				}
			}
		}
		if (current().type == TokenType::Else) {
			advance();

			find_else = true;
			
			if(!find_else_if) (*target_bytecode)[if_else_index] += ("else_" + if_number_to_str);

			target_bytecode->push_back("else_" + if_number_to_str + ':');
			expect(TokenType::LBrace, "Expected {!");
			parseBlock();
			expect(TokenType::RBrace, "Expected }!");
		}
		
		if(!find_else && !find_else_if) (*target_bytecode)[if_else_index] += ("end_if_" + if_number_to_str);

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

		VarInfo var = declare_or_getVar(id_name, SymbolType::Array);
		emitAlloc(var);
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

void Parser::collectFunctions() {
	std::unordered_set<std::string> param_names;
	while (current().type != TokenType::End) {

		if (current().type == TokenType::Func) {
			advance();
			const std::string& func_name = current().text;
			expect(TokenType::Identifier, "Expected function name!");

			if (func_table.count(func_name)) throw std::runtime_error("Redefine function!");
			expect(TokenType::LParen, "Expected (!");

			std::vector<SymbolType> param_types;
			TokenType cur = current().type;
			while (cur == TokenType::Identifier || cur == TokenType::Array) {
				if (cur == TokenType::Identifier) param_types.push_back(SymbolType::Scalar);
				else {
					param_types.push_back(SymbolType::Array);
					advance();
				}

				if (param_names.count(current().text)) throw std::runtime_error("Redefine function parameter!");
				param_names.insert(current().text);

				expect(TokenType::Identifier, "Expected variable name!");
				if (current().type == TokenType::RParen) break;
				expect(TokenType::Comma, "Expected comma!");
				cur = current().type;
				if (cur != TokenType::Identifier && cur != TokenType::Array) {
					throw std::runtime_error("Expected identifier or )!");
				}
			}
			expect(TokenType::RParen, "Expected )!");

			func_table.emplace(func_name, FuncInfo{ std::move(param_types)});
			param_names.clear();
		}
		advance();
	}
	pos = 0;
}

void Parser::parseFunctionDecl() {
	in_function = true;

	advance();
	const std::string& func_name = current().text;

	expect(TokenType::Identifier, "Expected function name!");

	target_bytecode = &func_bytecode;
	target_bytecode->push_back(func_name + ':');

	expect(TokenType::LParen, "Expected (!");

	const auto& param_types = func_table.at(func_name).param_types;
	size_t params_size = param_types.size();
	for (size_t i = 0; i < params_size; i++) {
		if (param_types[i] == SymbolType::Array) advance();

		local_symbol_table.emplace(current().text, Symbol{ param_types[i], next_free_local_address++ });
		advance();

		if (i + 1 < param_types.size()) advance(); 
	}

	expect(TokenType::RParen, "Expected )!");

	size_t counter_index = target_bytecode->size();
	target_bytecode->push_back("PUSH 0");
	target_bytecode->push_back("ENTER");

	for (size_t i = params_size; i > 0; i--) {
		emitStore(VarInfo{ i - 1,true });
	}

	expect(TokenType::LBrace, "Expected {!");
	parseBlock();
	expect(TokenType::RBrace, "Expected }!");
	(*target_bytecode)[counter_index] = "PUSH " + 
		std::to_string(next_free_local_address);

	if (target_bytecode->empty() || target_bytecode->back() != "RET") {
		target_bytecode->push_back("PUSH 0");
		target_bytecode->push_back("RET");
	}
	target_bytecode = &main_bytecode;

	next_free_local_address = 0;
	local_symbol_table.clear();
	in_function = false;
}

void Parser::parse() {
	collectFunctions();

	while (current().type != TokenType::End) {
		if (current().type == TokenType::Func) parseFunctionDecl();
		else parseStatement();
	}

	main_bytecode.push_back("HALT");
	final_bytecode = std::move(main_bytecode);
	final_bytecode.insert(final_bytecode.end(), func_bytecode.begin(), func_bytecode.end());
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
	case TokenType::PercentAssign:
		return "MOD";
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