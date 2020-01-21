#pragma once
#include "AST.h"
#include <unordered_map>
#include <vector>
#include <optional>

class Env
{
public:
	struct Value
	{
		int type;
		union Type
		{
			double dValue;
			int iValue;
			Type(int i) :iValue(i) {}
			Type(double i) :dValue(i) {}
		} value;
		Value() :type(0), value(0) {};
		Value(const Value&) = default;
		Value(int t, int v) :type(t), value(v) {}
		Value(int t, double v) :type(t), value(v) {}
	};
public:
	void push(int id, double value);
	void push(int id, int value);
	void push(int value);
	void push(double value);
	int find(int id);
	std::optional<Env::Value> pop();
	std::optional<Env::Value> get(int id);
private:
	std::unordered_map<int, int> _map;  //√˚≥∆µΩ’ªµÿ÷∑µƒ”≥…‰
	std::vector<Env::Value> _stack; //–Èƒ‚’ª
};

class Interpreter
{
public:
	bool Eval(AST::AST* ast);
private:
	Env _env;
	bool EvalStats(AST::AST* stats);
	bool EvalEcho(AST::Stat<'echo'>* echo);
	double EvalExpr(AST::Expr* expr);
	const char* EvalStr(AST::StrValue* str);
	template<typename T>
	void Print(T value);
};
