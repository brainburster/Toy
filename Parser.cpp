#include "Parser.h"
#include "Scanner.h"
#include <iostream>
#include "StringTable.h"

template<typename T, typename U>
inline T Min(T a, U b)
{
	return a < b ? a : b;
}
template<typename T, typename U>
inline T Max(T a, U b)
{
	return a > b ? a : b;
}
template<typename T, typename U, typename V>
inline T Clamp(T a, U b, V c)
{
	return Min(Max(a, b), c);
}

const Token::Token& Parser::Peek(int offset)
{
	return _buffer[Clamp<int, int, int>(_cur + offset, 0, (int)_buffer.size() - 1)];
}

int Parser::Seek(int offset)
{
	return _cur = Clamp<int, int, int>(_cur + offset, 0, (int)_buffer.size() - 1);
}

AST::AST* Parser::Parse(const char* path)
{
	Scan(path);
	return Stats();
}

void Parser::Scan(const char* path)
{
	Scanner s{ path };
	while (true)
	{
		auto t = s.GetToken();
		if (t.type == 'err') continue;
		_buffer.push_back(t);
		if (t.type == 'end') break;
	};
	//打印词法分析结果
	std::cout << "词法分析完成：" << std::endl;
	for (const auto& t : _buffer)
	{
		std::cout << s.Token2String(t) << std::endl;
	}
}

AST::AST* Parser::Stats()
{
	AST::Stats* head = AST::Create<AST::Stats>();
	AST::Stats* p = head;
	for (;;)
	{
		if (Match('end') || '}' == Peek().type)
		{
			//std::cout << "语法分析完成:" << std::endl;
			AST::L(p) = AST::Create<AST::ACC>();
			break;
		}
		else if (Match(';'))
		{
			continue;
		}
		else if (auto stat = Stat())
		{
			AST::L(p) = stat; //左子树当前语句
		}
		else
		{
			AST::L(p) = AST::Create<AST::Error>();
			break;
		}

		auto* next = AST::Create<AST::Stats>();
		AST::R(p) = next; //右子树下一段语句
		p = next;
	}
	return head;
}

AST::AST* Parser::Stat()
{
	if (auto echo = EchoStat()) return echo;
	if (auto ass = Assignment()) return ass;
	if (auto val = Value()) return val;
	return nullptr;
}

AST::AST* Parser::EchoStat()
{
	AST::AST* val;
	if (!Match('echo', '(')) return nullptr;
	if (val = Value(); !val) return nullptr;
	if (!Match(')')) return nullptr;
	return AST::Create<AST::Stat<'echo'>>(val);
}

AST::AST* Parser::Expr()
{
	auto t = TermExpr();
	if (Match('+'))
	{
		auto e = Expr();
		return  AST::CreateBinExpr<'+'>(t, e);
	}
	else if (Match('-'))
	{
		auto e = Expr();
		return  AST::CreateBinExpr<'-'>(t, e);
	}

	return t;
}

AST::AST* Parser::TermExpr()
{
	auto f = FactorExpr();
	if (Match('*'))
	{
		auto t = TermExpr();
		return  AST::CreateBinExpr<'*'>(f, t);
	}
	else if (Match('/'))
	{
		auto t = TermExpr();
		return  AST::CreateBinExpr<'/'>(f, t);
	}
	return f;
}

AST::AST* Parser::FactorExpr()
{
	if (Match('-'))
	{
		if (auto p = PrimExpr())
		{
			return AST::CreateBinExpr<'-'>(nullptr, p);
		}
	}
	auto p1 = PrimExpr();
	if (Match('**'))
	{
		if (auto p2 = PrimExpr())
		{
			return  AST::CreateBinExpr<'**'>(p1, p2);
		}
		else {
			return nullptr;
		}
	}
	return p1;
}

