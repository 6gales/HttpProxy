#include "CachedConnection.h"
#include <string.h>
#include <stdio.h>
#include <string>
#include <stdexcept>
#include <errno.h>

void CachedConnection::eventTriggeredCallback(short events)
{
	if (isHangedUp(events) || isErrorOccuerd(events) || isInvalid(events))
	{
		fprintf(stderr, "Client disconnected\n");
		finished = true;
		return;
	}

	if (canWrite(events))
	{
		ssize_t bytesWrote = cacheEntry->readFromRecord(this, writeOffset);
		if (bytesWrote < 0 && errno != EWOULDBLOCK)
		{
			throw std::runtime_error(std::string("send: ") + strerror(errno));
		}
		
		writeOffset += bytesWrote;
	}

	if (cacheEntry->recordSize() <= writeOffset && cacheEntry->isCompleted())
	{
		finished = true;
	}
}