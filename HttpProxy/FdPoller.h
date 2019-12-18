#pragma once
#include <vector>
#include <poll.h>
#include <unordered_map>
#include "Connections/AbstractConnection.h"
#include "Connections/ConnectionManager.h"

class FdPoller : public ConnectionManager
{
	pthread_mutex_t connectionLock;
	pthread_mutexattr_t recursiveAttr;

	std::unordered_map<int, AbstractConnection*> connections;
	std::vector<struct pollfd> subscribedFds;

	std::vector<AbstractConnection*> insertList;
	std::vector<int> deleteList;

	bool isChanged;
	bool inForEach = false;

	void emptyLists();

public:
	FdPoller()
	{
		pthread_mutexattr_init(&recursiveAttr);
		pthread_mutexattr_settype(&recursiveAttr, PTHREAD_MUTEX_RECURSIVE);

		pthread_mutex_init(&connectionLock, &recursiveAttr);
	}

	size_t connectionsSize();

	pthread_mutex_t *getLock() { return &connectionLock; }

	void gracefulShutdown();

	void addConnection(AbstractConnection *connection) override;

	void removeConnection(int sockFd) override;

	bool isConnectionFinished(int sockFd) override;

	int pollFds();

	void subscriptionChanged() override
	{
		isChanged = true;
	}

	~FdPoller()
	{
		pthread_mutexattr_destroy(&recursiveAttr);
		pthread_mutex_destroy(&connectionLock);

		for (auto it = connections.begin(); it != connections.end(); ++it)
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