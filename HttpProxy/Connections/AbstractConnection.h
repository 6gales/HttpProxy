#pragma once
#include <poll.h>

class AbstractConnection
{
protected:
	const int sockFd;
	bool closeForced = false;
	bool finished = false;
	short subscribedEvents = 0;

	bool canRead(short events)
	{
		return events & POLLIN;
	}

	bool canWrite(short events)
	{
		return events & POLLOUT;
	}

	bool isHangedUp(short events)
	{
		return events & POLLHUP;
	}

	bool isErrorOccuerd(short events)
	{
		return events & POLLERR;
	}

	bool isInvalid(short events)
	{
		return events & POLLNVAL;
	}

public:
	AbstractConnection(int _sockFd) : sockFd(_sockFd) {}

	short getSubscribedEvents() { return subscribedEvents; }

	int getFd() const { return sockFd; }

	bool isFinished() const { return finished; }

	void forceClose() { closeForced = true; }

	virtual void gracefulShutdown() {}

	virtual void eventTriggeredCallback(short events) = 0;

	virtual ~AbstractConnection() {}
};