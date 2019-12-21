#include "WorkerThreadData.h"
#include "Connections/HttpRequest.h"
#include <stdio.h>

size_t WorkerThreadData::getLoading()
{
	return poller.connectionsSize();
}

void WorkerThreadData::enqueue(AbstractConnection *connection)
{
	pthread_mutex_t* lock = poller.getLock();
	pthread_mutex_lock(lock);

	poller.addConnection(connection);
	pthread_cond_signal(&hasWork);

	pthread_mutex_unlock(lock);
}

void WorkerThreadData::enqueue(int fd)
{
	enqueue(new HttpRequest(fd, poller, cache));
}

bool WorkerThreadData::waitWork()
{
	pthread_mutex_t* lock = poller.getLock();
	pthread_mutex_lock(lock);

	while (poller.connectionsSize() == 0 && !isShutdowned)
	{
		pthread_cond_wait(&hasWork, lock);
	}

	pthread_mutex_unlock(lock);

	return !isShutdowned;
}

int WorkerThreadData::poll()
{
	return poller.pollFds();
}