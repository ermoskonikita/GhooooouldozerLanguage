#pragma once
#include <cstdint>
#include <vector>
#include <string>

class Virtual_Machine {
private:
	std::vector<uint8_t> bytecode;
	std::vector<int32_t> stack;
	std::vector<int32_t> call_stack;
	std::vector<int32_t> variables;
	std::vector<int32_t> heap;

	size_t ip = 0;
	bool running = true;

	int32_t read_int32();
	void pop_two(int32_t& right, int32_t& left);
public:
	Virtual_Machine();
	void run();
	void reset();
	void SetBytecode(const std::vector<uint8_t>& bc);
	void SetBytecode(std::vector<uint8_t>&& bc);	
};