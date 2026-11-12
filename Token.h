#pragma once
#include <string>

enum class TokenType {
	Number,
	Plus,Minus,Star,Slash,
	PlusPlus,MinusMinus,
	Assign,PlusAssign,
	Less, Greater, Equal,
	LessEqual, GreaterEqual, NotEqual,
	And,Or,Not,
	BAnd,BOr,BXor,BNot,
	LParen,RParen,
	LBracket,RBracket,
	LBrace,RBrace,
	Semicolon,
	Print,
	Func,
	Return,
	If,
	Else,
	While,
	Identifier,
	String,
	End
};

struct Token {
	TokenType type;
	std::string text;
};