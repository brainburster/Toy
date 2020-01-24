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
	if (auto echo = Echo()) return echo;
	if (auto ass = Assignment()) return ass;
	if (auto be = BoolExpr()) return be;
	//if (auto val = Value()) return val;
	if (auto funcdef = FuncDef()) return funcdef;
	if (auto funcall = FunCall()) return funcall;
	if (auto ifstat = If()) return ifstat;
	return nullptr;
}

AST::AST* Parser::Echo()
{
	AST::AST* val;
	if (!Match('echo', '(')) return nullptr;
	if (val = BoolExpr(); !val) return nullptr;
	if (!Match(')')) { SafeDelete(val); return nullptr; }
	return AST::Create<AST::Echo>(val);
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
	if (!Match(')')) { SafeDelete(e); return nullptr; }
	return e;
}

AST::AST* Parser::STR()
{
	if (!Match('str')) return nullptr;
	return AST::Create<AST::StrValue>(Peek(-1).value.iValue);
}

AST::AST* Parser::Assignment()
{
	if (!Match('id', '=')) return nullptr;
	Seek(-2);
	auto id = ID();
	Match('=');
	auto val = Value();
	if (!val) { SafeDelete(id); return nullptr; }
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
	if (auto funcall = FunCall()) return funcall;
	//if (auto id = ID()) return id;
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
		AST::L(arg) = value;
		if (Match(')')) { return head; }
		auto temp = AST::Create<AST::Args>();
		AST::R(arg) = temp;
		arg = temp;
	} while (Match(','));
	SafeDelete(head);
	return nullptr;
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
		AST::L(param) = id;
		if (Match(')')) return head;
		auto temp = AST::Create<AST::Params>();
		AST::R(param) = temp;
		param = temp;
	} while (Match(','));
	SafeDelete(head);
	return nullptr;
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
				return AST::Create<AST::FuncDef>(id, params, block);
			}
			SafeDelete(params);
		}
		SafeDelete(id);
	}

	return nullptr;
}

AST::AST* Parser::FunCall()
{
	if (!Match('id', '('))return nullptr;
	Seek(-2);
	if (auto id = ID())
	{
		if (auto args = Args())
		{
			return AST::Create<AST::FunCall>(id, args);
		}
		SafeDelete(id);
	}
	return nullptr;
}

AST::AST* Parser::BoolExpr()
{
	auto bt = BoolTerm();
	if (!bt) { return nullptr; }
	if (!Match('&&')) return bt;
	auto be = BoolExpr();
	if (!be) { SafeDelete(bt); return nullptr; }
	return AST::CreateBinExpr<'&&'>(bt, be);
}

AST::AST* Parser::BoolTerm()
{
	auto bf = BoolFactor();
	if (!bf) return nullptr;
	if (!Match('||')) return bf;
	auto bt = BoolTerm();
	if (!bt) { SafeDelete(bf); return nullptr; }
	return AST::CreateBinExpr<'||'>(bf, bt);
}

AST::AST* Parser::BoolFactor()
{
	if (Match('!'))
	{
		auto bp = BoolPrim();
		if (!bp) return nullptr;
		return AST::CreateBinExpr<'!'>(nullptr, bp);
	}
	auto bp = BoolPrim();
	if (!bp) return nullptr;
	return bp;
}

AST::AST* Parser::BoolPrim()
{
	auto val1 = Value();
	if (!val1) return nullptr;
	if (Match('==') || Match('>') || Match('>=') || Match('<=') || Match('!=') || Match('<') || Match('>'))
	{
		auto token = Peek(-1);
		auto val2 = Value();
		if (!val2) { SafeDelete(val1); return nullptr; }
		switch (token.type)
		{
		case '==':
			return AST::CreateBinExpr<'=='>(val1, val2);
		case '>':
			return AST::CreateBinExpr<'>'>(val1, val2);
		case '>=':
			return AST::CreateBinExpr<'>='>(val1, val2);
		case '<':
			return AST::CreateBinExpr<'<'>(val1, val2);
		case '<=':
			return AST::CreateBinExpr<'<='>(val1, val2);
		case '!=':
			return AST::CreateBinExpr<'!='>(val1, val2);
		default:
			break;
		}
	}
	return val1;
}

AST::AST* Parser::If()
{
	if (!Match('if', '(')) return nullptr;
	auto condition = BoolExpr();
	if (!Match(')')) { SafeDelete(condition); return nullptr; }
	auto block = Block();
	if (!block) { SafeDelete(condition); return nullptr; }
	//...
	auto elseIfList = nullptr;//todo:完成elseIfList
	return AST::Create<AST::IF>(condition, block, elseIfList);
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
		//std::cout << std::string_view(typeid(*ast).name() + 12, 3) << ":";
		std::cout << "'" << eors->toString() << "'";
	}
	else
	{
		std::cout << typeid(*ast).name() + 12;
	}
	std::cout << ")\n";

	ast->forEachChild([n](auto child) {Parser::PrintAST(child, n + 1); });
}