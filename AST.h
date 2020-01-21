#pragma once
#include <string>
#include <functional>

//#define SafeDelete(p) \
//	if((p)!=nullptr){\
//		delete (p);\
//		(p)=nullptr;\
//	}else {}

//#define L children[0]
//#define R children[1]

template<typename T>
inline bool SafeDelete(T* p)
{
	if (!p) return false;
	delete p;
	p = nullptr;
	return true;
}

namespace AST
{
	struct AST
	{
		virtual ~AST() = default;
		virtual void forEachChild(const std::function<void(AST * child)>&) {};
	};

	template<int n> struct Tree : AST
	{
		AST* children[n] = {};
		~Tree() noexcept override
		{
			for (int i = 0; i < n; ++i)
			{
				SafeDelete(children[i]);
			}
		}
		void forEachChild(const std::function<void(AST * child)>& callback) override
		{
			for (int i = 0; i < n; ++i)
			{
				callback(children[i]);
			}
		}
	};

	template<> struct Tree<0> : AST {};
	struct ACC :AST {};
	struct Error :AST {};

	template<int t = NULL> struct ASTypeToStr : ASTypeToStr<>
	{
		int type() override { return t; }
	};
	template<> struct ASTypeToStr<>
	{
		virtual int type() { return NULL; }
		std::string toString();
	};

	//2元表达式
	template<int t = NULL> struct BinExpr :Tree<2>, ASTypeToStr<t>, BinExpr<> {};
	template<> struct BinExpr<> { virtual void rtti() {} };
	//表达式
	using Expr = BinExpr<>;

	//标识符
	struct ID :Tree<0>, Expr { int id = 0; };
	using Name = ID;
	//值
	struct NumValue :Tree<0>, Expr { double value = 0; };
	struct StrValue :Tree<0> { int id = 0; };

	//语句
	struct Stats : Tree<2> {};
	template<int t = NULL, int n = 0> struct Stat : Stat<>, Tree<n>, ASTypeToStr<t> {};
	template<> struct Stat<> {};
	template<> struct Stat<'echo'> : Stat<'echo', 1> {};
	template<> struct Stat<'if'> : Stat<'if', 3> {};
	template<> struct Stat<'else'> : Stat<'else', 1> {};
	template<> struct Stat<'elif'> : Stat<'elif', 2> {};

	template<int type>
	inline auto CreateBinExpr(AST* a, AST* b)
	{
		auto e = new BinExpr<type>{};
		e->children[0] = a;
		e->children[1] = b;
		return e;
	};

	inline std::string ASTypeToStr<>::toString()
	{
		int tempi = type();
		char tempc[4] = {};
		memcpy(tempc, &tempi, sizeof(int));
		std::string str{};
		for (int i = 3; i > -1; --i)
		{
			if (!tempc[i]) continue;
			str += tempc[i];
		}
		return str;
	}
}
