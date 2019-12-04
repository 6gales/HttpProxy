#pragma once
#include <unistd.h>
#include <string.h>
#include "../HttpContext.h"

class ErrorConnection : public HttpContext::ProcessingState
{
	const int sockFd;
	size_t writeOffset;
	std::string errorMessage;

public:
	ErrorConnection(int socket, std::string _errorMessage) : sockFd(socket), errorMessage(_errorMessage)
	{
		writeOffset = 0;
	}

	ProcessingState* process(HttpContext* context)
	{
		if (context->getRegistrar().isSetWrite(sockFd))
		{
			ssize_t bytesWrote = send(sockFd, errorMessage.c_str() + writeOffset, errorMessage.size() - writeOffset, 0);
			if (bytesWrote < 0 && bytesWrote != EWOULDBLOCK)
			{
				throw std::runtime_error(std::string("send: ") + strerror(errno));
			}
			
			writeOffset += bytesWrote;
		}

		if (writeOffset == errorMessage.size())
		{
			finished = true;
		}
		else
		{
			context->getRegistrar().registerFd(sockFd, FdRegistrar::Options::Write);
		}
	}

	~ErrorConnection()
	{
		close(sockFd);
	}
};