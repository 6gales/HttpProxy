#pragma once
#include <pthread.h>
#include <vector>
#include <poll.h>
#include <map>
#include "Connections/AbstractConnection.h"
#include "Connections/ConnectionManager.h"

class FdPoller : public ConnectionManager
{
	pthread_mutex_t connectionLock;
	pthread_mutexattr_t recursiveAttr;

	std::map<int, AbstractConnection*> connections;
	std::vector<struct pollfd> subscribedFds;

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

	pthread_mutex_t* getLock() { return &connectionLock; }

	void addConnection(AbstractConnection* connection);

	void removeConnection(int sockFd);

	bool isConnectionFinished(int sockFd);

	int pollFds();

	~FdPoller()
	{
		pthread_mutexattr_destroy(&recursiveAttr);
		pthread_mutex_destroy(&connectionLock);

		for (std::map<int, AbstractConnection*>::iterator it = connections.begin(); it != connections.end(); ++it)
		{
			delete it->second;
		}
		connections.clear();
	}

	void subscriptionChanged()
	{
		isChanged = true;
	}
};