#pragma once

namespace Token
{
	union ValueType
	{
		double dValue;
		int iValue;
		//
		ValueType() = default;
		ValueType(int i) : iValue(i) {}
		ValueType(double d) : dValue(d) {}
		ValueType(const ValueType&) = default;
	};

	struct Token
	{
		int type;
		ValueType value;
		//
		Token() = default;
		template<typename T>
		Token(int t, T v) :type(t), value(v) {}
		Token(const Token& other) = default;
	};
}