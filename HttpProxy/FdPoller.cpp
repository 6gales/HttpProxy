#include "FdPoller.h"
#include <stdio.h>
#include <iostream>
#include <vector>
#include <map>
#include <poll.h>
#include "Connections/AbstractConnection.h"
#include "Utils/ConsistentVector.hpp"

class FdPoller::FdPollerImpl
{
	pthread_mutex_t connectionLock;
	pthread_mutexattr_t recursiveAttr;

	std::map<int, AbstractConnection *> connections;
	ConsistentVector<struct pollfd> subscribedFds;

	std::vector<AbstractConnection *> insertList;
	std::vector<int> deleteList;

	bool isChanged;
	bool inForEach;

	void emptyLists();

public:
	FdPollerImpl();

	size_t connectionsSize();

	pthread_mutex_t *getLock();

	void gracefulShutdown();

	void addConnection(AbstractConnection *connection);

	void removeConnection(int sockFd);

	bool isConnectionFinished(int sockFd);

	int pollFds();

	void subscriptionChanged();

	~FdPollerImpl();
};

pthread_mutex_t *FdPoller::FdPollerImpl::getLock()
{
	return &connectionLock;
}

void FdPoller::FdPollerImpl::gracefulShutdown()
{
	pthread_mutex_lock(&connectionLock);

	for (std::map<int, AbstractConnection *>::iterator conn = connections.begin(); conn != connections.end(); ++conn)
	{
		fprintf(stderr, "Gracefuly finishing %x...\n", conn->second);
		conn->second->gracefulShutdown();
		if (conn->second->isFinished())
		{
			deleteList.push_back(conn->first);
			fprintf(stderr, "Gracefuly finished %x\n", conn->second);
			delete conn->second;
		}
	}

	for (size_t i = 0; i < deleteList.size(); i++)
	{
		connections.erase(deleteList[i]);
	}
	deleteList.clear();

	isChanged = true;
	pthread_mutex_unlock(&connectionLock);
}

void FdPoller::FdPollerImpl::addConnection(AbstractConnection *connection)
{
	pthread_mutex_lock(&connectionLock);
	isChanged = true;

	if (inForEach)
	{
		insertList.push_back(connection);
	}
	else
	{
		std::map<int, AbstractConnection*>::iterator elem = connections.find(connection->getFd());

		if (elem != connections.end())
		{
			fprintf(stderr, "Deleting %x\n", elem->second);
			delete elem->second;
			connections.erase(elem);
		}
		connections.insert(std::make_pair(connection->getFd(), connection));
	}
	pthread_mutex_unlock(&connectionLock);
}

void FdPoller::FdPollerImpl::removeConnection(int sockFd)
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

bool FdPoller::FdPollerImpl::isConnectionFinished(int sockFd)
{
	pthread_mutex_lock(&connectionLock);

	std::map<int, AbstractConnection*>::iterator it = connections.find(sockFd);
	if (it != connections.end())
	{
		pthread_mutex_unlock(&connectionLock);
		return it->second->isFinished();
	}

	pthread_mutex_unlock(&connectionLock);
	return true;
}

void FdPoller::FdPollerImpl::emptyLists()
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

int FdPoller::FdPollerImpl::pollFds()
{
	if (isChanged)
	{
		pthread_mutex_lock(&connectionLock);
		subscribedFds.clear();

		for (std::map<int, AbstractConnection*>::iterator conn = connections.begin(); conn != connections.end(); ++conn)
		{
			struct pollfd poller;
			poller.fd = conn->first;
			poller.events = conn->second->getSubscribedEvents();
			subscribedFds.push_back(poller);
		}
		
		pthread_mutex_unlock(&connectionLock);
	}

	int polled;
	do
	{
		polled = poll(subscribedFds.data(), subscribedFds.size(), -1);

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

			std::map<int, AbstractConnection*>::iterator it = connections.find(subscribedFds[i].fd);
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
				catch (std::exception &e)
				{
					std::cerr << "Error occured: " << e.what() << std::endl;
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

size_t FdPoller::FdPollerImpl::connectionsSize()
{
	pthread_mutex_lock(&connectionLock);
	size_t size = connections.size();
	pthread_mutex_unlock(&connectionLock);

	return size;
}

void FdPoller::FdPollerImpl::subscriptionChanged()
{
	isChanged = true;
}

FdPoller::FdPollerImpl::~FdPollerImpl()
{
	pthread_mutexattr_destroy(&recursiveAttr);
	pthread_mutex_destroy(&connectionLock);

	for (std::map<int, AbstractConnection *>::iterator it = connections.begin(); it != connections.end(); ++it)
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

FdPoller::FdPollerImpl::FdPollerImpl()
{
	inForEach = false;
	pthread_mutexattr_init(&recursiveAttr);
	pthread_mutexattr_settype(&recursiveAttr, PTHREAD_MUTEX_RECURSIVE);

	pthread_mutex_init(&connectionLock, &recursiveAttr);
}

pthread_mutex_t *FdPoller::getLock()
{
	return impl->getLock();
}

void FdPoller::gracefulShutdown()
{
	impl->gracefulShutdown();
}

void FdPoller::addConnection(AbstractConnection *connection)
{
	impl->addConnection(connection);
}

void FdPoller::removeConnection(int sockFd)
{
	impl->removeConnection(sockFd);
}

bool FdPoller::isConnectionFinished(int sockFd)
{
	return impl->isConnectionFinished(sockFd);
}

int FdPoller::pollFds()
{
	return impl->pollFds();
}

size_t FdPoller::connectionsSize()
{
	return impl->connectionsSize();
}

void FdPoller::subscriptionChanged()
{
	impl->subscriptionChanged();
}

FdPoller::~FdPoller()
{
	delete impl;
}

FdPoller::FdPoller()
{
	impl = new FdPollerImpl();
}
