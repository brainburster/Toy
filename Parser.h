#pragma once
#include "AST.h"
#include "Token.h"
#include <vector>

class Parser
{
public:
	Parser() :_cur(0) {}
	AST::AST* Parse(const char*);
	void Reserve(int n) { _buffer.reserve(n); }
	static void PrintAST(AST::AST* ast, int n = 0);
private:
	std::vector<Token::Token> _buffer;
	int _cur;
	const Token::Token& Peek(int n = 0);
	int Seek(int offset = 1);
	void Scan(const char*);

	//������Ŀ
	AST::AST* Stats();
	AST::AST* Stat();
	AST::AST* EchoStat();
	AST::AST* Expr();
	AST::AST* TermExpr();
	AST::AST* FactorExpr();
	AST::AST* PrimExpr();
	AST::AST* Assignment();
	AST::AST* Num();
	AST::AST* STR();
	AST::AST* ID();
	AST::AST* Value();
	AST::AST* Block();
	AST::AST* Args();
	AST::AST* Params();
	AST::AST* FuncDef();
	AST::AST* FunCall();

	//ƥ��token
	template<typename... Args>
	auto Match(Args... args) ->typename std::enable_if<(std::is_integral_v<Args> &&...), bool>::type
	{
		int i = 0;
		bool status = (_Match(args, i++) &&...);
		status ? Seek(i) : 0;
		return status;
	}
	bool _Match(int type, int offset)
	{
		if (type == Peek(offset).type)
		{
			return true;
		}
		return false;
	}
};
