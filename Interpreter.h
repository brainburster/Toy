#pragma once
#include "AST.h"
//#include <unordered_map>
#include <map>
#include <vector>
#include <stack>
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
			bool bValue;
			AST::AST* astValue;
			Type() :astValue(nullptr) {};
			Type(int i) :iValue(i) {}
			Type(bool b) :bValue(b) {}
			Type(double d) :dValue(d) {}
			Type(AST::AST* p) :astValue(p) {}
		} value;
		Value() :type(0), value(0) {};
		Value(const Value&) = default;
		template<typename T>
		Value(int t, T v) :type(t), value(v) {}
	};
	friend class Interpreter;
private:
	Env() { _variable.reserve(1024); }
	template<typename T>
	void _push(int id, T value);
	template<typename T>
	void _push(T value);
	template<typename T>
	void push(int id, T value);
	template<typename T>
	void push(int id, int type, T value);
	template<typename T>
	void push(T value);
	void clearStack();
	int find(int id);
	std::optional<Env::Value> pop();
	std::optional<Env::Value> top();
	std::optional<Env::Value> get(int id);
	std::optional<Env::Value> at(int location);
	std::map<int, int> _map;
	std::vector<Env::Value> _variable;
	std::stack<Env::Value> _stack;
};

class Interpreter
{
public:
	bool Eval(AST::AST* ast);
	Interpreter() : _global(new Env{}), _curEnv(_global) {}
	~Interpreter() { SafeDelete(_global); }
private:
	Env* _global;
	Env* _curEnv;
	bool EvalFuncDef(AST::FuncDef* funcdef);
	bool EvalFunCall(AST::FunCall* funcall);
	bool EvalIf(AST::IF* ifstat);
	bool EvalElseIfList(AST::AST* elseIfList);
	bool EvalLoop(AST::Loop* loop);
	bool EvalRet(AST::Ret* ret);
	int EvalArray(AST::Array* arr);
	bool EvalAt(AST::At* at);
	bool EvalStats(AST::AST* stats);
	bool EvalAssignment(int id, AST::AST* value);
	bool EvalEcho(AST::Echo* echo);
	bool EvalExpr(AST::Expr* expr);
	bool EvalNeg(AST::Negative* neg);
	bool EvalID(AST::ID* id);
	bool CheckCondition(AST::Expr* condition);
	const char* EvalStr(AST::StrValue* str);
	std::optional<Env::Value> getVar(int id);
	template<typename T>
	void Print(T value);
};

#pragma region PUSH
template<typename T>
inline void  Env::_push(int id, T value)
{
	_variable[id] = { 'null',value };
}
template<>
inline void  Env::_push(int id, Env::Value value)
{
	_variable[id] = value;
}
template<>
inline void  Env::_push(int id, int value)
{
	_variable[id] = { 'str',value };
}
template<>
inline void  Env::_push(int id, double value)
{
	_variable[id] = { 'num',value };
}
template<>
inline void  Env::_push(int id, bool value)
{
	_variable[id] = { 'bool',value };
}
template<>
inline void  Env::_push(int id, AST::AST* value)
{
	_variable[id] = { 'ast',value };
}

template<typename T>
inline void Env::_push(T value)
{
	_variable.emplace_back('null', value);
}
template<>
inline void Env::_push(Env::Value value)
{
	_variable.push_back(value);
}
template<>
inline void  Env::_push(int value)
{
	_variable.emplace_back('str', value);
}
template<>
inline void  Env::_push(double value)
{
	_variable.emplace_back('num', value);
}
template<>
inline void  Env::_push(bool value)
{
	_variable.emplace_back('bool', value);
}
template<>
inline void  Env::_push(AST::AST* value)
{
	_variable.emplace_back('ast', value);
}

template<typename T>
inline void Env::push(int id, T value)
{
	if (auto iter = _map.find(id); iter != _map.end())
	{
		int id = iter->second;
		_push(id, value);
	}
	else
	{
		_push(value);
		_map[id] = (int)_variable.size() - 1;
	}
}

template<typename T>
inline void Env::push(int id, int type, T value)
{
	if (auto iter = _map.find(id); iter != _map.end())
	{
		int id = iter->second;
		_push(id, value);
	}
	else
	{
		_variable.emplace_back(type, value);
		_map[id] = (int)_variable.size() - 1;
	}
}

template<typename T>
inline void Env::push(T value)
{
	_stack.emplace('null', value);
}
template<>
inline void Env::push(Env::Value value)
{
	_stack.push(value);
}
template<>
inline void Env::push(double value)
{
	_stack.emplace('num', value);
}
template<>
inline void Env::push(int value)
{
	_stack.emplace('str', value);
}
template<>
inline void Env::push(bool value)
{
	_stack.emplace('bool', value);
}
template<>
inline void Env::push(AST::AST* value)
{
	_stack.emplace('ast', value);
}

#pragma endregion
