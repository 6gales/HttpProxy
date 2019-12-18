#pragma once
#include <unistd.h>
#include "AbstractConnection.h"
#include "../Utils/AbstractLoadBalancer.h"

class ServerSocket : public AbstractConnection
{
	AbstractLoadBalancer *loadBalancer;

	int acceptConnection() const;

public:
	ServerSocket(short port, AbstractLoadBalancer *loadBalancer);

	void eventTriggeredCallback(short events);

	void gracefulShutdown()
	{
		finished = true;
	}

	~ServerSocket()
	{
		close(sockFd);
		delete loadBalancer;
	}
};