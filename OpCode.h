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

	// Control flow & Functions
	JMP = 0x19,
	JZ = 0x1A,
	JNZ = 0x1B,
	CALL = 0x1C,
	RET = 0x1D,

	// Global memory & Heap
	LOAD = 0x1E,
	STORE = 0x1F,
	LOAD_INDEX = 0x20,
	STORE_INDEX = 0x21,
	ALLOC = 0x22,

	// Local frame memory
	ENTER = 0x23,
	LOAD_LOCAL = 0x24,
	STORE_LOCAL = 0x25,
	LOAD_LOCAL_INDEX = 0x26,
	STORE_LOCAL_INDEX = 0x27,
	ALLOC_LOCAL = 0x28,

	// I/O
	PRINT = 0x29,
	PRINT_CHAR = 0x2A,
	INPUT = 0x2B,

	// System
	HALT = 0xFF
};