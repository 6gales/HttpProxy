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
		try
		{
			ServerSocket *servSock = new ServerSocket(lport, new WorkerThreadLoadBalancer(datas));

			for (size_t i = 0; i < threadNum; i++)
			{
				datas[i] = new WorkerThreadData(cache);
			}

			datas[0]->enqueue(servSock);
		}
		catch (std::exception &e)
		{
			throw e;
		}
	}

	void run();

	~HttpProxy()
	{
		for (size_t i = 0; i < threadNum; i++)
		{
			delete datas[i];
		}
	}
};