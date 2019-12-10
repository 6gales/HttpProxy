#pragma once
#include <map>
#include <vector>
#include <unistd.h>
#include "../Cache/CacheEntry.h"
#include "AbstractConnection.h"

class CachingConnection : public AbstractConnection
{
	const std::string request;
	CacheEntry& cacheEntry;
	
	size_t writeOffset = 0;

	bool eof = false;

public:
	CachingConnection(int _sockFd, std::string _request, CacheEntry& _cacheEntry)
		: AbstractConnection(_sockFd), request(_request), cacheEntry(_cacheEntry)
	{
		subscribedEvents = POLLIN | POLLOUT;
	}

	void eventTriggeredCallback(short events) override;

	~CachingConnection()
	{
		close(sockFd);
	}
};

