#include "StringTable.h"

char* StringTable::GetStr(int id)
{
	return &_buffer[id];
}

int StringTable::GetId(const char* str)
{
	if (auto iter = _hashmap.find(str); iter != _hashmap.end())
	{
		return iter->second;
	}

	auto temp = str;
	int id = (int)_buffer.size();
	while (true)
	{
		_buffer.push_back(*temp);
		if (!*(temp++))
		{
			break;
		}
	}

	_hashmap.emplace(str, id);

	return id;
}

StringTable::StringTable()
{
	_buffer.reserve(65536);
	_hashmap.reserve(65535);
}

StringTable& StringTable::getInstance()
{
	static StringTable st;
	return st;
}