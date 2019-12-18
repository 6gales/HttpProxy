#pragma once
#include <memory>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include "ManagingConnection.h"
#include "../Utils/ConnectionBuffer.h"
#include "../Utils/SharedPtr.hpp"

class DirectConnection : public ManagingConnection
{
	SharedPtr<ConnectionBuffer> fromThis, toThis;

public:
	DirectConnection(int _sockFd,
					const SharedPtr<ConnectionBuffer> &_fromThis,
					const SharedPtr<ConnectionBuffer> &_toThis,
					ConnectionManager &manager)
					: ManagingConnection(_sockFd, manager),
					fromThis(_fromThis), toThis(_toThis)
	{
		subscribedEvents = POLLIN | POLLOUT;
		manager.subscriptionChanged();
	}

	void eventTriggeredCallback(short events);

	~DirectConnection()
	{
		fprintf(stderr, "Connection buffer destructor\n");
		close(sockFd);
	}
};