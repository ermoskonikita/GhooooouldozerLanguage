#include "Virtual_Machine.h"
#include "OpCode.h"
#include <cctype>
#include <unordered_map>
#include <iostream>
#include <stdexcept>

int32_t Virtual_Machine::read_int32() {
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

Virtual_Machine::Virtual_Machine(std::vector<uint8_t> bc) : bytecode(std::move(bc)), variables(1024, 0) {}

void Virtual_Machine::run() {
	while (running && ip < bytecode.size()) {
		OpCode code = static_cast<OpCode>(bytecode[ip++]);
		switch (code) {
		case OpCode::PUSH:
		{
			if (ip + 4 > bytecode.size()) {
				throw std::runtime_error("Unexpected end of bytecode while reading operand!");
			}

			int32_t arg = read_int32();
			stack.push_back(arg);
			break;
		}
		case OpCode::POP:
			if (stack.empty()) {
				throw std::runtime_error("Stack underflow!");
			}

			stack.pop_back();
			break;
		case OpCode::DUP:
			if (stack.empty()) {
				throw std::runtime_error("Stack underflow!");
			}

			stack.push_back(stack.back());
			break;
		case OpCode::ADD:
		{
			if (stack.size() < 2) {
				throw std::runtime_error("Stack underflow on ADD!");
			}

			int32_t right, left;
			pop_two(right, left);

			int64_t sum = static_cast<int64_t>(left) + right;

			if (sum > INT32_MAX || sum < INT32_MIN) {
				throw std::runtime_error("Overflow on ADD!");
			}

			stack.push_back(static_cast<int32_t>(sum));
			break;
		}
		case OpCode::SUB:
		{
			if (stack.size() < 2) {
				throw std::runtime_error("Stack underflow on SUB!");
			}

			int32_t right, left;
			pop_two(right, left);

			int64_t sub = static_cast<int64_t>(left) - right;

			if (sub > INT32_MAX || sub < INT32_MIN) {
				throw std::runtime_error("Overflow on SUB!");
			}

			stack.push_back(static_cast<int32_t>(sub));
			break;
		}
		case OpCode::MUL:
		{
			if (stack.size() < 2) {
				throw std::runtime_error("Stack underflow on MUL!");
			}

			int32_t right, left;
			pop_two(right, left);

			int64_t mul = static_cast<int64_t>(left) * right;

			if (mul > INT32_MAX || mul < INT32_MIN) {
				throw std::runtime_error("Overflow on MUL!");
			}

			stack.push_back(static_cast<int32_t>(mul));
			break;
		}
		case OpCode::DIV:
		{
			if (stack.size() < 2) {
				throw std::runtime_error("Stack underflow on DIV!");
			}
			if (stack.back() == 0) {
				throw std::runtime_error("Division by zero!");
			}

			int32_t right, left;
			pop_two(right, left);

			int64_t div = static_cast<int64_t>(left) / right;

			if (div > INT32_MAX || div < INT32_MIN) {
				throw std::runtime_error("Overflow on DIV!");
			}

			stack.push_back(static_cast<int32_t>(div));
			break;
		}
		case OpCode::BAND:
		{
			if (stack.size() < 2) {
				throw std::runtime_error("Stack underflow on BAND!");
			}

			int32_t right, left;
			pop_two(right, left);

			stack.push_back(left & right);
			break;
		}
		case OpCode::BOR:
		{
			if (stack.size() < 2) {
				throw std::runtime_error("Stack underflow on BOR!");
			}

			int32_t right, left;
			pop_two(right, left);

			stack.push_back(left | right);
			break;
		}
		case OpCode::BXOR:
		{
			if (stack.size() < 2) {
				throw std::runtime_error("Stack underflow on BXOR!");
			}

			int32_t right, left;
			pop_two(right, left);

			stack.push_back(left ^ right);
			break;
		}
		case OpCode::BNOT:
		{
			if (stack.empty()) {
				throw std::runtime_error("Stack underflow on BNOT!");
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
				throw std::runtime_error("Stack underflow on comparison or logical operation!");
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
				throw std::runtime_error("Stack underflow on NOT!");
			}

			int32_t val = stack.back();
			stack.pop_back();

			stack.push_back(!val);
			break;
		}
		case OpCode::JMP:
		{
			if (ip + 4 > bytecode.size()) {
				throw std::runtime_error("Unexpected end of bytecode while reading operand!");
			}

			int64_t pointer = ip;
			pointer += read_int32();

			if (pointer >= static_cast<int64_t>(bytecode.size()) || pointer < 0) {
				throw std::runtime_error("Jump target out of bytecode bounds!");
			}

			ip = pointer;
			break;
		}
		case OpCode::JZ:
		case OpCode::JNZ:
		{
			if (stack.empty()) {
				throw std::runtime_error("Stack underflow!");
			}

			if (ip + 4 > bytecode.size()) {
				throw std::runtime_error("Unexpected end of bytecode while reading operand!");
			}

			int64_t pointer = ip;
			pointer += read_int32();

			int32_t if0 = stack.back();
			stack.pop_back();
			if (if0 && code == OpCode::JZ) break;
			if (!if0 && code == OpCode::JNZ) break;

			if (pointer >= static_cast<int64_t>(bytecode.size()) || pointer < 0) {
				throw std::runtime_error("Jump target out of bytecode bounds!");
			}

			ip = pointer;
			break;
		}
		case OpCode::PRINT:
		case OpCode::PRINT_CHAR:
			if (stack.empty()) {
				throw std::runtime_error("Stack underflow on PRINT!");
			}

			if(code == OpCode::PRINT) std::cout << stack.back();
			else std::cout << static_cast<unsigned char>(stack.back());

			stack.pop_back();
			break;
		case OpCode::LOAD:
		{
			if (stack.empty()) {
				throw std::runtime_error("Stack underflow on LOAD!");
			}

			int32_t adr = stack.back();
			if (adr < 0 || static_cast<size_t>(adr) >= variables.size()) {
				throw std::runtime_error("Variable address out of bounds on LOAD!");
			}
			stack.pop_back();

			stack.push_back(variables[adr]);
			break;
		}
		case OpCode::STORE:
		{
			if (stack.size() < 2) {
				throw std::runtime_error("Stack underflow on STORE!");
			}

			int32_t adr = stack.back();
			if (adr < 0 || static_cast<size_t>(adr) >= variables.size()) {
				throw std::runtime_error("Variable address out of bounds on STORE!");
			}
			stack.pop_back();

			variables[adr] = stack.back();
			stack.pop_back();
			break;
		}
		case OpCode::LOAD_INDEX:
		{
			if (stack.size() < 2) {
				throw std::runtime_error("Stack underflow on LOAD_INDEX!");
			}

			int32_t adr = stack.back();
			if (adr < 0 || static_cast<size_t>(adr) >= variables.size()) {
				throw std::runtime_error("Variable address out of bounds on LOAD_INDEX!");
			}
			stack.pop_back();

			int32_t index = stack.back();
			stack.pop_back();

			int32_t heap_ptr = variables[adr];
			if (heap_ptr < 0 || static_cast<size_t>(heap_ptr) >= heap.size()) {
				throw std::runtime_error("Invalid array pointer on LOAD_INDEX!");
			}

			int32_t len = heap[heap_ptr];
			if (index < 0 || index >= len) {
				throw std::runtime_error("Array index out of bounds on LOAD_INDEX!");
			}

			stack.push_back(heap[static_cast<size_t>(heap_ptr) + index + 1]);
			break;
		}
		case OpCode::STORE_INDEX:
		{
			if (stack.size() < 3) {
				throw std::runtime_error("Stack underflow on STORE_INDEX!");
			}

			int32_t adr = stack.back();
			if (adr < 0 || static_cast<size_t>(adr) >= variables.size()) {
				throw std::runtime_error("Variable address out of bounds on STORE_INDEX!");
			}
			stack.pop_back();

			int32_t index = stack.back();
			stack.pop_back();

			int32_t heap_ptr = variables[adr];
			if (heap_ptr < 0 || static_cast<size_t>(heap_ptr) >= heap.size()) {
				throw std::runtime_error("Invalid array pointer on STORE_INDEX!");
			}

			int32_t len = heap[heap_ptr];
			if (index < 0 || index >= len) {
				throw std::runtime_error("Array index out of bounds on STORE_INDEX!");
			}

			heap[static_cast<size_t>(heap_ptr) + index + 1] = stack.back();
			stack.pop_back();

			break;
		}
		case OpCode::ALLOC:
		{
			if (stack.size() < 2) {
				throw std::runtime_error("Stack underflow on ALLOC!");
			}

			int32_t adr = stack.back();
			if (adr < 0 || static_cast<size_t>(adr) >= variables.size()) {
				throw std::runtime_error("Variable address out of bounds on ALLOC!");
			}
			stack.pop_back();

			int32_t size = stack.back();
			if (size < 0 || size > 10'000'000) {
				throw std::runtime_error("Invalid array size on ALLOC!");
			}
			stack.pop_back();

			size_t s32 = heap.size();
			if (s32 > INT32_MAX) {
				throw std::runtime_error("Heap overflow on ALLOC!");
			}

			try {
				heap.push_back(size);
				heap.resize(heap.size() + size, 0);
			}
			catch (const std::exception&) {
				throw std::runtime_error("VM Memory error on ALLOC!");
			}

			variables[adr] = static_cast<int32_t>(s32);

			break;
		}
		case OpCode::CALL:
		{
			if (ip + 4 > bytecode.size()) {
				throw std::runtime_error("Unexpected end of bytecode while reading operand!");
			}

			int64_t pointer = ip;
			int32_t jmp = read_int32();

			call_stack.push_back(static_cast<int32_t>(ip));
			if (call_stack.size() >= 10000) throw std::runtime_error("Call stack overflow!");

			pointer += jmp;
			if (pointer < 0 || pointer >= static_cast<int64_t>(bytecode.size())) {
				throw std::runtime_error("Jump target out of bytecode bounds!");
			}
			ip = pointer;

			break;
		}
		case OpCode::RET:
			if (call_stack.empty()) {
				throw std::runtime_error("Call stack underflow on RET!");
			}

			ip = call_stack.back();
			call_stack.pop_back();
			break;
		case OpCode::SWAP:
			if (stack.size() < 2) {
				throw std::runtime_error("Stack underflow on SWAP!");
			}
			std::swap(stack.back(), *(stack.end() - 2));
			break;
		case OpCode::SWAP3:
			if (stack.size() < 3) {
				throw std::runtime_error("Stack underflow on SWAP3!");
			}

			std::swap(stack.back(), *(stack.end() - 3));
			break;
		case OpCode::HALT:
			running = false;
			break;
		default:
			throw std::runtime_error("Unknown instruction!");
		}
	}
}