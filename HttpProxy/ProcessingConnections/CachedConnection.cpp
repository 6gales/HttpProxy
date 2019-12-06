#include "CachedConnection.h"
#include <string.h>

HttpContext::ProcessingState* CachedConnection::process(HttpContext* context)
{
	if (context->getRegistrar().isSetWrite(sockFd) && cacheEntry->second.size() != writeOffset)
	{
		ssize_t bytesWrote = send(sockFd, cacheEntry->second.data() + writeOffset, cacheEntry->first.size() - writeOffset, 0);
		if (bytesWrote < 0 && bytesWrote != EWOULDBLOCK)
		{
			throw std::runtime_error(std::string("send: ") + strerror(errno));
		}
		if (bytesWrote > 0)
			writeOffset += bytesWrote;
	}

	if (cacheEntry->second.size() > writeOffset)
	{
		context->getRegistrar().registerFd(sockFd, FdRegistrar::Options::Write);
	}
	else
	{
		finished = true;
	}

	return nullptr;
}