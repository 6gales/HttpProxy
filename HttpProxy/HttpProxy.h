#pragma once
#include <string>
#include <list>
#include <pthread.h>
#include "Connections/ServerSocket.h"
#include "Cache/Cache.h"
#include "WorkerThreadData.h"
#include "Utils/WorkerThreadLoadBalancer.hpp"

class HttpProxy
{
	Cache cache;

	const size_t threadNum;
	pthread_t *threads = nullptr;
	std::vector<WorkerThreadData *> datas;

	void work(size_t id);

public:
	enum Status
	{
		Running,
		GracefulShutdown,
		ForcedShutdown
	};

private:
	static Status proxyStatus;

	static void gracefulShutdownHandler(int signal);

	static void forcedShutdownHandler(int signal);

	struct ThreadInfo
	{
		size_t id;
		HttpProxy *proxy;
	};

	static void *startThread(void *threadInfo);

public:
	HttpProxy(short lport, size_t _threads) : threadNum(_threads), datas(_threads)
	{
		for (size_t i = 0; i < threadNum; i++)
		{
			datas[i] = new WorkerThreadData(cache);
		}

		datas[0]->enqueue(new ServerSocket(lport, new WorkerThreadLoadBalancer(datas)));

		threads = new pthread_t[threadNum - 1];
	}

	void run();

	~HttpProxy()
	{
		if (threadNum > 1)
		{
			for (size_t i = 0; i < threadNum; i++)
			{
				//				pthread_cancel
				//				pthread_join
			}
			delete[] threads;
		}

		for (size_t i = 0; i < threadNum; i++)
		{
			delete datas[i];
		}
	}
};