#include "FdPoller.h"

void FdPoller::addConnection(AbstractConnection* connection)
{
	isChanged = true;
	connections.emplace(connection->getFd(), connection);
}

void FdPoller::removeConnection(int sockFd)
{
	isChanged = true;
	connections.erase(sockFd);
}

bool FdPoller::isConnectionFinished(int sockFd)
{
	auto it = connections.find(sockFd);
	if (it != connections.end())
	{
		return it->second->isFinished();
	}
	return true;
}

int FdPoller::pollFds(pthread_mutex_t* unlockOnPoll)
{
	if (isChanged)
	{
		subscribedFds.clear();
		for (auto conn = connections.begin(); conn != connections.end(); ++conn)
		{
			struct pollfd poller;
			poller.fd = conn->first;
			poller.events = conn->second->getSubscribedEvents();//subscribed events
			subscribedFds.emplace_back(poller);
		}
	}

	int polled;
	do
	{
		pthread_mutex_unlock(unlockOnPoll);
		polled = poll(subscribedFds.data(), subscribedFds.size(), -1);
		pthread_mutex_lock(unlockOnPoll);

	} while (polled == 0);

	if (polled > 0)
	{
		for (size_t i = 0; i < subscribedFds.size(); i++)
		{
			auto it = connections.find(subscribedFds[i].fd);
			if (it != connections.end())
			{
				it->second->eventTriggeredCallback(subscribedFds[i].revents);
			}
		}
	}

	return polled;
}