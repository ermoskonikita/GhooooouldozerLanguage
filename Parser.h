#pragma once
#include "Token.h"
#include <vector>
#include <string>
#include <unordered_map>

enum class SymbolType {
	Scalar,
	Array
};

struct Symbol {
	SymbolType type;
	size_t address;
};

struct VarInfo {
	size_t address;
	bool is_local;
};

struct FuncInfo {
	std::vector<SymbolType> param_types;
};

class Parser{
public:
	Parser(std::vector<Token> t_vec);
	void parse();
	const std::vector<std::string>& getBytecode() const;
private:
	std::vector<Token> token_vec;
	size_t pos = 0;

	std::vector<std::string> main_bytecode;
	std::vector<std::string> func_bytecode;
	std::vector<std::string> final_bytecode;
	std::vector<std::string>* target_bytecode;

	std::unordered_map<std::string, Symbol> symbol_table;
	std::unordered_map<std::string, Symbol> local_symbol_table;
	std::unordered_map<std::string, FuncInfo> func_table;

	size_t next_free_address = 0;
	size_t next_free_local_address = 0;
	bool in_function = false;

	VarInfo getVar(const std::string& str, SymbolType t = SymbolType::Scalar);
	VarInfo declare_or_getVar(const std::string& str, SymbolType t = SymbolType::Scalar);

	void emitLoad(const VarInfo& var);
	void emitStore(const VarInfo& var);
	void emitLoadIndex(const VarInfo& var);
	void emitStoreIndex(const VarInfo& var);
	void emitAlloc(const VarInfo& var);

	size_t if_counter = 0;
	size_t while_counter = 0;

	const Token& current();
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
	void parseAssign();
	void parseStatement();
	void parseBlock();
	void parseFunctionDecl();
	void collectFunctions();

	std::string assignType(TokenType op);
};