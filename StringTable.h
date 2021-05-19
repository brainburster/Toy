#pragma once
#include <vector>
#include <string>
#include <unordered_map>

class StringTable
{
public:
	char* GetStr(int id);
	int GetId(const char* str);
	static StringTable& getInstance();
private:
	std::vector<char> _buffer;
	std::unordered_map<std::string, int > _hashmap;
	StringTable();
};
