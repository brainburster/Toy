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

class A {
public:
	virtual ~A() {
		std::cout << "A" << std::endl;
	}
};
class B :public A {
public:
	~B() {
		std::cout << "B" << std::endl;
	}
};
int main()
{
	Parser parser;
	Interpreter interpreter;
	AST::AST* ast;
	{
		Redirect rd("./token.txt");
		ast = parser.Parse("./test.txt");
	}

	{
		Redirect rd("./ast.txt");
		Parser::PrintAST(ast);
	}
	interpreter.Eval(ast);
	delete ast;
}