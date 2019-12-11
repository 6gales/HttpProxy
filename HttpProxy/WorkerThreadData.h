#pragma once
#include <list>
#include <pthread.h>
#include "FdPoller.h"
#include "Connections/HttpRequest.h"

class WorkerThreadData
{
	FdPoller poller;
	Cache& cache;

	pthread_cond_t hasWork;
	
public:
	WorkerThreadData(Cache& _cache) : cache(_cache)
	{
		pthread_cond_init(&hasWork, NULL);
	}

	size_t getLodaing();

	void enqueue(AbstractConnection* connection);

	void enqueue(int fd);

	void waitWork(size_t id);

	int poll();

	~WorkerThreadData()
	{
		pthread_cond_destroy(&hasWork);
	}
};