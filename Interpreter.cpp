#include "Interpreter.h"
#include "StringTable.h"
#include <iostream>
#include <list>

#pragma region PRINT

template<typename T>
inline void Interpreter::Print(T value)
{
	std::cout << ">>: " << value << std::endl;
}

template<>
inline void Interpreter::Print(const char* value)
{
	std::cout << ">>: \"" << value << "\"" << std::endl;
}

template<>
inline void Interpreter::Print(char* value)
{
	std::cout << ">>: \"" << value << "\"" << std::endl;
}

template<>
inline void Interpreter::Print(AST::ID* id)
{
	auto value = getVar(id->id);
	if (!value.has_value()) return;
	if (value->type == 'num')
	{
		Print(value->value.dValue);
	}
	else if (value->type == 'str')
	{
		Print(StringTable::getInstance().GetStr(value->value.iValue));
	}
	else if (value->type == 'arr')
	{
		std::cout << ">>: [";
		auto v = _curEnv->get(id->id);
		if (!v.has_value()) return;
		int location = v.value().value.iValue;
		int length = _curEnv->_variable[location - 1].value.iValue;
		for (int i = 0; i < length; ++i)
		{
			auto v = _curEnv->at(location + i);
			if (!v.has_value()) return;
			auto value = v.value();
			if (value.type == 'num')
			{
				std::cout << value.value.dValue;
			}
			else if (value.type == 'str')
			{
				std::cout << "\"" << StringTable::getInstance().GetStr(value.value.iValue) << "\"";
			}
			if (i != length - 1)
			{
				std::cout << ", ";
			}
		}
		std::cout << "]" << std::endl;
	}
}

#pragma endregion

bool Interpreter::Eval(AST::AST* ast)
{
	auto stats = dynamic_cast<AST::Stats*>(ast);
	while (nullptr != stats)
	{
		if (auto acc = dynamic_cast<AST::ACC*>(AST::L(stats)))
		{
			return true;
		}
		if (auto ret = dynamic_cast<AST::Ret*>(AST::L(stats)))
		{
			return EvalRet(ret);
		}
		if (!EvalStats(AST::L(stats)))
		{
			return false;
		}
		stats = dynamic_cast<AST::Stats*>(AST::R(stats));
	}
	return true;
}

bool Interpreter::EvalFuncDef(AST::FuncDef* funcdef)
{
	auto id = dynamic_cast<AST::ID*>(funcdef->children[0]);
	if (!id) { return false; }
	_curEnv->push(id->id, funcdef);
	return true;
}

bool Interpreter::EvalFunCall(AST::FunCall* funcall)
{
	auto id = dynamic_cast<AST::ID*>(funcall->children[0]);
	auto args = dynamic_cast<AST::Args*>(funcall->children[1]);
	if (!id || !args) { return false; }
	auto temp = getVar(id->id);
	if (!temp.has_value()) { return false; }
	auto funcdef = dynamic_cast<AST::FuncDef*>(temp->value.astValue);
	if (!funcdef) { return false; }
	auto params = dynamic_cast<AST::Params*>(funcdef->children[1]);
	auto block = dynamic_cast<AST::Stats*>(funcdef->children[2]);
	if (!params || !block) { return false; }
	std::list<int> paramlist;
	for (;;)
	{
		auto param = dynamic_cast<AST::ID*>(AST::L(params));
		if (!param) break;
		paramlist.push_back(param->id);
		params = dynamic_cast<AST::Params*>(AST::R(params));
		if (!params) break;
	}
	std::list <Env::Value> arglist;
	for (int param : paramlist)
	{
		if (auto expr = dynamic_cast<AST::Expr*>(AST::L(args))) {
			if (EvalExpr(expr))
			{
				auto v = _curEnv->pop();
				arglist.push_back(v.value_or(Env::Value{ 'num',0 }));
			}
		}
		if (args = dynamic_cast<AST::Args*>(AST::R(args)); !args) break;
	}

	Env env{};
	auto old = _curEnv;
	_curEnv = &env;

	for (int param : paramlist)
	{
		_curEnv->push(param, arglist.front());
		arglist.pop_front();
	}

	Eval(block);
	auto result = _curEnv->pop();
	_curEnv = old;
	if (result.has_value())
	{
		_curEnv->push(result.value());
	}
	else
	{
		_curEnv->push(Env::Value{ 'null',0 });
	}
	return true;
}

