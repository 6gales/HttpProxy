#pragma once
#include <vector>

class CacheEntry
{
	bool isFull;
	std::vector<char> record;

	std::vector<int> readers;


};