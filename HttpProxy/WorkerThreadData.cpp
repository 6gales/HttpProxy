#include "WorkerThreadData.h"
#include <stdio.h>

size_t WorkerThreadData::getLodaing()
{
	return poller.connectionsSize();
}

void WorkerThreadData::enqueue(AbstractConnection* connection)
{
	pthread_mutex_t* lock = poller.getLock();
	pthread_mutex_lock(lock);

	poller.addConnection(connection);
	pthread_cond_signal(&hasWork);

	pthread_mutex_unlock(lock);
}

void WorkerThreadData::enqueue(int fd)
{
	AbstractConnection* conn = new HttpRequest(fd, poller, cache);
	fprintf(stderr, "%x\n", conn);
	enqueue(conn);
}

void WorkerThreadData::waitWork(size_t id)
{
	pthread_mutex_t* lock = poller.getLock();
	pthread_mutex_lock(lock);

	fprintf(stderr, "Thread #%lu, lock: %X\n", id, lock);
	while (poller.connectionsSize() == 0)
	{
		pthread_cond_wait(&hasWork, lock);
	}

	pthread_mutex_unlock(lock);
}

int WorkerThreadData::poll()
{
	return poller.pollFds();
}