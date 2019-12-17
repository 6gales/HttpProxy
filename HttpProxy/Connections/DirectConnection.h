#pragma once
#include <memory>
#include <unistd.h>
#include <sys/types.h>
#include "ManagingConnection.h"
#include "../Utils/ConnectionBuffer.h"

class DirectConnection : public ManagingConnection
{
	std::shared_ptr<ConnectionBuffer> fromThis, toThis;

public:
	DirectConnection(int _sockFd,
					std::shared_ptr<ConnectionBuffer> _fromThis,
					std::shared_ptr<ConnectionBuffer> _toThis,
					ConnectionManager &manager)
					: ManagingConnection(_sockFd, manager),
					fromThis(_fromThis), toThis(_toThis)
	{
		subscribedEvents = POLLIN | POLLOUT;
		manager.subscriptionChanged();
	}

	void eventTriggeredCallback(short events) override;

	~DirectConnection()
	{
		fprintf(stderr, "Connection buffer destructor\n");
		close(sockFd);
	}
};