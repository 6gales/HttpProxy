#pragma once
#include <string>
#include <netdb.h>
#include <list>
#include "ServerSocket.h"
#include "FdRegistrar.hpp"
#include "Cache.h"
#include "HttpContext.h"
#include "WorkerThreadData.h"
#include <pthread.h>

class HttpProxy
{
	const ServerSocket servSock;

	FdRegistrar registrar;
	Cache cache;

	const size_t threadNum;
	pthread_t* threads = nullptr;
	std::vector<WorkerThreadData*> datas;

public:
	HttpProxy(int lport, size_t _threads) : servSock(lport), threadNum(_threads), datas(_threads)
	{
		for (size_t i = 0; i < threadNum; i++)
		{
			datas.push_back(new WorkerThreadData());
		}
	}

	void run();

	void acceptCallback(short events);

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