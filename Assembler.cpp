#include "Assembler.h"
#include <sstream>
#include <fstream>
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

void Assembler::PushMemoryArg(std::vector<uint8_t>& bytecode, uint32_t adr) {
	for (int k = 0; k <= 3; k++) {
		bytecode.push_back(static_cast<uint8_t>(adr));
		adr >>= 8;
	}
}

Assembler::Assembler(const std::string& str) {
	std::ifstream file(str);
	if (!file.is_open()) throw std::runtime_error("Не удалось открыть файл: " + str);

	std::stringstream buf;
	buf << file.rdbuf();
	op_str = buf.str();

	size_t i;
	while ((i = op_str.find("//")) != std::string::npos) {
		size_t length = op_str.length();
		size_t l = i;
		while (l < length && op_str[l] != '\n') l++;
		op_str.erase(op_str.begin() + i, op_str.begin() + l);
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
		else if (str == "ALLOC") i += 2;
		else if (str == "LOAD" || str == "STORE") { position += 6; i++; }
		else if(str == "LOAD_INDEX" || str == "STORE_INDEX") { position += 7; i++; }
		else if (str == "POP" || str == "DUP" || str == "ADD" ||
			str == "SUB" || str == "MUL" || str == "DIV" || str == "BAND" || str == "BOR" ||
			str == "BXOR" || str == "BNOT" || str == "LT" ||str == "LE"  || str == "GT" ||
			str == "GE" || str == "EQ" || str == "NE" || str == "AND" || str == "OR" ||
			str == "NOT" || str == "PRINT" || str == "PRINT_CHAR" || str == "RET" ||
			str == "SWAP" || str == "HALT") position++;
		else if (op_vector[i].back() == ':') {
			std::string c{ op_vector[i].begin(),op_vector[i].end() - 1 };
			if (cases.count(c)) return {};
			cases.emplace(c, position);
		}
		else return {};
	}

	position = 0;
	for (size_t i = 0; i < size; i++) {
		const std::string& str = op_vector[i];

		if (str == "PUSH") {
			if (++i == size) return {};
			bytecode.push_back(static_cast<uint8_t>(OpCode::PUSH));
			try { PushArg(bytecode, i); }
			catch (...) { return {}; }
			position += 5;
		}
		else if (str == "POP") { bytecode.push_back(static_cast<uint8_t>(OpCode::POP)); position++; }
		else if (str == "DUP") { bytecode.push_back(static_cast<uint8_t>(OpCode::DUP)); position++; }
		else if (str == "ADD") { bytecode.push_back(static_cast<uint8_t>(OpCode::ADD)); position++; }
		else if (str == "SUB") { bytecode.push_back(static_cast<uint8_t>(OpCode::SUB)); position++; }
		else if (str == "MUL") { bytecode.push_back(static_cast<uint8_t>(OpCode::MUL)); position++; }
		else if (str == "DIV") { bytecode.push_back(static_cast<uint8_t>(OpCode::DIV)); position++; }
		else if (str == "BAND") { bytecode.push_back(static_cast<uint8_t>(OpCode::BAND)); position++; }
		else if (str == "BOR") { bytecode.push_back(static_cast<uint8_t>(OpCode::BOR)); position++; }
		else if (str == "BXOR") { bytecode.push_back(static_cast<uint8_t>(OpCode::BXOR)); position++; }
		else if (str == "BNOT") { bytecode.push_back(static_cast<uint8_t>(OpCode::BNOT)); position++; }
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
			if (++i == size) return {};
			bytecode.push_back(static_cast<uint8_t>(OpCode::JMP));
			if (auto it = cases.find(op_vector[i]); it != cases.end()) {
				PushCaseArg(bytecode, it, position);
			}
			else {
				try { PushArg(bytecode, i); }
				catch (...) { return {}; }
			}
			position += 5;
		}
		else if (str == "JZ") {
			if (++i == size) return {};
			bytecode.push_back(static_cast<uint8_t>(OpCode::JZ));
			if (auto it = cases.find(op_vector[i]); it != cases.end()) {
				PushCaseArg(bytecode, it, position);
			}
			else {
				try { PushArg(bytecode, i); }
				catch (...) { return {}; }
			}
			position += 5;
		}
		else if (str == "JNZ") {
			if (++i == size) return {};
			bytecode.push_back(static_cast<uint8_t>(OpCode::JNZ));
			if (auto it = cases.find(op_vector[i]); it != cases.end()) {
				PushCaseArg(bytecode, it, position);
			}
			else {
				try { PushArg(bytecode, i); }
				catch (...) { return {}; }
			}
			position += 5;
		}
		else if (str == "PRINT") { bytecode.push_back(static_cast<uint8_t>(OpCode::PRINT)); position++; }
		else if (str == "PRINT_CHAR") { bytecode.push_back(static_cast<uint8_t>(OpCode::PRINT_CHAR)); position++; }
		else if (str == "LOAD" || str == "STORE") {
			bytecode.push_back(static_cast<uint8_t>(OpCode::PUSH));
			if (++i == size) return {};

			if (auto it = variables.find(op_vector[i]);it != variables.end()) {
				PushMemoryArg(bytecode, it->second);
			}
			else {
				if (str == "LOAD") return {};
				variables.emplace(op_vector[i], next_var_address);
				PushMemoryArg(bytecode, next_var_address);
				next_var_address++;
			}

			if (str == "LOAD") bytecode.push_back(static_cast<uint8_t>(OpCode::LOAD));
			else bytecode.push_back(static_cast<uint8_t>(OpCode::STORE));

			position += 6;
		}
		else if (str == "ALLOC") {
			if (++i == size) return {};
			if (auto it = variables.find(op_vector[i]); it == variables.end()) {
				variables.emplace(op_vector[i], next_var_address);
				if (++i == size) return {};
				try {
					int32_t size_of_arr = std::stoi(op_vector[i]);
					if (size_of_arr > 1024 - static_cast<int64_t>(next_var_address) ||
						size_of_arr < 1) return {};
					next_var_address += size_of_arr;
				}
				catch (...) { return {}; }
			}
			else return {};
		}
		else if (str == "LOAD_INDEX" || str == "STORE_INDEX") {
			if (++i == size) return {};

			bytecode.push_back(static_cast<uint8_t>(OpCode::PUSH));
			if (auto it = variables.find(op_vector[i]); it != variables.end()) {
				PushMemoryArg(bytecode, it->second);
			}
			else return {};
			bytecode.push_back(static_cast<uint8_t>(OpCode::ADD));

			if (str == "LOAD_INDEX") bytecode.push_back(static_cast<uint8_t>(OpCode::LOAD));
			else bytecode.push_back(static_cast<uint8_t>(OpCode::STORE));

			position += 7;
		}
		else if(str == "SWAP") { bytecode.push_back(static_cast<uint8_t>(OpCode::SWAP)); position++; }
		else if (str == "CALL") {
			if (++i == size) return {};
			bytecode.push_back(static_cast<uint8_t>(OpCode::CALL));
			if (auto it = cases.find(op_vector[i]); it != cases.end()) {
				PushCaseArg(bytecode, it, position);
				position += 5;
			}
			else return {};
		}
		else if (str == "RET") { bytecode.push_back(static_cast<uint8_t>(OpCode::RET)); position++; }
		else if (str == "HALT") { bytecode.push_back(static_cast<uint8_t>(OpCode::HALT)); position++; }
		else if (str.back() == ':') continue;
		else return {};
	}

	return bytecode;
}