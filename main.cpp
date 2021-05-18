#include <iostream>
#include "Parser.h"
#include "Interpreter.h"
#include <sstream>
#include <fstream>

//÷ÿ∂®œÚ
class Redirect
{
private:
	std::ofstream _file;
	std::streambuf* _outbuf;
public:
	Redirect(const char* path) :
		_file(path),
		_outbuf(nullptr)
	{
		_outbuf = std::cout.rdbuf(_file.rdbuf());
	}
	~Redirect()
	{
		std::cout.rdbuf(_outbuf);
	}
};

int main(int argc, char** argv)
{
	const char* path;

	if (argc == 1)
	{
		path = "./test.txt";
	}
	else
	{
		path = argv[1];
	}

	FileScanner scanner{ path };
	Parser parser;
	AST::AST* ast = nullptr;
	{
		Redirect rd("./token.txt");
		ast = parser.Parse(std::move(scanner));
	}

	{
		Redirect rd("./ast.txt");
		Parser::PrintAST(ast);
	}

	Interpreter interpreter;
	interpreter.Eval(ast);
	if (ast) {
		delete ast;
	}
}
