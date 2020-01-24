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
}

#pragma endregion

bool Interpreter::Eval(AST::AST* ast)
{
	auto stats = dynamic_cast<AST::Stats*>(ast);
	while (nullptr != stats)
	{
		if (!EvalStats(AST::L(stats)))
		{
			//_curEnv->clearStack();
			return false;
		}
		//_curEnv->clearStack();//清空了栈就没有返回值了
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
	Env env{};
	auto old = _curEnv;
	_curEnv = &env;
	//参数压栈
	for (int param : paramlist)
	{
		AST::AST* value = AST::L(args);
		EvalAssignment(param, value);
		if (args = dynamic_cast<AST::Args*>(AST::R(args)); !args) break;
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
	if (!condition) { {return false; } }
	auto block = ifstat->children[1];
	auto elseIfList = ifstat->children[2];
	EvalExpr(condition);
	bool con = false;
	if (auto temp = _curEnv->pop(); temp.has_value())
	{
		switch (temp->type)
		{
		case 'num':
			con = temp->value.dValue;
			break;
		case 'bool':
			con = temp->value.bValue;
			break;
		case 'str':
			con = true;
		case 'null':
			con = false;
		default:
			break;
		}
	}

	if (con)
	{
		return Eval(block);
	}
	if (elseIfList)
	{
		EvalElseIfList(elseIfList);
		return true;
	}

	return true;
}

bool Interpreter::EvalElseIfList(AST::AST* elseIfList)
{
	if (auto _else = dynamic_cast<AST::Else*>(elseIfList))
	{
		return Eval(_else->children[0]);
	}
	if (auto elif = dynamic_cast<AST::IF*>(elseIfList))
	{
		EvalIf(elif);
		return true;
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
		if (EvalAssignment(id->id, AST::R(ass)))
		{
			return true;
		}
		return false;
	}
	if (auto funcdef = dynamic_cast<AST::FuncDef*>(stat)) {
		return EvalFuncDef(funcdef);
	}
	if (auto funcall = dynamic_cast<AST::FunCall*>(stat))
	{
		return EvalFunCall(funcall);
	}
	if (auto ifstat = dynamic_cast<AST::IF*>(stat))
	{
		return EvalIf(ifstat);
	}
	//其他
	if (auto e = dynamic_cast<AST::BinExpr<>*>(stat))
	{
		if (!EvalExpr(e)) { return false; }
		Print(_curEnv->top()->value.dValue);
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
		//计算表达式的值
		if (!EvalExpr(expr)) { return false; }
		auto result = _curEnv->pop();
		if (!result.has_value()) { return false; }
		_curEnv->push(id, result.value());
		return true;
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
	//打印
	return false;
}

bool Interpreter::EvalExpr(AST::Expr* expr)
{
	if (auto d = dynamic_cast<AST::NumValue*>(expr))
	{
		_curEnv->push(d->value);
		return true;
	}
	if (auto id = dynamic_cast<AST::ID*>(expr))
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
		_curEnv->pop();
		_curEnv->push(!_curEnv->pop()->value.bValue);
		return true;
	}
	if (typeid(AST::BinExpr<'=='>) == typeid(*expr))
	{
		_curEnv->push(_curEnv->pop()->value.dValue == _curEnv->pop()->value.dValue);
		return true;
	}
	if (typeid(AST::BinExpr<'>='>) == typeid(*expr))
	{
		_curEnv->push(_curEnv->pop()->value.dValue >= _curEnv->pop()->value.dValue);
		return true;
	}
	if (typeid(AST::BinExpr<'<='>) == typeid(*expr))
	{
		_curEnv->push(_curEnv->pop()->value.dValue <= _curEnv->pop()->value.dValue);
		return true;
	}
	if (typeid(AST::BinExpr<'!='>) == typeid(*expr))
	{
		_curEnv->push(_curEnv->pop()->value.dValue != _curEnv->pop()->value.dValue);
		return true;
	}
	if (typeid(AST::BinExpr<'>'>) == typeid(*expr))
	{
		_curEnv->push(_curEnv->pop()->value.dValue > _curEnv->pop()->value.dValue);
		return true;
	}
	if (typeid(AST::BinExpr<'<'>) == typeid(*expr))
	{
		_curEnv->push(_curEnv->pop()->value.dValue < _curEnv->pop()->value.dValue);
		return true;
	}
	if (typeid(AST::BinExpr<'+'>) == typeid(*expr))
	{
		_curEnv->push(_curEnv->pop()->value.dValue + _curEnv->pop()->value.dValue);
		return true;
	}
	if (typeid(AST::BinExpr<'-'>) == typeid(*expr))
	{
		_curEnv->push(_curEnv->pop()->value.dValue - _curEnv->pop()->value.dValue);
		return true;
	}
	if (typeid(AST::BinExpr<'*'>) == typeid(*expr))
	{
		_curEnv->push(_curEnv->pop()->value.dValue * _curEnv->pop()->value.dValue);
		return true;
	}
	if (typeid(AST::BinExpr<'/'>) == typeid(*expr))
	{
		_curEnv->push(_curEnv->pop()->value.dValue / _curEnv->pop()->value.dValue);
		return true;
	}
	if (typeid(AST::BinExpr<'**'>) == typeid(*expr))
	{
		_curEnv->push(pow(_curEnv->pop()->value.dValue, _curEnv->pop()->value.dValue));
		return true;
	}
	return false;
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
	if (_stack.size() < 1) return std::optional<Value>{};
	auto v = _stack.top();
	_stack.pop();
	return v;
}

std::optional<Env::Value> Env::top()
{
	if (_stack.size() < 1) return std::optional<Value>{};
	return _stack.top();
}

inline std::optional<Env::Value> Env::get(int id)
{
	if (auto iter = _map.find(id); iter != _map.end())
	{
		return _variable.at((size_t)iter->second);
	}
	return std::optional<Value>{};
}