bool Interpreter::EvalIf(AST::IF* ifstat)
{
	auto condition = dynamic_cast<AST::Expr*>(ifstat->children[0]);
	auto block = ifstat->children[1];
	auto elseIfList = ifstat->children[2];
	if (CheckCondition(condition))
	{
		return Eval(block);
	}
	if (elseIfList)
	{
		return EvalElseIfList(elseIfList);
	}

	return true;
}

bool Interpreter::CheckCondition(AST::Expr* condition)
{
	if (EvalExpr(condition))
	{
		if (auto temp = _curEnv->pop(); temp.has_value())
		{
			switch (temp->type)
			{
			case 'arr':
				return true;
			case 'len':
				return true;
			case 'num':
				return temp->value.dValue;
			case 'bool':
				return temp->value.bValue;
			case 'str':
				return true;
			case 'null':
				return false;
			}
		}
	}
	return false;
}

bool Interpreter::EvalElseIfList(AST::AST* elseIfList)
{
	if (auto _else = dynamic_cast<AST::Else*>(elseIfList))
	{
		return Eval(_else->children[0]);
	}
	if (auto elif = dynamic_cast<AST::IF*>(elseIfList))
	{
		return EvalIf(elif);
	}
	return true;
}

bool Interpreter::EvalLoop(AST::Loop* loop)
{
	auto condition = dynamic_cast<AST::Expr*>(loop->children[0]);
	auto block = loop->children[1];
	while (CheckCondition(condition))
	{
		if (!Eval(block)) {
			break;
		}
	}
	return true;
}

bool Interpreter::EvalRet(AST::Ret* ret)
{
	if (auto expr = dynamic_cast<AST::Expr*>(ret->children[0]))
	{
		EvalExpr(expr);
	}
	else
	{
		_curEnv->push(Env::Value{ 'null',0 });
	}
	return false;
}

int Interpreter::EvalArray(AST::Array* arr)
{
	int location = static_cast<int>(_curEnv->_variable.size()) + 1;
	int length = 0;
	_curEnv->_push(length);
	if (!AST::L(arr))
	{
		_curEnv->_push(0);
		return location;
	}
	for (;;)
	{
		auto v = dynamic_cast<AST::Expr*>(AST::L(arr));
		if (!v) return -1;
		if (!EvalExpr(v)) return -1;
		auto value = _curEnv->pop();
		if (!value.has_value()) return false;
		_curEnv->_push(value.value());
		_curEnv->_variable[location - 1] = { 'len', ++length }; //cookie
		arr = dynamic_cast<AST::Array*>(AST::R(arr));
		if (!arr) break;
	}
	return location;
}

