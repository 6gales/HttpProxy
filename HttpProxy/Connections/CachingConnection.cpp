#include "CachingConnection.h"
#include <string.h>
#include <poll.h>
#include <netdb.h>

void CachingConnection::eventTriggeredCallback(short events)
{
	if (canWrite(events) && request.size() != writeOffset)
	{
		ssize_t bytesWrote = send(sockFd, request.c_str() + writeOffset, request.size() - writeOffset, 0);
		if (bytesWrote < 0 && errno != EWOULDBLOCK)
		{
			throw std::runtime_error(std::string("send: ") + strerror(errno));
		}
		
		writeOffset += bytesWrote;

		if (request.size() == writeOffset)
		{
			subscribedEvents = POLLIN;
		}
	}

	if (canRead(events))
	{
		ssize_t bytesRead = cacheEntry.writeToRecord(sockFd);
		if (bytesRead < 0)
		{
			throw std::runtime_error(std::string("recv: ") + strerror(errno));
		}
		
		finished = bytesRead == 0;
	}
}