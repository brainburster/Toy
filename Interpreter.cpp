#include "Interpreter.h"
#include "StringTable.h"
#include <iostream>

#pragma region PRINT

//私有模板函数，实现可以放到cpp文件中
template<typename T>
inline void Interpreter::Print(T value)
{
	std::cout << "echo: " << value << std::endl;
}
template<>
inline void Interpreter::Print(AST::ID* id)
{
	auto value = _env.get(id->id);
	if (value.has_value())
	{
		if (value->type == 'num')
		{
			Print(value->value.dValue);
		}
		else if (value->type == 'str')
		{
			Print(StringTable::getInstance().GetStr(value->value.iValue));
		}
	}
}

#pragma endregion

bool Interpreter::Eval(AST::AST* ast)
{
	auto stats = dynamic_cast<AST::Stats*>(ast);
	while (nullptr != stats)
	{
		if (!EvalStats(AST::L(stats)))
		{
			return false;
		}
		stats = dynamic_cast<AST::Stats*>(AST::R(stats));
	}
	return true;
}

bool Interpreter::EvalStats(AST::AST* stat)
{
	if (auto echo = dynamic_cast<AST::Echo*>(stat))
	{
		return EvalEcho(echo);
	}
	if (auto ass = dynamic_cast<AST::BinExpr<'='>*>(stat))
	{
		auto id = dynamic_cast<AST::ID*>(AST::L(ass));
		if (!id)
		{
			return false;
		}
		if (auto str = dynamic_cast<AST::StrValue*>(AST::R(ass)))
		{
			_env.push(id->id, str->id);
			return true;
		}
		if (auto expr = dynamic_cast<AST::BinExpr<>*>(AST::R(ass)))
		{
			//计算表达式的值
			double val = EvalExpr(expr);
			_env.push(id->id, val);
			return true;
		}
		return false;
	}
	//其他
	if (auto e = dynamic_cast<AST::BinExpr<>*>(stat))
	{
		Print(EvalExpr(e));
		return true;
	}
	if (auto str = dynamic_cast<AST::StrValue*>(stat))
	{
		Print(EvalStr(str));
		return true;
	}
	return false;
}

bool Interpreter::EvalEcho(AST::Echo* echo)
{
	auto childern = AST::L(echo);
	if (auto id = dynamic_cast<AST::ID*>(childern))
	{
		Print(id);
		return true;
	}
	if (auto e = dynamic_cast<AST::BinExpr<>*>(childern))
	{
		auto value = EvalExpr(e);
		Print(value);
		return true;
	}
	if (auto str = dynamic_cast<AST::StrValue*>(childern))
	{
		auto value = EvalStr(str);
		Print(value);
		return true;
	}
	Print(typeid(*childern).name());
	//打印
	return false;
}

double Interpreter::EvalExpr(AST::Expr* expr)
{
	if (auto d = dynamic_cast<AST::NumValue*>(expr))
	{
		return d->value;
	}
	if (auto id = dynamic_cast<AST::ID*>(expr))
	{
		auto value = _env.get(id->id);
		if (!value.has_value()) return 0.0;
		if (value->type != 'num')
		{
			double v = atof(StringTable::getInstance().GetStr(value->value.iValue));
			return v;
		}
		return value->value.dValue;
	}
	AST::Tree<2>* node;
	double v1 = 0.0;
	double v2 = 0.0;
	if (node = dynamic_cast<AST::Tree<2>*>(expr); !node) return  0.0;

	if (auto c1 = dynamic_cast<AST::BinExpr<>*>(AST::L(node)))
	{
		v1 = EvalExpr(c1);
	}
	if (auto c2 = dynamic_cast<AST::BinExpr<>*>(AST::R(node)))
	{
		v2 = EvalExpr(c2);
	}

	if (typeid(AST::BinExpr<'+'>) == typeid(*expr))
	{
		return v1 + v2;
	}
	if (typeid(AST::BinExpr<'-'>) == typeid(*expr))
	{
		return v1 - v2;
	}
	if (typeid(AST::BinExpr<'*'>) == typeid(*expr))
	{
		return v1 * v2;
	}
	if (typeid(AST::BinExpr<'/'>) == typeid(*expr))
	{
		return v1 / v2;
	}
	if (typeid(AST::BinExpr<'**'>) == typeid(*expr))
	{
		return pow(v1, v2);
	}
	return 0.0;
}

inline const char* Interpreter::EvalStr(AST::StrValue* str)
{
	return 	StringTable::getInstance().GetStr(str->id);
}

inline void Env::push(int id, double value)
{
	if (auto iter = _map.find(id); iter != _map.end())
	{
		int id = iter->second;
		_stack[id] = { 'num',value };
	}
	else
	{
		push(value);
		_map[id] = (int)_stack.size() - 1;
	}
}

inline void Env::push(int id, int value)
{
	if (auto iter = _map.find(id); iter != _map.end())
	{
		int id = iter->second;
		_stack[id] = { 'str',value };
	}
	else
	{
		push(value);
		_map[id] = (int)_stack.size() - 1;
	}
}

inline void Env::push(int value)
{
	_stack.emplace_back('str', value);
}

inline void Env::push(double value)
{
	_stack.emplace_back('num', value);
}

inline int Env::find(int id)
{
	if (auto iter = _map.find(id); iter != _map.end())
	{
		return iter->second;
	}

	return -1;
}

inline std::optional<Env::Value> Env::pop()
{
	if (_stack.size() < 1) return std::optional<Value>{};
	auto v = _stack.back();
	_stack.pop_back();
	return v;
}

inline std::optional<Env::Value> Env::get(int id)
{
	if (auto iter = _map.find(id); iter != _map.end())
	{
		return _stack.at(iter->second);
	}
	return std::optional<Value>{};
}