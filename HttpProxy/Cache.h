#pragma once
#include <map>
#include <vector>

class Cache
{
	std::map<std::string, std::vector<char>> cache;

public:
	std::pair<std::map<std::string, std::vector<char>>::iterator, bool> createIfNotExists(std::string key)
	{
		return cache.insert(std::make_pair(key, std::vector<char>()));
	}
};