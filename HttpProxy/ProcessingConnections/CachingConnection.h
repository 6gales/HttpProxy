#pragma once
#include <map>
#include <vector>
#include <unistd.h>
#include "../HttpContext.h"

class CachingConnection : public HttpContext::ProcessingState
{
	constexpr static size_t BUFF_SIZE = 1024;

	const int sockFd;
	std::map<std::string, std::vector<char>>::iterator cacheEntry;
	
	char buffer[BUFF_SIZE];
	size_t writeOffset = 0;

	bool eof = false;

public:
	CachingConnection(int _sockFd, std::map<std::string, std::vector<char>>::iterator _cacheEntry)
		: sockFd(_sockFd), cacheEntry(_cacheEntry)
	{
		cacheEntry->second.resize(BUFF_SIZE);
	}

	ProcessingState* process(HttpContext* context) override;

	~CachingConnection()
	{
		close(sockFd);
	}
};

