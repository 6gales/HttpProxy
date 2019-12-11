#include "FdPoller.h"

void FdPoller::addConnection(AbstractConnection* connection)
{
	pthread_mutex_lock(&connectionLock);
	isChanged = true;

	if (inForEach)
	{
		insertList.push_back(connection);
	}
	else
	{
		auto elem = connections.find(connection->getFd());

		if (elem != connections.end())
		{
			delete elem->second;
			fprintf(stderr, "Deleting %x\n", elem->second);
			connections.erase(elem);
		}
		connections.emplace(connection->getFd(), connection);
	}
	pthread_mutex_unlock(&connectionLock);
}

void FdPoller::removeConnection(int sockFd)
{
	pthread_mutex_lock(&connectionLock);

	isChanged = true;
	if (inForEach)
	{
		deleteList.push_back(sockFd);
	}
	else
	{
		connections.erase(sockFd);
	}
	
	pthread_mutex_unlock(&connectionLock);
}

bool FdPoller::isConnectionFinished(int sockFd)
{
	pthread_mutex_lock(&connectionLock);

	auto it = connections.find(sockFd);
	if (it != connections.end())
	{
		pthread_mutex_unlock(&connectionLock);
		return it->second->isFinished();
	}

	pthread_mutex_unlock(&connectionLock);
	return true;
}

void FdPoller::emptyLists()
{
	for (size_t i = 0; i < deleteList.size(); i++)
	{
		removeConnection(deleteList[i]);
	}
	deleteList.clear();
	
	for (size_t i = 0; i < insertList.size(); i++)
	{
		addConnection(insertList[i]);
	}
	insertList.clear();
}

int FdPoller::pollFds()
{
	if (isChanged)
	{
		subscribedFds.clear();

		pthread_mutex_lock(&connectionLock);
		
		for (auto conn = connections.begin(); conn != connections.end(); ++conn)
		{
			struct pollfd poller;
			poller.fd = conn->first;
			poller.events = conn->second->getSubscribedEvents() | POLLERR | POLLHUP | POLLNVAL;
			subscribedFds.emplace_back(poller);
		}
		
		pthread_mutex_unlock(&connectionLock);
	}

	int polled;
	do
	{
		polled = poll(subscribedFds.data(), subscribedFds.size(), -1);
		fprintf(stderr, "Polled %d\n", polled);

	} while (polled == 0);

	if (polled > 0)
	{
		pthread_mutex_lock(&connectionLock);
		
		inForEach = true;

		for (size_t i = 0; i < subscribedFds.size(); i++)
		{
			if (subscribedFds[i].revents == 0)
			{
				continue;
			}

			auto it = connections.find(subscribedFds[i].fd);
			if (it != connections.end())
			{
				try
				{
					it->second->eventTriggeredCallback(subscribedFds[i].revents);
					if (it->second->isFinished())
					{
						fprintf(stderr, "Deleting in poll %x\n", it->second);
						delete it->second;
						connections.erase(it);
					}
				}
				catch (std::exception e)
				{
					fprintf(stderr, "Error occured: %s\n", e.what());
					delete it->second;
					connections.erase(it);
				}
			}
		}
		
		inForEach = false;
		emptyLists();

		pthread_mutex_unlock(&connectionLock);
	}

	return polled;
}

size_t FdPoller::connectionsSize()
{
	pthread_mutex_lock(&connectionLock);
	size_t size = connections.size();
	pthread_mutex_unlock(&connectionLock);

	return size;
}