#include "DirectConnection.h"
#include <cstring>
#include <stdio.h>
#include <netdb.h>
#include <stdexcept>
#include <errno.h>

void DirectConnection::eventTriggeredCallback(short events)
{
	if (isHangedUp(events) || isErrorOccuerd(events) || isInvalid(events))
	{
		fprintf(stderr, "Client disconnected\n");
		fromThis->finish();
		finished = true;
		return;
	}

	if (canRead(events))
	{
		ssize_t bytesRead = fromThis->writeToRecord(this);
		if (bytesRead < 0 && errno != EWOULDBLOCK)
		{
			throw std::runtime_error(std::string("recv: ") + strerror(errno));
		}
		if (0 == bytesRead)
		{
			subscribedEvents = POLLOUT;
		}
	}

	if (canWrite(events))// && buffer.hasData()))
	{
		ssize_t bytesWrote = toThis->readFromRecord(this);
		if (bytesWrote < 0 && errno != EWOULDBLOCK)
		{
			throw std::runtime_error(std::string("send: ") + strerror(errno));
		}
	}

	if (fromThis->isCompleted() && toThis->isCompleted())
	{
		finished = true;
	}
}