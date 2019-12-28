#include "HttpProxy.h"
#include <stdexcept>
#include <iostream>
#include <algorithm>
#include <string.h>
#include <signal.h>
#include "Connections/ServerSocket.h"
#include "Utils/WorkerThreadLoadBalancer.hpp"

extern HttpProxy::Status HttpProxy::proxyStatus;

void HttpProxy::work(size_t id)
{
	fprintf(stderr, "Thread #%lu started\n", id);

	while (proxyStatus == Running)
	{
		if (datas[id]->waitWork())
		{
			//fprintf(stderr, "Thread #%lu polled %d\n", id,
			datas[id]->poll();//);
		}
	}

	datas[id]->shutdown();
	fprintf(stderr, "Graceful shutdown in thread #%lu\n", id);

	while (proxyStatus == GracefulShutdown && datas[id]->getLoading() > 0)
	{
		//fprintf(stderr, "Thread #%lu polled %d\n", id, 
		datas[id]->poll();//);
	}
}

void HttpProxy::run()
{
	std::cerr << "Starting proxy on port: " << lport << std::endl
		<< "Hit Ctrl^C for graceful shutdown" << std::endl
		<< "Hit twice to force shutdown" << std::endl;

	proxyStatus = Running;
	signal(SIGPIPE, SIG_IGN);

	try
	{
		ServerSocket *servSock = new ServerSocket(lport, new WorkerThreadLoadBalancer(datas));
		datas[0]->enqueue(servSock);
	}
	catch (std::exception & e)
	{
		throw e;
	}

	pthread_t *threads = new pthread_t[threadNum - 1];
	ThreadInfo threadInfos[threadNum - 1];

	for (size_t i = 0; i < threadNum - 1; i++)
	{
		threadInfos[i].id = i + 1;
		threadInfos[i].proxy = this;
		pthread_create(&threads[i], NULL, &HttpProxy::startThread, &threadInfos[i]);
	}

	signal(SIGINT, gracefulShutdownHandler);

	work(0);

	for (size_t i = 1; i < threadNum; i++)
	{
		datas[i]->shutdown();
	}

	for (ssize_t i = 0; i < threadNum - 1; i++)
	{
		if (proxyStatus == ForcedShutdown)
		{
			pthread_cancel(threads[i]);
		}
		if (pthread_join(threads[i], NULL) == -1)
		{
			i--;
		}
	}
	delete[] threads;
}

void *HttpProxy::startThread(void *threadInfo)
{
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGINT);
	pthread_sigmask(SIG_SETMASK, &set, NULL);

	ThreadInfo *info = reinterpret_cast<ThreadInfo *>(threadInfo);
	info->proxy->work(info->id);
	return NULL;
}

void HttpProxy::gracefulShutdownHandler(int signum)
{
	proxyStatus = GracefulShutdown;
	signal(SIGINT, forcedShutdownHandler);
}

void HttpProxy::forcedShutdownHandler(int signum)
{
	proxyStatus = ForcedShutdown;
	signal(SIGINT, SIG_DFL);
}