bool Interpreter::EvalAt(AST::At* at)
{
	if (auto v = _curEnv->get(static_cast<AST::ID*>(AST::L(at))->id))
	{
		int location = v.value().value.iValue;
		if (auto off = dynamic_cast<AST::Expr*>(AST::R(at)))
		{
			if (!EvalExpr(off)) return false;
			auto value = _curEnv->pop();
			if (!value.has_value()) return false;
			int offset = static_cast<int>(value.value().value.dValue);
			int length = _curEnv->_variable[location - 1].value.iValue;
			if (offset >= length)
			{
				std::cout << "Error: " << "out of range" << std::endl;
				return false;
			}
			auto v = _curEnv->at(location + offset);
			if (!v.has_value()) return false;
			_curEnv->push(v.value());
		}
		else
		{
			std::cout << "Error: " << "no index in \"[]\"" << std::endl;
			return false;
		}
	}
	else
	{
		auto vv = _global->get(static_cast<AST::ID*>(AST::L(at))->id);
		if (!vv.has_value()) return false;
		int location = vv.value().value.iValue;
		if (auto off = dynamic_cast<AST::Expr*>(AST::R(at)))
		{
			if (!EvalExpr(off)) return false;
			auto value = _curEnv->pop();
			if (!value.has_value()) return false;
			int offset = static_cast<int>(value.value().value.dValue);
			int length = _global->_variable[location - 1].value.iValue;
			if (offset >= length)
			{
				std::cout << "Error: " << "out of range" << std::endl;
				return false;
			}
			auto v = _global->at(location + offset);
			if (!v.has_value()) return false;
			_curEnv->push(v.value());
		}
		else
		{
			std::cout << "Error: " << "no index in \"[]\"" << std::endl;
			return false;
		}
	}
	return true;
}

bool Interpreter::EvalStats(AST::AST* stat)
{
	if (auto echo = dynamic_cast<AST::Echo*>(stat))
	{
		if (!EvalEcho(echo))
		{
			std::cout << "Error: echo error." << std::endl;
		}
		return true;
	}
	if (auto ass = dynamic_cast<AST::BinExpr<'='>*>(stat))
	{
		if (auto id = dynamic_cast<AST::ID*>(AST::L(ass)))
		{
			if (EvalAssignment(id->id, AST::R(ass)))
			{
				return true;
			}
		}
		else if (auto at = dynamic_cast<AST::At*>(AST::L(ass)))
		{
			if (auto v = _curEnv->get(static_cast<AST::ID*>(AST::L(at))->id)) {
				int location = v.value().value.iValue;
				if (auto off = dynamic_cast<AST::Expr*>(AST::R(at)))
				{
					if (!EvalExpr(off)) return false;
					auto value = _curEnv->pop();
					if (!value.has_value()) return false;
					int offset = static_cast<int>(value.value().value.dValue);
					int length = _curEnv->_variable[location - 1].value.iValue;
					if (offset >= length)
					{
						std::cout << "Error: " << "out of range" << std::endl;
						return false;
					}
					auto v = dynamic_cast<AST::Expr*>(AST::R(ass));
					if (!v) return false;
					if (!EvalExpr(v)) return false;
					auto v2 = _curEnv->pop();
					if (!v2.has_value()) return false;
					_curEnv->_variable[location + offset] = v2.value();
					return true;
				}
			}
			else {
				auto vv = _global->get(static_cast<AST::ID*>(AST::L(at))->id);
				if (!vv.has_value()) return false;
				int location = vv.value().value.iValue;
				if (auto off = dynamic_cast<AST::Expr*>(AST::R(at)))
				{
					if (!EvalExpr(off)) return false;
					auto value = _curEnv->pop();
					if (!value.has_value()) return false;
					int offset = static_cast<int>(value.value().value.dValue);
					int length = _global->_variable[location - 1].value.iValue;
					if (offset >= length)
					{
						std::cout << "Error: " << "out of range" << std::endl;
						return false;
					}
					auto v = dynamic_cast<AST::Expr*>(AST::R(ass));
					if (!v) return false;
					if (!EvalExpr(v)) return false;
					auto v2 = _curEnv->pop();
					if (!v2.has_value()) return false;
					_global->_variable[location + offset] = v2.value();
					return true;
				}
			}
		}
		std::cout << "EError: Assignment error. " << std::endl;
		return false;
	}
	if (auto funcdef = dynamic_cast<AST::FuncDef*>(stat)) {
		if (!EvalFuncDef(funcdef))
		{
			std::cout << "EError: funcdef error." << std::endl;
			return false;
		}
		return true;
	}

	if (auto ifstat = dynamic_cast<AST::IF*>(stat))
	{
		return EvalIf(ifstat);
	}
	if (auto loop = dynamic_cast<AST::Loop*>(stat))
	{
		return EvalLoop(loop);
	}

	if (auto e = dynamic_cast<AST::BinExpr<>*>(stat))
	{
		if (!EvalExpr(e))
		{
			std::cout << "EError: Expr error." << std::endl;
			return false;
		}
		//Print(_curEnv->top()->value.dValue);
		return true;
	}
	if (auto str = dynamic_cast<AST::StrValue*>(stat))
	{
		//EvalStr(str);
		Print(EvalStr(str));
		return true;
	}
	return false;
}

