#pragma once
#include <pthread.h>
#include "Connections/ConnectionManager.h"

class FdPoller : public ConnectionManager
{
	class FdPollerImpl;
	FdPollerImpl *impl;

public:
	FdPoller();

	size_t connectionsSize();

	pthread_mutex_t *getLock();

	void gracefulShutdown();

	void addConnection(AbstractConnection *connection);

	void removeConnection(int sockFd);

	bool isConnectionFinished(int sockFd);

	int pollFds();

	void subscriptionChanged();

	~FdPoller();
};
