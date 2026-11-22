#pragma once
#include <cstdint>

enum class OpCode : uint8_t {
	// Stack manipulation
	PUSH = 0x01,
	POP = 0x02,
	DUP = 0x03,
	SWAP = 0x04,
	SWAP3 = 0x05,

	// Arithmetic
	ADD = 0x06,
	SUB = 0x07,
	MUL = 0x08,
	DIV = 0x09,
	MOD = 0x0A,
	NEG = 0x0B,

	// Comparison & Logic
	LT = 0x0C,
	LE = 0x0D,
	GT = 0x0E,
	GE = 0x0F,
	EQ = 0x10,
	NE = 0x11,
	AND = 0x12,
	OR = 0x13,
	NOT = 0x14,

	// Bitwise
	BAND = 0x15,
	BOR = 0x16,
	BXOR = 0x17,
	BNOT = 0x18,
	SHL = 0x19,
	SHR = 0x1A,

	// Control flow & Functions
	JMP = 0x1B,
	JZ = 0x1C,
	JNZ = 0x1D,
	CALL = 0x1E,
	RET = 0x1F,

	// Global memory & Heap
	LOAD = 0x20,
	STORE = 0x21,
	LOAD_INDEX = 0x22,
	STORE_INDEX = 0x23,
	ALLOC = 0x24,

	// Local frame memory
	ENTER = 0x25,
	LOAD_LOCAL = 0x26,
	STORE_LOCAL = 0x27,
	LOAD_LOCAL_INDEX = 0x28,
	STORE_LOCAL_INDEX = 0x29,
	ALLOC_LOCAL = 0x2A,

	// I/O & Arrays
	PRINT = 0x2B,
	PRINT_CHAR = 0x2C,
	INPUT = 0x2D,
	LEN = 0x2E,

	// System
	HALT = 0xFF
};