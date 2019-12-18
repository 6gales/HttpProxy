#include "HttpProxy.h"
#include <stdexcept>
#include <iostream>
#include <algorithm>
#include <string.h>
#include <signal.h>

extern HttpProxy::Status HttpProxy::proxyStatus;

void HttpProxy::work(size_t id)
{
	fprintf(stderr, "Thread #%lu started\n", id);

	while (proxyStatus == HttpProxy::Status::Running)
	{
		if (datas[id]->waitWork())
		{
			fprintf(stderr, "Thread #%lu polled %d\n", id, datas[id]->poll());
		}
	}

	datas[id]->shutdown();
	fprintf(stderr, "Graceful shutdown in thread #%lu\n", id);

	while (proxyStatus == HttpProxy::Status::GracefulShutdown && datas[id]->getLodaing() > 0)
	{
		fprintf(stderr, "Thread #%lu polled %d\n", id, datas[id]->poll());
	}
}

void HttpProxy::run()
{
	std::cerr << "Proxy started" << std::endl
		<< "Hit Ctrl^C for graceful shutdown" << std::endl
		<< "Hit twice to force shutdown" << std::endl;

	proxyStatus = HttpProxy::Status::Running;

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
		if (proxyStatus == HttpProxy::Status::ForcedShutdown)
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