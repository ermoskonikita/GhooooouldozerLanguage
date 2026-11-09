#pragma once
#include <string>

enum class TokenType {
	Number,
	Plus,Minus,Star,Slash,
	Assign,
	Less, Greater, Equal,
	LessEqual, GreaterEqual, NotEqual,
	And,Or,Not,
	BAnd,BOr,BXor,BNot,
	LParen,RParen,
	LBrace,RBrace,
	Semicolon,
	Print,
	Func,
	Return,
	If,
	Else,
	While,
	Identifier,
	End
};

struct Token {
	TokenType type;
	std::string text;
};