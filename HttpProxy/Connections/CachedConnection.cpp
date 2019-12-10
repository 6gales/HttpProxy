#include "CachedConnection.h"
#include <string.h>

void CachedConnection::eventTriggeredCallback(short events)
{
	if (canWrite(events))
	{
		ssize_t bytesWrote = cacheEntry.readFromRecord(this, writeOffset);
		if (bytesWrote < 0 && errno != EWOULDBLOCK)
		{
			throw std::runtime_error(std::string("send: ") + strerror(errno));
		}
		
		writeOffset += bytesWrote;
	}

	if (cacheEntry.recordSize() <= writeOffset && cacheEntry.isCompleted())
	{
		finished = true;
	}
}