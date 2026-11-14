#include "Lexer.h"
#include <fstream>
#include <stdexcept>
#include <sstream>

Lexer::Lexer(std::string str) : src(std::move(str)),pos(0) {
	clearComments();

	keywords.emplace("print",TokenType::Print);
	keywords.emplace("func",TokenType::Func);
	keywords.emplace("return", TokenType::Return);
	keywords.emplace("if", TokenType::If);
	keywords.emplace("else", TokenType::Else);
	keywords.emplace("while", TokenType::While);
	keywords.emplace("array", TokenType::Array);
}

void Lexer::clearComments() {
	size_t left = 0;
	size_t right = 0;
	while ((right = src.find("//")) != std::string::npos) {
		left = right;
		while (right < src.size() && src[right] != '\n') right++;
		src.erase(src.begin() + left, src.begin() + right);
	}
}

Token Lexer::GetNextToken() {
	Token t;

	while (pos < src.size() && std::isspace(static_cast<unsigned char>(src[pos]))) pos++;
	if (pos == src.size()) {
		t.type = TokenType::End;
		return t;
	}

	if (std::isdigit(static_cast<unsigned char>(src[pos]))) {
		size_t start_pos = pos;
		while (pos < src.size() && std::isdigit(static_cast<unsigned char>(src[pos]))) pos++;

		t.text = src.substr(start_pos, pos - start_pos);
		t.type = TokenType::Number;
	}
	else if (std::isalpha(static_cast<unsigned char>(src[pos])) || src[pos] == '_') {
		size_t start_pos = pos;
		while (pos < src.size() && (std::isalnum(static_cast<unsigned char>(src[pos])) ||
			src[pos] == '_')) pos++;
		t.text = src.substr(start_pos, pos - start_pos);

		if (auto it = keywords.find(t.text);it != keywords.end()) t.type = it->second;
		else t.type = TokenType::Identifier;
	}
	else if (src[pos] == '"') {
		pos++;
		size_t start_pos = pos;
		while (pos < src.size() && src[pos] != '"') {
			if (src[pos] == '\\' && pos + 1 < src.size() 
				&& (src[pos + 1] == 'n' || src[pos + 1] == 't')) {

				char c = src[pos + 1];
				t.text += src.substr(start_pos, pos - start_pos);

				if (c == 'n') t.text.push_back('\n');
				else t.text.push_back('\t');

				pos += 2;
				start_pos = pos;
			}
			else pos++;
		}

		t.text += src.substr(start_pos, pos - start_pos);

		if (pos == src.size()) throw std::runtime_error(std::string("Отсутствуют кавычки!"));
		else pos++;

		t.type = TokenType::String;
	}
	else {
		char current = src[pos++];
		t.text.push_back(current);
		switch (current) {
		case '+': {
			if (pos < src.size() && src[pos] == '+') {
				pos++;
				t.text.push_back('+');
				t.type = TokenType::PlusPlus;
			}
			else if (pos < src.size() && src[pos] == '=') {
				pos++;
				t.text.push_back('=');
				t.type = TokenType::PlusAssign;
			}
			else {
				t.type = TokenType::Plus;
			}
			break;
		}
		case '-': {
			if (pos < src.size() && src[pos] == '-') {
				pos++;
				t.text.push_back('-');
				t.type = TokenType::MinusMinus;
			}
			else if (pos < src.size() && src[pos] == '=') {
				pos++;
				t.text.push_back('=');
				t.type = TokenType::MinusAssign;
			}
			else t.type = TokenType::Minus;
			break;
		}
		case '*': {
			if (pos < src.size() && src[pos] == '=') {
				pos++;
				t.text.push_back('=');
				t.type = TokenType::StarAssign;
			}
			else t.type = TokenType::Star;
			break;
		}
		case '/': {
			if (pos < src.size() && src[pos] == '=') {
				pos++;
				t.text.push_back('=');
				t.type = TokenType::SlashAssign;
			}
			else t.type = TokenType::Slash;
			break;
		}
		case '=': {
			if (pos < src.size() && src[pos] == '=') {
				pos++;
				t.text.push_back('=');
				t.type = TokenType::Equal;
			}
			else t.type = TokenType::Assign;
			break;
		}
		case '<': {
			if (pos < src.size() && src[pos] == '=') {
				pos++;
				t.text.push_back('=');
				t.type = TokenType::LessEqual;
			}
			else t.type = TokenType::Less;
			break;
		}
		case '>':
		{
			if (pos < src.size() && src[pos] == '=') {
				pos++;
				t.text.push_back('=');
				t.type = TokenType::GreaterEqual;
			}
			else t.type = TokenType::Greater;
			break;
		}
		case '&': {
			if (pos < src.size() && src[pos] == '&') {
				pos++;
				t.text.push_back('&');
				t.type = TokenType::And;
			}
			else if (pos < src.size() && src[pos] == '=') {
				pos++;
				t.text.push_back('=');
				t.type = TokenType::BAndAssign;
			}
			else t.type = TokenType::BAnd;
			break;
		}
		case '|': {
			if (pos < src.size() && this->src[pos] == '|') {
				pos++;
				t.type = TokenType::Or;
				t.text += '|';
			}
			else if (pos < src.size() && src[pos] == '=') {
				pos++;
				t.text.push_back('=');
				t.type = TokenType::BOrAssign;
			}
			else t.type = TokenType::BOr;
			break;
		}
		case '!': {
			if (pos < src.size() && this->src[pos] == '=') {
				pos++;
				t.text.push_back('=');
				t.type = TokenType::NotEqual;
			}
			else t.type = TokenType::Not;
			break;
		}
		case '^': {
			if (pos < src.size() && src[pos] == '=') {
				pos++;
				t.text.push_back('=');
				t.type = TokenType::BXorAssign;
			}
			else t.type = TokenType::BXor;
			break;
		}
		case '~':t.type = TokenType::BNot; break;
		case '(':t.type = TokenType::LParen; break;
		case ')':t.type = TokenType::RParen; break;
		case '[':t.type = TokenType::LBracket; break;
		case ']':t.type = TokenType::RBracket; break;
		case '{':t.type = TokenType::LBrace; break;
		case '}':t.type = TokenType::RBrace; break;
		case ';':t.type = TokenType::Semicolon; break;
		default: throw std::runtime_error(std::string("Неизвестный символ: ") + current); break;
		}
	}
	
	return t;
}

std::vector<Token> Lexer::tokenize() {
	std::vector<Token> token_vec;
	while (pos < src.size()) {
		token_vec.push_back(GetNextToken());
	}
	if (token_vec.empty() || token_vec.back().type != TokenType::End) {
		Token token;
		token.type = TokenType::End;
		token_vec.push_back(std::move(token));
	}
	return token_vec;
}