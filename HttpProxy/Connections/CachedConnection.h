#pragma once
#include <map>
#include <unistd.h>
#include "ManagingConnection.h"
#include "../Cache/CacheEntry.h"

class CachedConnection : public ManagingConnection
{
	CacheEntry &cacheEntry;

	size_t writeOffset = 0;

public:
	CachedConnection(int _sockFd, ConnectionManager &manager, CacheEntry &_cacheEntry)
		: ManagingConnection(_sockFd, manager), cacheEntry(_cacheEntry)
	{
		subscribedEvents = POLLOUT;
	}

	void eventTriggeredCallback(short events) override;

	~CachedConnection()
	{
		close(sockFd);
	}
};