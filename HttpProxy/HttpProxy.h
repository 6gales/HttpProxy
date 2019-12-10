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
	pthread_t* threads = nullptr;
	std::vector<WorkerThreadData*> datas;

	void work(size_t id);

	struct ThreadInfo
	{
		size_t id;
		HttpProxy* proxy;
	};

	static void* startThread(void* threadInfo)
	{
		ThreadInfo* info = reinterpret_cast<ThreadInfo*>(threadInfo);
		info->proxy->work(info->id);
		return NULL;
	}

public:
	HttpProxy(int lport, size_t _threads) : datas(_threads), threadNum(_threads)
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