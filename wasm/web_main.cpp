#include <iostream>
#include "Parser.h"
#include "Interpreter.h"
#include <sstream>
#include <string>

class SSRedirect
{
private:
	std::stringstream _ss;
	std::streambuf* _outbuf;
public:
	SSRedirect() :
		_ss{},
		_outbuf{nullptr}
	{
		_outbuf = std::cout.rdbuf(_ss.rdbuf());
	}
	~SSRedirect()
	{
		std::cout.rdbuf(_outbuf);
	}
	std::stringstream& getSS()
	{
		return _ss;
	}
};

std::string str;

extern "C" const char* eval(char* input)
{
	char* output;

	SrcScanner scanner{ input };
	Parser parser;
	AST::AST* ast = nullptr;
	ast = parser.Parse(std::move(scanner));
	Parser::PrintAST(ast);

	{
		Interpreter interpreter;
		SSRedirect ss_redirect{};
		interpreter.Eval(ast);

		if (ast)
		{
			delete ast;
		}

		str = ss_redirect.getSS().str().c_str(); //防止cow
	}

	return str.c_str();
}