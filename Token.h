#pragma once
#include <string>

enum class TokenType {
	Number,
	Plus,Minus,Star,Slash,Percent,
	PlusPlus, MinusMinus,
	Assign, PlusAssign, MinusAssign, StarAssign, SlashAssign, PercentAssign, ShlAssign, ShrAssign,
	BAndAssign, BOrAssign, BXorAssign,
	Less, Greater, Equal,
	LessEqual, GreaterEqual, NotEqual,
	And,Or,Not,
	BAnd,BOr,BXor,BNot,
	Shl,Shr,
	LParen,RParen,
	LBracket,RBracket,
	LBrace,RBrace,
	Semicolon,
	Comma,
	Print,
	Input,
	Length,
	Func,
	Return,
	If,
	Else,
	While,
	For,
	Break,Continue,
	Identifier,
	Array,
	String,
	End
};

struct Token {
	TokenType type;
	std::string text;
};