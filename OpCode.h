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
	NEG = 0x0A,

	// Comparison & Logic
	LT = 0x0B,
	LE = 0x0C,
	GT = 0x0D,
	GE = 0x0E,
	EQ = 0x0F,
	NE = 0x10,
	AND = 0x11,
	OR = 0x12,
	NOT = 0x13,

	// Bitwise
	BAND = 0x14,
	BOR = 0x15,
	BXOR = 0x16,
	BNOT = 0x17,

	// Control flow & Functions
	JMP = 0x18,
	JZ = 0x19,
	JNZ = 0x1A,
	CALL = 0x1B,
	RET = 0x1C,

	// Global memory & Heap
	LOAD = 0x1D,
	STORE = 0x1E,
	LOAD_INDEX = 0x1F,
	STORE_INDEX = 0x20,
	ALLOC = 0x21,

	// Local frame memory
	ENTER = 0x22,
	LOAD_LOCAL = 0x23,
	STORE_LOCAL = 0x24,
	LOAD_LOCAL_INDEX = 0x25,
	STORE_LOCAL_INDEX = 0x26,
	ALLOC_LOCAL = 0x27,

	// I/O
	PRINT = 0x28,
	PRINT_CHAR = 0x29,
	INPUT = 0x2A,

	// System
	HALT = 0xFF
};