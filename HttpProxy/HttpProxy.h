#pragma once
#include <string>
#include <list>
#include "WorkerThreadData.h"

class HttpProxy
{
	short lport;
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
	HttpProxy(short _lport, size_t _threads)
		: lport(_lport), threadNum(_threads), datas(_threads)
	{
		for (size_t i = 0; i < threadNum; i++)
		{
			datas[i] = new WorkerThreadData(cache);
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