#pragma once
#include <map>
#include <string>
#include <unistd.h>
#include "../Cache/CacheEntry.h"
#include "AbstractConnection.h"

class CachingConnection : public AbstractConnection
{
	const std::string request;
	CacheEntry *cacheEntry;

	size_t writeOffset;

	bool eof;

public:
	CachingConnection(int _sockFd, const std::string &_request, CacheEntry *_cacheEntry)
		: AbstractConnection(_sockFd), request(_request), cacheEntry(_cacheEntry)
	{
		writeOffset = 0;
		eof = false;
		subscribedEvents = POLLOUT;
		cacheEntry->setWriter(true);
	}

	void eventTriggeredCallback(short events);

	~CachingConnection()
	{
		cacheEntry->setWriter(false);
		close(sockFd);
	}
};

