#pragma once
#include <list>
#include <unordered_set>
#include <pthread.h>
#include "FdPoller.h"
#include "Connections/HttpRequest.h"

class WorkerThreadData
{
	FdPoller poller;
	Cache& cache;

	pthread_cond_t hasWork;
	pthread_mutexattr_t recursiveAttr;
	pthread_mutex_t mutex;

public:
	WorkerThreadData(Cache& _cache) : cache(_cache)
	{
		pthread_mutexattr_init(&recursiveAttr);
		pthread_mutexattr_settype(&recursiveAttr, PTHREAD_MUTEX_RECURSIVE);

		pthread_mutex_init(&mutex, &recursiveAttr);

		pthread_cond_init(&hasWork, NULL);
	}

	size_t getLodaing();

	void enqueue(AbstractConnection* connection);

	void enqueue(int fd);

	void waitWork();

	int poll();

	~WorkerThreadData()
	{
		pthread_mutexattr_destroy(&recursiveAttr);
		pthread_mutex_destroy(&mutex);
		pthread_cond_destroy(&hasWork);
	}
};