#pragma once
#include <vector>
#include <map>
#include <pthread.h>
#include <poll.h>
#include "Connections/AbstractConnection.h"
#include "Connections/ConnectionManager.h"
#include "Utils/ConsistentVector.hpp"

class FdPoller : public ConnectionManager
{
	pthread_mutex_t connectionLock;
	pthread_mutexattr_t recursiveAttr;

	std::map<int, AbstractConnection*> connections;
	ConsistentVector<struct pollfd> subscribedFds;

	std::vector<AbstractConnection*> insertList;
	std::vector<int> deleteList;

	bool isChanged;
	bool inForEach;

	void emptyLists();

public:
	FdPoller();

	size_t connectionsSize();

	pthread_mutex_t *getLock() { return &connectionLock; }

	void gracefulShutdown();

	void addConnection(AbstractConnection *connection);

	void removeConnection(int sockFd);

	bool isConnectionFinished(int sockFd);

	int pollFds();

	void subscriptionChanged();

	~FdPoller();
};
