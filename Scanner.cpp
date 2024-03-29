#include "Scanner.h"
#include "StringTable.h"
#include <iostream>
#include <map>

IScanner::IScanner() :
	_buffer(), _cur(0), _last(0)
{
}

Token::Token IScanner::GetToken()
{
	_cur = _buffer.peek();

	IgnoreBlank();

	IgnoreComment();

	if (EOF == _cur)
	{
		return { 'end',0 };
	}

	if ('\"' == _cur || '\'' == _cur)
	{
		std::string str = GetStringLiteral(_cur);
		return { 'str',StringTable::getInstance().GetId(str.c_str()) };
	}

	if ('_' == _cur || _cur > 127)
	{
		std::string str = GetIdentifierOrKeyword();
		return { 'id', StringTable::getInstance().GetId(str.c_str()) };
	}

	if (isalpha(_cur))
	{
		std::string str = GetIdentifierOrKeyword();
		int keyword = GetKeyWord(str);
		if (keyword == 'true')
		{
			return { 'bool',1 };
		}
		if (keyword == 'fals')
		{
			return { 'bool',0 };
		}
		if (keyword)
		{
			return { keyword,0 };
		}
		return { 'id',StringTable::getInstance().GetId(str.c_str()) };
	}

	if (isdigit(_cur) || ((' ' == _last || '\n' == _last) && '.' == _cur))
	{
		double number = GetNumber();
		return { 'num',number };
	}

	int op = GetOperator();
	if (op)
	{
		return { op,0 };
	}

	Read();
	return { 'err', 0 };
}

inline void IScanner::Read()
{
	_last = _buffer.get();
	_cur = _buffer.peek();
	return;
}

void IScanner::IgnoreBlank()
{
	while (' ' == _cur || '\t' == _cur || '\r' == _cur || '\n' == _cur)
	{
		Read();
	}
}

void IScanner::IgnoreComment()
{
	if ('#' != _cur)
	{
		return;
	}
	while ('\n' != _cur && EOF != _cur)
	{
		Read();
	}
	IgnoreBlank();
	IgnoreComment();
}

std::string IScanner::GetIdentifierOrKeyword()
{
	std::string str;
	str.reserve(16);
	for (;;)
	{
		if (_cur > 127) {
			//GBK
			str += _cur;
			Read();
			str += _cur;
			Read();
			//UTF8
			//str += _cur;
			//Read();
		}
		else if (isalnum(_cur) || '_' == _cur)
		{
			str += _cur;
			Read();
		}
		else
		{
			break;
		}
	}
	return str;
}

std::string IScanner::GetStringLiteral(int quote)
{
	std::string str;
	str.reserve(16);
	do
	{
		Read();
		if (_cur == EOF || _cur == '\r' || _cur == '\n')
		{
			break;
		}
		if (_cur == quote)
		{
			Read();
			break;
		}
		str += _cur;
	} while (true);
	return str;
}

double IScanner::GetNumber()
{
	double number;
	_buffer >> number;
	return number;
}

int IScanner::GetOperator()
{
	int op = _cur;
	switch (_cur)
	{
	case '{':
	case '}':
	case '(':
	case ')':
	case '[':
	case ']':
	case '<':
	case '>':
	case '|':
	case '&':
	case '^':
	case '!':
	case '?':
	case ':':
	case ';':
	case '+':
	case '-':
	case '*':
	case '/':
	case '%':
	case '=':
	case '.':
	case ',':
	case '$':
		Read();
		op = (op << 8) | _cur;
		switch (op)
		{
		case '++':
		case '--':
		case '**':
		case '==':
		case '!=':
		case '<=':
		case '>=':
		case '+=':
		case '-=':
		case '*=':
		case '/=':
		case '%=':
		case ':=':
		case '||':
		case '&&':
			Read();
			return op;
		default:
			return _last;
		}
		break;
	default:
		break;
	}
	return 0;
}

int IScanner::GetKeyWord(const std::string& str)
{
	if (str.compare("if") == 0)
	{
		return 'if'; //26982
	}
	if (str.compare("true") == 0)
	{
		return 'true';
	}
	if (str.compare("false") == 0)
	{
		return 'fals';
	}
	if (str.compare("elif") == 0)
	{
		return 'elif';
	}
	if (str.compare("else") == 0)
	{
		return 'else';
	}
	if (str.compare("func") == 0)
	{
		return 'func';
	}
	if (str.compare("echo") == 0)
	{
		return 'echo';
	}
	if (str.compare("class") == 0)
	{
		return 'cls';
	}
	if (str.compare("goto") == 0)
	{
		return 'goto';
	}
	if (str.compare("return") == 0)
	{
		return 'ret';
	}
	if (str.compare("loop") == 0)
	{
		return 'loop';
	}
	return 0;
}

//int EndianChange( int i )
//{
//	return ((i & 0x000000FF) << 24) | ((i & 0x0000FF00) << 8) | ((i & 0x00FF0000) >> 8) | ((i & 0xFF000000) >> 24);
//}

std::string IScanner::Token2String(const Token::Token& token)
{
	std::stringstream ss;

	char type[4] = { 0 };
	//int itype = EndianChange( token.type );

	memcpy(type, &token.type, sizeof(int));

	ss << "< ";
	for (int i = 3; i > -1; --i)
	{
		if (type[i]) ss << type[i];
	}
	ss << " , ";

	switch (token.type)
	{
	case 'err':
		ss << '~';
		break;
	case 'end':
		ss << '~';
		break;
	case 'num':
		ss << token.value.dValue;
		break;
	case'str':
		ss << '\"' << StringTable::getInstance().GetStr(token.value.iValue) << '\"';
		break;
	case 'id':
		ss << StringTable::getInstance().GetStr(token.value.iValue);
		break;
	default:
		break;
	}

	ss << " >";
	return ss.str();
}

FileScanner::FileScanner(const char* filePath)
{
	std::ifstream sourceFile(filePath);

	_buffer << sourceFile.rdbuf();
}

SrcScanner::SrcScanner(const std::string& src)
{
	_buffer << src;
}
