#include "CachingConnection.h"
#include <string.h>
#include <netdb.h>
#include <stdexcept>
#include <errno.h>
#include "../Utils/InetUtils.h"

void CachingConnection::eventTriggeredCallback(short events)
{
	if (!isConnected)
	{
		try
		{
			isConnected = checkIfConnected(sockFd);
		}
		catch (std::exception &e)
		{
			throw e;
		}
	}
	
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
		ssize_t bytesRead = cacheEntry->writeToRecord(sockFd);
		if (bytesRead < 0)
		{
			throw std::runtime_error(std::string("recv: ") + strerror(errno));
		}

		finished = (bytesRead == 0);//TODO isCompleted?
	}
}