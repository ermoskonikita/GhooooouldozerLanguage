#pragma once
#include <cstdint>
#include <vector>
#include <string>

struct CallFrame {
	size_t return_ip;
	size_t prev_fp;
};

class Virtual_Machine {
private:
	std::vector<uint8_t> bytecode;

	std::vector<int32_t> stack;
	std::vector<CallFrame> call_stack;

	std::vector<int32_t> variables;
	std::vector<int32_t> locals_stack;
	std::vector<int32_t> heap;

	size_t ip = 0;
	size_t fp = 0;
	bool running = true;

	int32_t read_int32();
	void pop_two(int32_t& right, int32_t& left);
	int32_t& get_var_slot(int32_t adr, bool is_local);
public:
	Virtual_Machine(std::vector<uint8_t> bc);
	void run();
};