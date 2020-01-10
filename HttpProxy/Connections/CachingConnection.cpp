#include "CachingConnection.h"
#include <string.h>
#include <netdb.h>
#include <stdexcept>
#include <errno.h>
#include <stdio.h>

void CachingConnection::eventTriggeredCallback(short events)
{
	bool isUseful = false;
	
	if (canWrite(events) && request.size() != writeOffset)
	{
		ssize_t bytesWrote = send(sockFd, request.c_str() + writeOffset, request.size() - writeOffset, 0);
		if (bytesWrote < 0 && errno != EWOULDBLOCK)
		{
			throw std::runtime_error(std::string("send: ") + strerror(errno));
		}
		/////////////////////
		if (bytesWrote > 0)
		{
			isUseful = true;
		}
		else
		{
			fprintf(stderr, "caching conn wrote %d bytes\n", bytesWrote);
		}
		/////////////////////
		writeOffset += bytesWrote;

		if (request.size() == writeOffset)
		{
			subscribedEvents = POLLIN;
		}
	}

	if (canRead(events))
	{
		ssize_t bytesRead = cacheEntry->writeToRecord(sockFd);
		if (bytesRead < 0)
		{
			throw std::runtime_error(std::string("recv: ") + strerror(errno));
		}
		/////////////////
		if (bytesRead > 0)
		{
			isUseful = true;
		}
		else
		{
			fprintf(stderr, "caching conn read %d bytes\n", bytesRead);
		}
		
		finished = (bytesRead == 0);//TODO isCompleted?
	}

	if (!isUseful)
	{
		fprintf(stderr, "Useless iteration\n");
	}
}