AST::AST* Parser::PrimExpr()
{
	AST::AST* e;
	if (auto id = ID()) return id;
	if (auto num = Num()) return num;
	if (!Match('(')) return nullptr;
	if (e = Expr(); !e) return  nullptr;
	if (!Match(')')) return nullptr;
	return e;
}

AST::AST* Parser::STR()
{
	if (!Match('str')) return nullptr;
	return AST::Create<AST::StrValue>(Peek(-1).value.iValue);
}

AST::AST* Parser::Assignment()
{
	auto id = ID();
	if (!id) return nullptr;
	if (!Match('=')) return id;
	auto val = Value();
	return AST::CreateBinExpr<'='>(id, val);
}

AST::AST* Parser::Num()
{
	if (!Match('num'))	return nullptr;
	return AST::Create<AST::NumValue>(Peek(-1).value.dValue);
}

AST::AST* Parser::ID()
{
	if (!Match('id')) return nullptr;
	return AST::Create<AST::ID>(Peek(-1).value.iValue);
}

AST::AST* Parser::Value()
{
	if (auto e = Expr()) return e;
	if (auto str = STR()) return str;
	return nullptr;
}

AST::AST* Parser::Block()
{
	if (!Match('{')) return nullptr;
	auto ss = Stats();
	if (!Match('}')) return nullptr;
	return ss;
}

AST::AST* Parser::Args()
{
	if (Match('(', ')')) return AST::Create<AST::Args>();
	if (!Match('(')) return nullptr;
	auto head = AST::Create<AST::Args>();
	auto arg = head;
	do {
		auto value = Value();
		if (!value) return nullptr;
		auto temp = AST::Create<AST::Args>();
		AST::L(arg) = value;
		AST::R(arg) = temp;
		arg = temp;
	} while (Match(','));
	if (!Match(')')) return nullptr;
	return head;
}

AST::AST* Parser::Params()
{
	if (Match('(', ')')) return  AST::Create< AST::Params>();
	if (!Match('(')) return nullptr;
	auto head = AST::Create< AST::Params>();
	auto param = head;
	do {
		auto id = ID();
		if (!id) return nullptr;
		auto temp = AST::Create< AST::Params>();
		AST::L(param) = id;
		AST::L(param) = temp;
		param = temp;
	} while (Match(','));
	if (!Match(')')) return nullptr;
	return head;
}

AST::AST* Parser::FuncDef()
{
	if (!Match('func')) return nullptr;
	if (auto id = ID())
	{
		if (auto params = Params())
		{
			if (auto block = Block())
			{
				auto funcdef = AST::Create<AST::FuncDef>(id, params, block);
			}
		}
	}

	return nullptr;
}

AST::AST* Parser::FunCall()
{
	return nullptr;
}

//打印抽象语法树
void Parser::PrintAST(AST::AST* ast, int n)
{
	if (!ast)
	{
		return;
	}

	for (int i = 0; i < n; i++)
	{
		std::cout << '\t';
	}
	std::cout << "(";
	if (auto astd = dynamic_cast<AST::NumValue*>(ast))
	{
		std::cout << astd->value;
	}
	else if (auto aststr = dynamic_cast<AST::StrValue*>(ast))
	{
		char* str = StringTable::getInstance().GetStr(aststr->id);
		std::cout << "\"" << str << "\"";
	}
	else if (auto astid = dynamic_cast<AST::ID*>(ast))
	{
		char* id = StringTable::getInstance().GetStr(astid->id);
		std::cout << "id : " << id;
	}
	else if (auto eors = dynamic_cast<AST::ASTypeToStr<>*>(ast))
	{
		std::cout << std::string_view(typeid(*ast).name() + 12, 1) << ":";
		std::cout << "'" << eors->toString() << "'";
	}
	else
	{
		std::cout << typeid(*ast).name() + 12;
	}
	std::cout << ")\n";

	ast->forEachChild([n](auto child) {Parser::PrintAST(child, n + 1); });
}