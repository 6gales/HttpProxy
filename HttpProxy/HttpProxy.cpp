#include "HttpProxy.h"
#include <stdexcept>
#include <iostream>
#include <algorithm>
#include <string.h>
#include <fcntl.h>
#include "InetUtils.h"

void HttpProxy::run()
{
	std::cerr << "Proxy started" << std::endl;

	while (true)
	{
		//if (contexts.size())
		//{
		//	registrar.registerFd(4, FdRegistrar::Options::Both);
		//}
		//registrar.registerFd(servSock.getFd(), FdRegistrar::Options::Read);
		//
		//if (registrar.selectFds() < 0)
		//{
		//	throw std::runtime_error(std::string("select: ") + strerror(errno));
		//}
		//
		//if (registrar.isSetRead(servSock.getFd()))
		//{
		//	fprintf(stderr, "New client connected\n");
		//	int clientSock = servSock.acceptConnection();

		//	contexts.push_back(new HttpContext(clientSock, registrar, cache));
		//}

		//if (contexts.size())
		//{
		//	fprintf(stderr, "r:%d w:%d\n", registrar.isSetRead(4), registrar.isSetWrite(4));
		//}

		//for (auto it = contexts.begin(); it != contexts.end(); ++it)
		//{
		//	(*it)->process();
		//	if ((*it)->isFinished())
		//	{
		//		delete *it;
		//		contexts.erase(it--);
		//	}
		//}
	}
}

void HttpProxy::acceptCallback(short events)
{
	if (events & POLLIN)
	{
		fprintf(stderr, "New client connected\n");
		int clientSock = servSock.acceptConnection();

		auto it = std::min_element(datas.begin(), datas.end(),
			[](WorkerThreadData* a, WorkerThreadData* b)
			{
				return a->getLodaing() < b->getLodaing();
			});

		(*it)->enqueue(clientSock);
	}
}