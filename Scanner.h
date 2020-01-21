#pragma once
#include <fstream>
#include <sstream>
#include "Token.h"

class Scanner
{
public:
	Token::Token GetToken();
	static std::string Token2String(const Token::Token& token);
	Scanner(const char* filePath);
private:
	std::stringstream _buffer;
	int _cur;
	int _last;
	void Read();
	void IgnoreBlank();
	void IgnoreComment();
	std::string GetIdentifierOrKeyword();
	std::string GetStringLiteral(int quote);
	double GetNumber();
	int GetOperator();
	int GetKeyWord(const std::string& str);
};
