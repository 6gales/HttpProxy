#pragma once
#include <poll.h>

class AbstractConnection
{
protected:
	const int sockFd;
	bool finished;
	short subscribedEvents;

	bool canRead(short events)
	{
		return events & POLLIN;
	}

	bool canWrite(short events)
	{
		return events & POLLOUT;
	}

public:
	AbstractConnection(int _sockFd) : sockFd(_sockFd)
	{
		finished = false;
		subscribedEvents = 0;
	}

	short getSubscribedEvents() { return subscribedEvents; }

	int getFd() const { return sockFd; }

	bool isFinished() const { return finished; }

	virtual void eventTriggeredCallback(short events) = 0;

	virtual ~AbstractConnection() {}
};