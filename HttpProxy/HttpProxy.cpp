#include "HttpProxy.h"
#include <stdexcept>
#include <iostream>
#include <algorithm>
#include <string.h>

void HttpProxy::work(size_t id)
{
	std::cout << "Thread #" << id << " started" << std::endl;

	while (true)
	{
		datas[id]->waitWork();
		datas[id]->poll();
	}
}

void HttpProxy::run()
{
	std::cerr << "Proxy started" << std::endl;

	ThreadInfo threadInfos[threadNum - 1];
	for (size_t i = 0; i < threadNum - 1; i++)
	{
		threadInfos[i].id = i + 1;
		threadInfos[i].proxy = this;
		pthread_create(&threads[i], NULL, &HttpProxy::startThread, &threadInfos[i]);
	}

	work(0);
}