bool Interpreter::EvalAssignment(int id, AST::AST* value)
{
	if (auto str = dynamic_cast<AST::StrValue*>(value))
	{
		_curEnv->push(id, str->id);
		return true;
	}
	if (auto expr = dynamic_cast<AST::BinExpr<>*>(value))
	{
		if (!EvalExpr(expr)) {
			return false;
		}
		auto result = _curEnv->pop();
		if (!result.has_value()) { return false; }
		_curEnv->push(id, result.value());
		return true;
	}
	if (auto arr = dynamic_cast<AST::Array*>(value))
	{
		if (int loc = EvalArray(arr); loc > -1)
		{
			_curEnv->push(id, 'arr', loc);
			return true;
		}
		return false;
	}
	if (auto funcall = dynamic_cast<AST::FunCall*>(value))
	{
		if (!EvalFunCall(funcall)) { return false; }
		auto result = _curEnv->pop();
		if (!result.has_value()) { return false; }
		_curEnv->push(id, result.value());
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
		if (!EvalExpr(e)) { return false; }
		Print(_curEnv->pop()->value.dValue);
		return true;
	}
	if (auto str = dynamic_cast<AST::StrValue*>(childern))
	{
		auto value = EvalStr(str);
		Print(value);
		return true;
	}
	Print(typeid(*childern).name());

	return false;
}

bool Interpreter::EvalExpr(AST::Expr* expr)
{
	if (auto funcall = dynamic_cast<AST::FunCall*>(expr))
	{
		if (!EvalFunCall(funcall))
		{
			std::cout << "EError: funcall error." << std::endl;
			return false;
		}
		return true;
	}
	if (auto d = dynamic_cast<AST::NumValue*>(expr))
	{
		_curEnv->push(d->value);
		return true;
	}
	if (auto d = dynamic_cast<AST::BoolValue*>(expr))
	{
		_curEnv->push(d->value);
		return true;
	}
	if (auto neg = dynamic_cast<AST::Negative*>(expr))
	{
		return EvalNeg(neg);
	}
	if (auto id = dynamic_cast<AST::ID*>(expr))
	{
		return EvalID(id);
	}
	if (auto at = dynamic_cast<AST::At*>(expr))
	{
		return EvalAt(at);
	}

	AST::Tree<2>* node;
	if (node = dynamic_cast<AST::Tree<2>*>(expr); !node) { return false; }

	if (auto c1 = dynamic_cast<AST::BinExpr<>*>(AST::L(node)))
	{
		if (!EvalExpr(c1)) { return false; }
	}
	if (auto c2 = dynamic_cast<AST::BinExpr<>*>(AST::R(node)))
	{
		if (!EvalExpr(c2)) { return false; }
	}

	if (typeid(AST::BinExpr<'&&'>) == typeid(*expr))
	{
		_curEnv->push(_curEnv->pop()->value.bValue && _curEnv->pop()->value.bValue);
		return true;
	}
	if (typeid(AST::BinExpr<'||'>) == typeid(*expr))
	{
		_curEnv->push(_curEnv->pop()->value.bValue || _curEnv->pop()->value.bValue);
		return true;
	}
	if (typeid(AST::BinExpr<'!'>) == typeid(*expr))
	{
		auto value = _curEnv->pop();
		if (!value.has_value()) { return false; }
		switch (value->type)
		{
		case 'num':
			_curEnv->push(!value->value.dValue);
			break;
		case 'bool':
			_curEnv->push(!value->value.bValue);
			break;
		case 'str':
			_curEnv->push(false);
			break;
		default:
			_curEnv->push(true);
			break;
		}
		return true;
	}
	auto rhs = _curEnv->pop()->value.dValue;
	auto lhs = _curEnv->pop()->value.dValue;

	if (typeid(AST::BinExpr<'=='>) == typeid(*expr))
	{
		_curEnv->push(lhs == rhs);
		return true;
	}
	if (typeid(AST::BinExpr<'>='>) == typeid(*expr))
	{
		_curEnv->push(lhs >= rhs);
		return true;
	}
	if (typeid(AST::BinExpr<'<='>) == typeid(*expr))
	{
		_curEnv->push(lhs <= rhs);
		return true;
	}
	if (typeid(AST::BinExpr<'!='>) == typeid(*expr))
	{
		_curEnv->push(lhs != rhs);
		return true;
	}
	if (typeid(AST::BinExpr<'>'>) == typeid(*expr))
	{
		_curEnv->push(lhs > rhs);
		return true;
	}
	if (typeid(AST::BinExpr<'<'>) == typeid(*expr))
	{
		_curEnv->push(lhs < rhs);
		return true;
	}
	if (typeid(AST::BinExpr<'+'>) == typeid(*expr))
	{
		_curEnv->push(lhs + rhs);
		return true;
	}
	if (typeid(AST::BinExpr<'-'>) == typeid(*expr))
	{
		_curEnv->push(lhs - rhs);
		return true;
	}
	if (typeid(AST::BinExpr<'*'>) == typeid(*expr))
	{
		_curEnv->push(lhs * rhs);
		return true;
	}
	if (typeid(AST::BinExpr<'/'>) == typeid(*expr))
	{
		_curEnv->push(lhs / rhs);
		return true;
	}
	if (typeid(AST::BinExpr<'**'>) == typeid(*expr))
	{
		_curEnv->push(pow(lhs, rhs));
		return true;
	}
	return false;
}

