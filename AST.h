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

	template<int t> struct ASTypeToStr;
	template<> struct ASTypeToStr<NULL>
	{
		virtual int type() = 0;
		std::string toString();
	};
	template<int t = NULL> struct ASTypeToStr : ASTypeToStr<NULL>
	{
		int type() override { return t; }
	};

	//2Ԫ����ʽ
	template<int t> struct BinExpr;
	template<> struct BinExpr<NULL> { virtual void RTTI() {} };
	template<int t = NULL> struct BinExpr : Tree<2>, ASTypeToStr<t>, BinExpr<NULL> {};
	//����ʽ
	using Expr = BinExpr<>;

	//��ʶ��
	struct ID : Tree<0>, Expr { int id = 0; };
	using Name = ID;
	//ֵ
	struct NumValue : Tree<0>, Expr { double value = 0; };
	struct StrValue : Tree<0> { int id = 0; };
	struct BoolValue : Tree<0>, Expr { bool value = false; };
	struct Negative : Tree<1>, Expr {};
	//ʵ�Σ��β��б�
	struct Args : Tree<2> {};
	struct Params : Tree<2> {};
	//���
	struct Stats : Tree<2> {};
	struct Echo : Tree<1> {};
	struct FuncDef : Tree<3> {};
	struct FunCall : Tree<2> {};
	struct IF : Tree<3> {};
	struct Else : Tree<1> {};

	template<typename T, typename Enable = std::enable_if_t<1 <= std::extent_v<decltype(T::children)>>>
	AST*& L(T* t)
	{
		return t->children[0];
	}
	template<typename T, typename Enable = std::enable_if_t<2 <= std::extent_v<decltype(T::children)>>>
	AST*& R(T* t)
	{
		return t->children[1];
	}

	template<typename... Args>
	struct Length;
	template<typename T, typename... Args>
	struct Length<T, Args...>
	{
		enum { value = Length<Args...>::value + 1 };
	};
	template<typename T>
	struct Length<T>
	{
		enum { value = 1 };
	};
	template<>
	struct Length<>
	{
		enum { value = 0 };
	};
	template<typename... Args>
	constexpr int Length_v = Length<Args...>::value;

	//��֮����
	template<typename T, typename... Args, typename Enable = std::enable_if_t<std::extent_v<decltype(T::children)> >= Length_v<Args...> && ((std::is_base_of_v<AST, std::remove_pointer_t<Args>> || std::is_same_v<nullptr_t, Args>) &&...) >>
	inline auto Create(Args... args) ->T* //-> Tree<std::extent_v<decltype(T::children)>>*
	{
		auto* tree = new T{};
		int i = 0;
		//std::initializer_list<int>{ ((tree->children[i++] = args), 0)... }; //c++17֮ǰ��д��
		(((tree->children[i++] = args), 0) + ... + 0); //д��3Ԫ�۵�����ʽ��Ŀ����Ϊ��ʹargs����Ϊ��
		return tree;
	}

	template<typename T>
	inline auto Create(decltype(T::id) id)
	{
		auto leaf = new T{};
		leaf->id = id;
		return leaf;
	}

	template<typename T>
	inline auto Create(decltype(T::value) num)
	{
		auto leaf = new T{};
		leaf->value = num;
		return leaf;
	}

	template<typename U, typename V> constexpr bool is_same_v = false;
	template<typename U> constexpr bool is_same_v<U, U> = true;
	template<typename T, typename... Types> constexpr bool is_one_of_v = (is_same_v<T, Types> || ...);
	//template<bool b, typename T = void>
	//struct enable_if {};
	//template<typename T>
	//struct enable_if <true, T> { using type = T; };
	//template<bool b, typename T = void> using enable_if_t = typename enable_if<b, T>::type;

	template<typename T, typename Enable = std::enable_if_t<is_one_of_v<T, ACC, Error>>>
	inline auto Create()
	{
		return new T{};
	}

	//��Ԫ����������
	template<int type>
	inline auto CreateBinExpr(AST* a, AST* b)
	{
		return Create<BinExpr<type>>(a, b);
	};

	//�Ѷ��ֽ��ַ�����������ת��Ϊ�ַ���
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
