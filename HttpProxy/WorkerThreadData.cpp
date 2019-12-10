#include "WorkerThreadData.h"

size_t WorkerThreadData::getLodaing()
{
	return poller.connectionsSize();
}

void WorkerThreadData::enqueue(AbstractConnection* connection)
{
	poller.addConnection(connection);
}

void WorkerThreadData::enqueue(int fd)
{
	AbstractConnection* conn = new HttpRequest(fd, poller, cache);
	fprintf(stderr, "%x\n", conn);
	enqueue(conn);
}

void WorkerThreadData::waitWork()
{
	pthread_mutex_t* lock = poller.getLock();
	pthread_mutex_lock(lock);

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