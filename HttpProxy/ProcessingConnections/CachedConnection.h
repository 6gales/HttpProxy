#pragma once
#include <unistd.h>
#include "../HttpContext.h"

class CachedConnection : public HttpContext::ProcessingState
{
	const int sockFd;
	std::map<std::string, std::vector<char>>::iterator cacheEntry;

	size_t writeOffset = 0;

public:
	CachedConnection(int _sockFd, std::map<std::string, std::vector<char>>::iterator _cacheEntry)
		: sockFd(_sockFd), cacheEntry(_cacheEntry)
	{}

	ProcessingState* process(HttpContext* context) override;

	~CachedConnection()
	{
		close(sockFd);
	}
};