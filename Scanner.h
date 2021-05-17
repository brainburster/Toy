#pragma once
#include <fstream>
#include <sstream>
#include "Token.h"

class IScanner
{
public:
	Token::Token GetToken();
	static std::string Token2String(const Token::Token& token);
	IScanner();
protected:
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
	IScanner(const IScanner& other) = delete;
	IScanner& operator=(const IScanner& other) = delete;
	IScanner(IScanner&& other) = delete;
	IScanner& operator=(IScanner&& other) = delete;
};

class FileScanner : public IScanner
{
public:
	FileScanner(const char* filePath);
};

class SrcScanner : public IScanner
{
public:
	SrcScanner(const std::string& src);
};
