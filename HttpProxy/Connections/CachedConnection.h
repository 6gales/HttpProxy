#pragma once
#include <map>
#include <unistd.h>
#include "ManagingConnection.h"
#include "../Cache/CacheEntry.h"

class CachedConnection : public ManagingConnection
{
	CacheEntry& cacheEntry;

	size_t writeOffset;

public:
	CachedConnection(int _sockFd, ConnectionManager& manager, CacheEntry& _cacheEntry)
		: ManagingConnection(_sockFd, manager), cacheEntry(_cacheEntry)
	{
		writeOffset = 0;
		subscribedEvents = POLLOUT;
	}

	void eventTriggeredCallback(short events);

	void suspendFromPoll()
	{
		manager.removeConnection(sockFd);
	}

	void restoreToPoll()
	{
		manager.addConnection(this);
	}

	~CachedConnection()
	{
		close(sockFd);
	}
};