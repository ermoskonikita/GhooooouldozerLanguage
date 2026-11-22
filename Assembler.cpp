#include "Assembler.h"
#include <sstream>
#include <stdexcept>

void Assembler::PushCaseArg(std::vector<uint8_t>& bytecode, std::unordered_map<std::string, int32_t>::iterator it, int32_t position) {
	uint32_t arg = it->second - (position + 1);
	for (int k = 0; k <= 3; k++) {
		bytecode.push_back(static_cast<uint8_t>(arg));
		arg >>= 8;
	}
}

void Assembler::PushArg(std::vector<uint8_t>& bytecode, size_t& i) {
	uint32_t arg = std::stoi(op_vector[i]);
	for (int k = 0; k <= 3; k++) {
		bytecode.push_back(static_cast<uint8_t>(arg));
		arg >>= 8;
	}
}

Assembler::Assembler(const std::vector<std::string>& str_vec) {
	std::stringstream buf;
	for (const auto& str : str_vec) buf << str << ' ';

	std::string temp;
	while (buf >> temp) {
		op_vector.push_back(std::move(temp));
	}
}

std::vector<uint8_t> Assembler::Compile() {
	std::vector<uint8_t> bytecode;
	std::unordered_map<std::string, int32_t> cases;

	size_t size = op_vector.size();
	int32_t position = 0;

	for (size_t i = 0; i < size; i++) {
		const std::string& str = op_vector[i];
		if (str == "PUSH" || str == "JMP" || str == "JZ" || str == "JNZ" || str == "CALL" ) {
			position += 5;
			i++;
		}
		else if (str == "POP" || str == "DUP" || str == "ADD" ||
			str == "SUB" || str == "MUL" || str == "DIV" || str == "MOD" || str == "NEG" ||
			str == "BAND" || str == "BOR" || str == "BXOR" ||
			str == "BNOT" || str == "SHL" || str == "SHR" || str == "LT" || str == "LE" || str == "GT" ||
			str == "GE" || str == "EQ" || str == "NE" || str == "AND" || str == "OR" ||
			str == "NOT" || str == "PRINT" || str == "PRINT_CHAR" || str == "INPUT" ||
			str == "LEN" || str == "RET" || str == "LOAD" || str == "STORE" || str == "LOAD_INDEX" ||
			str == "STORE_INDEX" || str == "ALLOC" || str == "ENTER" || str == "LOAD_LOCAL" ||
			str == "STORE_LOCAL" || str == "LOAD_LOCAL_INDEX" || str == "STORE_LOCAL_INDEX" ||
			str == "ALLOC_LOCAL" || str == "SWAP" || str == "SWAP3" || str == "HALT") position++;
		else if (op_vector[i].back() == ':') {
			std::string c{ op_vector[i].begin(),op_vector[i].end() - 1 };
			if (cases.count(c)) {
				throw std::runtime_error("Assembler: Fatal error! Repeating label!");
			}
			cases.emplace(c, position);
		}
		else throw std::runtime_error("Assembler: Fatal error! Unknown operation!");
	}

	bytecode.reserve(position);
	position = 0;
	for (size_t i = 0; i < size; i++) {
		const std::string& str = op_vector[i];

		if (str == "PUSH") {
			if (++i == size) {
				throw std::runtime_error("Assembler: Fatal error!");
			}
			bytecode.push_back(static_cast<uint8_t>(OpCode::PUSH));
			try { PushArg(bytecode, i); }
			catch (...) {
				throw std::runtime_error("Assembler: Fatal error!");
			}
			position += 5;
		}
		else if (str == "POP") { bytecode.push_back(static_cast<uint8_t>(OpCode::POP)); position++; }
		else if (str == "DUP") { bytecode.push_back(static_cast<uint8_t>(OpCode::DUP)); position++; }
		else if (str == "ADD") { bytecode.push_back(static_cast<uint8_t>(OpCode::ADD)); position++; }
		else if (str == "SUB") { bytecode.push_back(static_cast<uint8_t>(OpCode::SUB)); position++; }
		else if (str == "MUL") { bytecode.push_back(static_cast<uint8_t>(OpCode::MUL)); position++; }
		else if (str == "DIV") { bytecode.push_back(static_cast<uint8_t>(OpCode::DIV)); position++; }
		else if (str == "MOD") { bytecode.push_back(static_cast<uint8_t>(OpCode::MOD)); position++; }
		else if (str == "NEG") { bytecode.push_back(static_cast<uint8_t>(OpCode::NEG)); position++; }
		else if (str == "BAND") { bytecode.push_back(static_cast<uint8_t>(OpCode::BAND)); position++; }
		else if (str == "BOR") { bytecode.push_back(static_cast<uint8_t>(OpCode::BOR)); position++; }
		else if (str == "BXOR") { bytecode.push_back(static_cast<uint8_t>(OpCode::BXOR)); position++; }
		else if (str == "BNOT") { bytecode.push_back(static_cast<uint8_t>(OpCode::BNOT)); position++; }
		else if (str == "SHL") { bytecode.push_back(static_cast<uint8_t>(OpCode::SHL)); position++; }
		else if (str == "SHR") { bytecode.push_back(static_cast<uint8_t>(OpCode::SHR)); position++; }
		else if (str == "LT") { bytecode.push_back(static_cast<uint8_t>(OpCode::LT)); position++; }
		else if (str == "LE") { bytecode.push_back(static_cast<uint8_t>(OpCode::LE)); position++; }
		else if (str == "GT") { bytecode.push_back(static_cast<uint8_t>(OpCode::GT)); position++; }
		else if (str == "GE") { bytecode.push_back(static_cast<uint8_t>(OpCode::GE)); position++; }
		else if (str == "EQ") { bytecode.push_back(static_cast<uint8_t>(OpCode::EQ)); position++; }
		else if (str == "NE") { bytecode.push_back(static_cast<uint8_t>(OpCode::NE)); position++; }
		else if (str == "AND") { bytecode.push_back(static_cast<uint8_t>(OpCode::AND)); position++; }
		else if (str == "OR") { bytecode.push_back(static_cast<uint8_t>(OpCode::OR)); position++; }
		else if (str == "NOT") { bytecode.push_back(static_cast<uint8_t>(OpCode::NOT)); position++; }
		else if (str == "JMP") {
			if (++i == size) {
				throw std::runtime_error("Assembler: Fatal error!");
			}
			bytecode.push_back(static_cast<uint8_t>(OpCode::JMP));
			if (auto it = cases.find(op_vector[i]); it != cases.end()) {
				PushCaseArg(bytecode, it, position);
			}
			else {
				try { PushArg(bytecode, i); }
				catch (...) {
					throw std::runtime_error("Assembler: Fatal error!");
				}
			}
			position += 5;
		}
		else if (str == "JZ") {
			if (++i == size) {
				throw std::runtime_error("Assembler: Fatal error!");
			}
			bytecode.push_back(static_cast<uint8_t>(OpCode::JZ));
			if (auto it = cases.find(op_vector[i]); it != cases.end()) {
				PushCaseArg(bytecode, it, position);
			}
			else {
				try { PushArg(bytecode, i); }
				catch (...) { 
					throw std::runtime_error("Assembler: Fatal error!");
				}
			}
			position += 5;
		}
		else if (str == "JNZ") {
			if (++i == size) {
				throw std::runtime_error("Assembler: Fatal error!");
			}
			bytecode.push_back(static_cast<uint8_t>(OpCode::JNZ));
			if (auto it = cases.find(op_vector[i]); it != cases.end()) {
				PushCaseArg(bytecode, it, position);
			}
			else {
				try { PushArg(bytecode, i); }
				catch (...) {
					throw std::runtime_error("Assembler: Fatal error!");
				}
			}
			position += 5;
		}
		else if (str == "PRINT") { bytecode.push_back(static_cast<uint8_t>(OpCode::PRINT)); position++; }
		else if (str == "PRINT_CHAR") { bytecode.push_back(static_cast<uint8_t>(OpCode::PRINT_CHAR)); position++; }
		else if (str == "INPUT") { bytecode.push_back(static_cast<uint8_t>(OpCode::INPUT)); position++; }
		else if (str == "LEN") { bytecode.push_back(static_cast<uint8_t>(OpCode::LEN)); position++; }
		else if (str == "LOAD") { bytecode.push_back(static_cast<uint8_t>(OpCode::LOAD)); position++; } 
		else if (str == "STORE") { bytecode.push_back(static_cast<uint8_t>(OpCode::STORE)); position++; }
		else if (str == "LOAD_INDEX") { bytecode.push_back(static_cast<uint8_t>(OpCode::LOAD_INDEX)); position++; }
		else if (str == "STORE_INDEX") { bytecode.push_back(static_cast<uint8_t>(OpCode::STORE_INDEX)); position++; }
		else if (str == "ALLOC") { bytecode.push_back(static_cast<uint8_t>(OpCode::ALLOC)); position++; }
		else if (str == "ENTER") { bytecode.push_back(static_cast<uint8_t>(OpCode::ENTER)); position++; }
		else if (str == "LOAD_LOCAL") { bytecode.push_back(static_cast<uint8_t>(OpCode::LOAD_LOCAL)); position++; }
		else if (str == "STORE_LOCAL") { bytecode.push_back(static_cast<uint8_t>(OpCode::STORE_LOCAL)); position++; }
		else if (str == "LOAD_LOCAL_INDEX") { bytecode.push_back(static_cast<uint8_t>(OpCode::LOAD_LOCAL_INDEX)); position++; }
		else if (str == "STORE_LOCAL_INDEX") { bytecode.push_back(static_cast<uint8_t>(OpCode::STORE_LOCAL_INDEX)); position++; }
		else if (str == "ALLOC_LOCAL") { bytecode.push_back(static_cast<uint8_t>(OpCode::ALLOC_LOCAL)); position++; }
		else if(str == "SWAP") { bytecode.push_back(static_cast<uint8_t>(OpCode::SWAP)); position++; }
		else if (str == "SWAP3") { bytecode.push_back(static_cast<uint8_t>(OpCode::SWAP3)); position++; }
		else if (str == "CALL") {
			if (++i == size) {
				throw std::runtime_error("Assembler: Fatal error! Undeclared function!");
			}
			bytecode.push_back(static_cast<uint8_t>(OpCode::CALL));
			if (auto it = cases.find(op_vector[i]); it != cases.end()) {
				PushCaseArg(bytecode, it, position);
				position += 5;
			}
			else {
				throw std::runtime_error("Assembler: Fatal error! Undeclared function!");
			}
		}
		else if (str == "RET") { bytecode.push_back(static_cast<uint8_t>(OpCode::RET)); position++; }
		else if (str == "HALT") { bytecode.push_back(static_cast<uint8_t>(OpCode::HALT)); position++; }
		else if (str.back() == ':') continue;
		else {
			throw std::runtime_error("Assembler: Fatal error! Unknown operation!");
		}
	}

	return bytecode;
}