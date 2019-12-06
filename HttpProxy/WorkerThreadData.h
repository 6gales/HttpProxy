#pragma once
#include <list>
#include <unordered_set>
#include <pthread.h>
#include "FdPoller.hpp"
#include "HttpContext.h"


class WorkerThreadData
{
	std::unordered_set<int> fds;
	FdPoller poller;
	std::list<HttpContext*> contexts;

	pthread_cond_t hasWork;
	pthread_mutex_t mutex;

public:
	WorkerThreadData()
	{
		pthread_mutex_init(&mutex, NULL);
		pthread_cond_init(&hasWork, NULL);
	}

	size_t getLodaing()
	{
		return fds.size();
	}

	void enqueue(int fd)
	{
		fds.insert(fd);
	}

	void waitWork()
	{
		pthread_mutex_lock(&mutex);
		
		while (fds.size() == 0)
		{
			pthread_cond_wait(&hasWork, &mutex);
		}

		pthread_mutex_unlock(&mutex);
	}

	~WorkerThreadData()
	{
		pthread_mutex_destroy(&mutex);
		pthread_cond_destroy(&hasWork);

		for (auto it = contexts.begin(); it != contexts.end(); ++it)
		{
			delete *it;
			*it = nullptr;
			contexts.erase(it--);
		}
	}
};