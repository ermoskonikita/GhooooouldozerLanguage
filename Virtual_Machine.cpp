#include "Virtual_Machine.h"
#include "OpCode.h"
#include <cctype>
#include <unordered_map>
#include <iostream>

int32_t Virtual_Machine::read_int32() {
	if (ip + 4 > bytecode.size()) {
		running = false;
		return 0;
	}

	int32_t arg = 0;
	for (int i = 0; i < 4; i++) {
		arg |= static_cast<int32_t>(bytecode[ip++]) << (8 * i);
	}

	return arg;
}

void Virtual_Machine::pop_two(int32_t& right, int32_t& left) {
	right = stack.back();
	stack.pop_back();
	left = stack.back();
	stack.pop_back();
}

Virtual_Machine::Virtual_Machine() : variables(1024, 0) {}

void Virtual_Machine::run() {
	while (running && ip < bytecode.size()) {
		OpCode code = static_cast<OpCode>(bytecode[ip++]);
		switch (code) {
		case OpCode::PUSH:
		{
			int32_t arg = read_int32();
			if (running) stack.push_back(arg);
			break;
		}
		case OpCode::POP:
			if (stack.empty()) {
				running = false;
				break;
			}

			stack.pop_back();
			break;
		case OpCode::DUP:
			if (stack.empty()) {
				running = false;
				break;
			}

			stack.push_back(stack.back());
			break;
		case OpCode::ADD:
		{
			if (stack.size() < 2) {
				running = false;
				break;
			}

			int32_t right, left;
			pop_two(right, left);

			int64_t sum = static_cast<int64_t>(left) + right;

			if (sum > INT_MAX || sum < INT_MIN) {
				running = false;
				break;
			}

			stack.push_back(static_cast<int32_t>(sum));
			break;
		}
		case OpCode::SUB:
		{
			if (stack.size() < 2) {
				running = false;
				break;
			}

			int32_t right, left;
			pop_two(right, left);

			int64_t sub = static_cast<int64_t>(left) - right;

			if (sub > INT_MAX || sub < INT_MIN) {
				running = false;
				break;
			}

			stack.push_back(static_cast<int32_t>(sub));
			break;
		}
		case OpCode::MUL:
		{
			if (stack.size() < 2) {
				running = false;
				break;
			}

			int32_t right, left;
			pop_two(right, left);

			int64_t mul = static_cast<int64_t>(left) * right;

			if (mul > INT_MAX || mul < INT_MIN) {
				running = false;
				break;
			}

			stack.push_back(static_cast<int32_t>(mul));
			break;
		}
		case OpCode::DIV:
		{
			if (stack.size() < 2) {
				running = false;
				break;
			}
			if (stack.back() == 0) {
				running = false;
				break;
			}

			int32_t right, left;
			pop_two(right, left);

			int64_t div = static_cast<int64_t>(left) / right;

			if (div > INT_MAX || div < INT_MIN) {
				running = false;
				break;
			}

			stack.push_back(static_cast<int32_t>(div));
			break;
		}
		case OpCode::BAND:
		{
			if (stack.size() < 2) {
				running = false;
				break;
			}

			int32_t right, left;
			pop_two(right, left);

			stack.push_back(left & right);
			break;
		}
		case OpCode::BOR:
		{
			if (stack.size() < 2) {
				running = false;
				break;
			}

			int32_t right, left;
			pop_two(right, left);

			stack.push_back(left | right);
			break;
		}
		case OpCode::BXOR:
		{
			if (stack.size() < 2) {
				running = false;
				break;
			}

			int32_t right, left;
			pop_two(right, left);

			stack.push_back(left ^ right);
			break;
		}
		case OpCode::BNOT:
		{
			if (stack.empty()) {
				running = false;
				break;
			}

			int32_t val = stack.back();
			stack.pop_back();

			stack.push_back(~val);
			break;
		}
		case OpCode::LT:
		case OpCode::LE:
		case OpCode::GT:
		case OpCode::GE:
		case OpCode::EQ:
		case OpCode::NE:
		case OpCode::AND:
		case OpCode::OR:
		{
			if (stack.size() < 2) {
				running = false;
				break;
			}

			int32_t right, left;
			pop_two(right, left);

			if (code == OpCode::LT) stack.push_back(left < right);
			else if (code == OpCode::LE) stack.push_back(left <= right);
			else if (code == OpCode::GT) stack.push_back(left > right);
			else if (code == OpCode::GE) stack.push_back(left >= right);
			else if (code == OpCode::EQ) stack.push_back(left == right);
			else if (code == OpCode::NE) stack.push_back(left != right);
			else if (code == OpCode::AND) stack.push_back(left && right);
			else if (code == OpCode::OR) stack.push_back(left || right);
			break;
		}
		case OpCode::NOT:
		{
			if (stack.empty()) {
				running = false;
				break;
			}

			int32_t val = stack.back();
			stack.pop_back();

			stack.push_back(!val);
			break;
		}
		case OpCode::JMP:
		{
			int64_t pointer = ip;
			pointer += read_int32();
			if (!running) break;

			if (pointer >= static_cast<int64_t>(bytecode.size()) || pointer < 0) {
				running = false;
				break;
			}
			ip = pointer;
			break;
		}
		case OpCode::JZ:
		case OpCode::JNZ:
		{
			if (stack.empty()) {
				running = false;
				break;
			}

			int64_t pointer = ip;
			pointer += read_int32();
			if (!running) break;

			int32_t if0 = stack.back();
			stack.pop_back();
			if (if0 && code == OpCode::JZ) break;
			if (!if0 && code == OpCode::JNZ) break;

			if (pointer >= static_cast<int64_t>(bytecode.size()) || pointer < 0) {
				running = false;
				break;
			}

			ip = pointer;
			break;
		}
		case OpCode::PRINT:
			if (stack.empty()) {
				running = false;
				break;
			}

			std::cout << stack.back() << '\n';
			stack.pop_back();
			break;
		case OpCode::PRINT_CHAR:
			if (stack.empty()) {
				running = false;
				break;
			}

			std::cout << static_cast<unsigned char>(stack.back());
			stack.pop_back();
			break;
		case OpCode::LOAD:
		{
			if (stack.empty()) {
				running = false;
				break;
			}

			int32_t adr = stack.back();
			if (adr >= variables.size() || adr < 0) {
				running = false;
				break;
			}
			stack.pop_back();

			stack.push_back(variables[adr]);
			break;
		}
		case OpCode::STORE:
		{
			if (stack.size() < 2) {
				running = false;
				break;
			}

			int32_t adr = stack.back();
			if (adr >= variables.size() || adr < 0) {
				running = false;
				break;
			}
			stack.pop_back();

			variables[adr] = stack.back();
			stack.pop_back();
			break;
		}
		case OpCode::LOAD_INDEX:
		{
			if (stack.size() < 2) {
				running = false;
				break;
			}

			int32_t adr = stack.back();
			if (adr >= variables.size() || adr < 0) {
				running = false;
				break;
			}
			stack.pop_back();

			int32_t len = heap[variables[adr]];
			int32_t index = stack.back();
			stack.pop_back();

			if (index < 0 || index >= len) {
				running = false;
				break;
			}

			stack.push_back(heap[variables[adr] + index + 1]);
			break;
		}
		case OpCode::STORE_INDEX:
		{
			if (stack.size() < 3) {
				running = false;
				break;
			}

			int32_t adr = stack.back();
			if (adr >= variables.size() || adr < 0) {
				running = false;
				break;
			}
			stack.pop_back();

			int32_t len = heap[variables[adr]];
			int32_t index = stack.back();
			stack.pop_back();

			if (index < 0 || index >= len) {
				running = false;
				break;
			}

			heap[variables[adr] + index + 1] = stack.back();
			stack.pop_back();

			break;
		}
		case OpCode::ALLOC:
		{
			if (stack.size() < 2) {
				running = false;
				break;
			}

			int32_t adr = stack.back();
			if (adr >= variables.size() || adr < 0) {
				running = false;
				break;
			}
			stack.pop_back();

			int32_t size = stack.back();
			if (size < 0 || size > 10'000'000) {
				running = false;
				break;
			}
			stack.pop_back();

			size_t s32 = heap.size();
			if (s32 > INT_MAX) {
				running = false;
				break;
			}
			variables[adr] = static_cast<int32_t>(s32);

			try {
				heap.push_back(size);
				heap.resize(heap.size() + size, 0);
			}
			catch (const std::exception& e) {
				std::cerr << "VM Memory Error: " << e.what() << '\n';
				running = false;
				break;
			}

			break;
		}
		case OpCode::CALL:
		{
			int64_t pointer = ip;
			int32_t jmp = read_int32();
			if (!running) break;

			call_stack.push_back(static_cast<int32_t>(ip));
			pointer += jmp;
			if (pointer < 0 || pointer >= static_cast<int64_t>(bytecode.size())) {
				running = false;
				break;
			}
			ip = pointer;
			break;
		}
		case OpCode::RET:
			if (call_stack.empty()) {
				running = false;
				break;
			}
			ip = call_stack.back();
			call_stack.pop_back();
			break;
		case OpCode::SWAP:
			if (stack.size() < 2) {
				running = false;
				break;
			}
			std::swap(stack.back(), *(stack.end() - 2));
			break;
		case OpCode::HALT:
			running = false;
			break;
		default:
			running = false;
			break;
		}
	}
}

void Virtual_Machine::reset() {
	ip = 0;
	stack.clear();
	running = true;
}

void Virtual_Machine::SetBytecode(const std::vector<uint8_t>& bc) {
	reset();
	bytecode = bc;
}

void Virtual_Machine::SetBytecode(std::vector<uint8_t>&& bc) {
	reset();
	bytecode = std::move(bc);
}