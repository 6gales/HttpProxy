#pragma once
#include <pthread.h>
#include <vector>
#include <poll.h>
#include <map>
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
	FdPoller()
	{
		inForEach = false;
		pthread_mutexattr_init(&recursiveAttr);
		pthread_mutexattr_settype(&recursiveAttr, PTHREAD_MUTEX_RECURSIVE);

		pthread_mutex_init(&connectionLock, &recursiveAttr);
	}

	size_t connectionsSize();

	pthread_mutex_t *getLock() { return &connectionLock; }

	void gracefulShutdown();

	void addConnection(AbstractConnection *connection);

	void removeConnection(int sockFd);

	bool isConnectionFinished(int sockFd);

	int pollFds();

	void subscriptionChanged()
	{
		isChanged = true;
	}

	~FdPoller()
	{
		pthread_mutexattr_destroy(&recursiveAttr);
		pthread_mutex_destroy(&connectionLock);

		for (std::map<int, AbstractConnection*>::iterator it = connections.begin(); it != connections.end(); ++it)
		{
			it->second->forceClose();
			delete it->second;
		}
		connections.clear();

		for (size_t i = 0; i < insertList.size(); i++)
		{
			insertList[i]->forceClose();
			delete insertList[i];
		}
	}
};