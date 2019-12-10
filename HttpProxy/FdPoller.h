#pragma once
#include <vector>
#include <poll.h>
#include <unordered_map>
#include "Connections/AbstractConnection.h"
#include "Connections/ConnectionManager.h"

class FdPoller : public ConnectionManager
{
	std::unordered_map<int, AbstractConnection*> connections;
	std::vector<struct pollfd> subscribedFds;
	bool isChanged;

public:
	size_t connectionsSize() { return connections.size(); }

	void addConnection(AbstractConnection* connection) override;

	void removeConnection(int sockFd) override;

	bool isConnectionFinished(int sockFd) override;

	int pollFds(pthread_mutex_t* unlockOnPoll);
};