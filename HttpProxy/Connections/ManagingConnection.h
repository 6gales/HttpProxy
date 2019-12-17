#pragma once
#include "AbstractConnection.h"
#include "ConnectionManager.h"

class ManagingConnection : public AbstractConnection
{
protected:
	ConnectionManager &manager;
public:
	ManagingConnection(int _sockFd, ConnectionManager &_manager)
		: AbstractConnection(_sockFd), manager(_manager) {}

	virtual void eventTriggeredCallback(short events) override = 0;

	void disableWrite()
	{
		//turn off writing ability
		subscribedEvents &= ~POLLOUT;
		manager.subscriptionChanged();
	}

	void enableWrite()
	{
		//restore writing ability
		subscribedEvents |= POLLOUT;
		manager.subscriptionChanged();
	}

	void disableRead()
	{
		subscribedEvents &= ~POLLIN;
		manager.subscriptionChanged();
	}

	void enableRead()
	{
		subscribedEvents |= POLLIN;
		manager.subscriptionChanged();
	}

	virtual ~ManagingConnection() {}
};