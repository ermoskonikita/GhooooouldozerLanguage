#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include <cstdint>
#include "OpCode.h"

class Assembler {
private:
	std::string op_str;
	std::vector<std::string> op_vector;

	void PushCaseArg(std::vector<uint8_t>& bytecode, std::unordered_map<std::string, int32_t>::iterator it, int32_t position);
	void PushArg(std::vector<uint8_t>& bytecode, size_t& i);
public:
	Assembler(const std::string& str);
	Assembler(const std::vector<std::string>& str_vec);
	std::vector<uint8_t> Compile();
};