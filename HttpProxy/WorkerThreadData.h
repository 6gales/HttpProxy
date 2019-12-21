#pragma once
#include <list>
#include <pthread.h>
#include "Cache/Cache.h"
#include "FdPoller.h"

class WorkerThreadData
{
	FdPoller poller;
	Cache &cache;
	bool isShutdowned;

	pthread_cond_t hasWork;

public:
	WorkerThreadData(Cache &_cache) : cache(_cache)
	{
		isShutdowned = false;
		pthread_cond_init(&hasWork, NULL);
	}

	size_t getLoading();

	void shutdown()
	{
		pthread_mutex_lock(poller.getLock());
		isShutdowned = true;
		poller.gracefulShutdown();
		pthread_cond_signal(&hasWork);

		pthread_mutex_unlock(poller.getLock());
	}

	void enqueue(AbstractConnection *connection);

	void enqueue(int fd);

	bool waitWork();

	int poll();

	~WorkerThreadData()
	{
		pthread_cond_destroy(&hasWork);
	}
};