bool Interpreter::EvalNeg(AST::Negative* neg)
{
	if (auto e = dynamic_cast<AST::Expr*>(AST::L(neg))) {
		if (!EvalExpr(e)) { return false; }
		_curEnv->push(-_curEnv->pop()->value.dValue);
		return true;
	}
	return false;
}

bool Interpreter::EvalID(AST::ID* id)
{
	auto value = getVar(id->id);
	if (!value.has_value()) { return false; }
	switch (value->type)
	{
	case 'num':
		_curEnv->push(value->value.dValue);
		break;
	case 'str':
		_curEnv->push(atof(StringTable::getInstance().GetStr(value->value.iValue)));
		break;
	case 'bool':
		_curEnv->push((double)value->value.bValue);
		break;
	default:
		_curEnv->push(0);
		break;
	}
	return true;
}

inline const char* Interpreter::EvalStr(AST::StrValue* str)
{
	return 	StringTable::getInstance().GetStr(str->id);
}

std::optional<Env::Value> Interpreter::getVar(int id)
{
	auto temp = _curEnv->get(id);
	if (!temp.has_value())temp = _global->get(id);
	return temp;
}

void Env::clearStack()
{
	//std::stack<Env::Value> newStack{};
	//_stack.swap(newStack);
	while (!_stack.empty())
	{
		_stack.pop();
	}
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
	if (_stack.size() < 1) return std::nullopt;
	auto v = _stack.top();
	_stack.pop();
	return v;
}

std::optional<Env::Value> Env::top()
{
	if (_stack.size() < 1) return std::nullopt;
	return _stack.top();
}

inline std::optional<Env::Value> Env::get(int id)
{
	if (auto iter = _map.find(id); iter != _map.end())
	{
		return _variable.at((size_t)iter->second);
	}
	return std::nullopt;
}

std::optional<Env::Value> Env::at(int location)
{
	if (location >= 0 && location < _variable.size()) {
		return _variable[location];
	}
	return std::nullopt;
}
