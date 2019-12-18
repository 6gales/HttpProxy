#pragma once
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "AbstractConnection.h"

class ErrorConnection : public AbstractConnection
{
	size_t writeOffset;
	std::string errorMessage;

public:
	ErrorConnection(int _sockFd, std::string _errorMessage)
		: AbstractConnection(_sockFd), errorMessage(_errorMessage)
	{
		subscribedEvents = POLLOUT;
		writeOffset = 0;
	}

	void eventTriggeredCallback(short events)
	{
		if (canWrite(events))
		{
			ssize_t bytesWrote = send(sockFd, errorMessage.c_str() + writeOffset, errorMessage.size() - writeOffset, 0);
			if (bytesWrote < 0 && errno != EWOULDBLOCK)
			{
				throw std::runtime_error(std::string("send: ") + strerror(errno));
			}
			
			writeOffset += bytesWrote;
		}

		if (writeOffset >= errorMessage.size())
		{
			finished = true;
		}
	}

	~ErrorConnection()
	{
		close(sockFd);
	}
};