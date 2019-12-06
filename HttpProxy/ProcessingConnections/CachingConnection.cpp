#include "CachingConnection.h"
#include <string.h>

HttpContext::ProcessingState* CachingConnection::process(HttpContext* context)
{
	if (context->getRegistrar().isSetWrite(sockFd) && cacheEntry->first.size() != writeOffset)
	{
		ssize_t bytesWrote = send(sockFd, cacheEntry->first.c_str() + writeOffset, cacheEntry->first.size() - writeOffset, 0);
		if (bytesWrote < 0 && bytesWrote != EWOULDBLOCK)
		{
			throw std::runtime_error(std::string("send: ") + strerror(errno));
		}
		if (bytesWrote > 0)
			writeOffset += bytesWrote;
	}

	if (context->getRegistrar().isSetRead(sockFd))
	{
		ssize_t bytesRead = recv(sockFd, buffer, BUFF_SIZE, 0);
		if (bytesRead < 0)
		{
			throw std::runtime_error(std::string("recv: ") + strerror(errno));
		}
		if (0 < bytesRead)
		{
			if (cacheEntry->second.capacity() < cacheEntry->second.size() + bytesRead)
			{
				cacheEntry->second.resize(cacheEntry->second.size() + bytesRead);
			}
			
			for (size_t i = 0; i < bytesRead; i++)
			{
				cacheEntry->second.push_back(buffer[i]);
			}
		}
		else
		{
			eof = true;
			cacheEntry->second.shrink_to_fit();
			finished = true;
		}
	}

	if (!eof)
	{
		context->getRegistrar().registerFd(sockFd, FdRegistrar::Options::Read);
	}
	if (cacheEntry->first.size() != writeOffset)
	{
		context->getRegistrar().registerFd(sockFd, FdRegistrar::Options::Write);
	}

	return nullptr;
}