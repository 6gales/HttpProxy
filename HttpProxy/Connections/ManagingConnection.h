#pragma once
#include "AbstractConnection.h"
#include "ConnectionManager.h"

class ManagingConnection : public AbstractConnection
{
protected:
	ConnectionManager& manager;
public:
	ManagingConnection(int _sockFd, ConnectionManager& _manager)
		: AbstractConnection(_sockFd), manager(_manager) {}

	virtual void eventTriggeredCallback(short events) override = 0;

	void suspendFromPoll()
	{
		manager.removeConnection(sockFd);
	}

	void restoreToPoll()
	{
		manager.addConnection(this);
	}

	virtual ~ManagingConnection() {}
};