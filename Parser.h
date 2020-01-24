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

	AST::AST* Stats();
	AST::AST* Stat();
	AST::AST* Echo();
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
	AST::AST* BoolExpr();
	AST::AST* BoolTerm();
	AST::AST* BoolFactor();
	AST::AST* BoolPrim();
	AST::AST* If();
	AST::AST* ElseIfList();
	AST::AST* ElseIf();
	AST::AST* Else();

	//∆•≈‰token
	template<typename... Args>
	auto Match(Args... args) ->typename std::enable_if<(std::is_integral_v<Args> &&...), int>::type
	{
		int i = 0;
		bool status = (_Match(args, i++) &&...);
		i = status ? (Seek(i), i) : 0;
		return i;
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
