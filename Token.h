#pragma once
#include <string>

enum class TokenType {
	Number,
	Plus,Minus,Star,Slash,Percent,
	PlusPlus, MinusMinus,
	Assign, PlusAssign, MinusAssign, StarAssign, SlashAssign, PercentAssign,
	BAndAssign, BOrAssign, BXorAssign,
	Less, Greater, Equal,
	LessEqual, GreaterEqual, NotEqual,
	And,Or,Not,
	BAnd,BOr,BXor,BNot,
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
	Identifier,
	Array,
	String,
	End
};

struct Token {
	TokenType type;
	std::string text;
};