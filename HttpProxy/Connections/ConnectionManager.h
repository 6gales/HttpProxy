#pragma once
#include "AbstractConnection.h"

class ConnectionManager
{
public:
	virtual void addConnection(AbstractConnection* connection) = 0;

	virtual void removeConnection(int sockFd) = 0;

	virtual bool isConnectionFinished(int sockFd) = 0;

	virtual ~ConnectionManager() {}
};