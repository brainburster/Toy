#include <iostream>
#include "Parser.h"
#include "Interpreter.h"
#include <sstream>
#include <fstream>

//ÖØ¶¨Ïò
class Redirect
{
private:
	std::ofstream* _file;
	std::streambuf* _outbuf;
public:
	Redirect(const char* path) :
		_file(nullptr),
		_outbuf(nullptr)
	{
		_file = new std::ofstream(path);
		_outbuf = std::cout.rdbuf(_file->rdbuf());
	}
	~Redirect()
	{
		std::cout.rdbuf(_outbuf);
		delete _file;
	}
};

int main()
{
	std::cout << AST::Length<int, int, int>::value << std::endl;

	Parser parser;
	Interpreter interpreter;
	AST::AST* ast;

	{
		Redirect rd("./token.txt");
		ast = parser.Parse("./test.toy");
	}

	{
		Redirect rd("./ast.txt");
		Parser::PrintAST(ast);
	}

	interpreter.Eval(ast);
	delete ast;
}