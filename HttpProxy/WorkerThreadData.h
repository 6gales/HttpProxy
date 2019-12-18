#pragma once
#include <list>
#include <unordered_set>
#include <pthread.h>
#include "FdPoller.h"
#include "Connections/HttpRequest.h"

class WorkerThreadData
{
	FdPoller poller;
	Cache &cache;
	bool isShutdowned = false;

	pthread_cond_t hasWork;

public:
	WorkerThreadData(Cache &_cache) : cache(_cache)
	{
		pthread_cond_init(&hasWork, NULL);
	}

	size_t getLodaing();

	void shutdown()
	{
		pthread_mutex_lock(poller.getLock());
		isShutdowned = true;
		poller.gracefulShutdown();
		pthread_cond_signal(&hasWork);
		fprintf(stderr, "Gracefuly s\n");

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