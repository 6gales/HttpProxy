#include "WorkerThreadData.h"

size_t WorkerThreadData::getLodaing()
{
	pthread_mutex_lock(&mutex);

	size_t size = poller.connectionsSize();

	pthread_mutex_unlock(&mutex);

	return size;
}

void WorkerThreadData::enqueue(AbstractConnection* connection)
{
	pthread_mutex_lock(&mutex);

	poller.addConnection(connection);

	pthread_mutex_unlock(&mutex);
}

void WorkerThreadData::enqueue(int fd)
{
	enqueue(new HttpRequest(fd, poller, cache));
}

void WorkerThreadData::waitWork()
{
	pthread_mutex_lock(&mutex);

	while (poller.connectionsSize() == 0)
	{
		pthread_cond_wait(&hasWork, &mutex);
	}
	pthread_mutex_unlock(&mutex);
}

int WorkerThreadData::poll()
{
	pthread_mutex_lock(&mutex);

	int polled = poller.pollFds(&mutex);

	pthread_mutex_unlock(&mutex);

	return polled;
}