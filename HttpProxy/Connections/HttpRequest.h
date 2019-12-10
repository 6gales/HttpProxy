#pragma once

#include <string>
#include <unistd.h>
#include <sys/types.h>
#include "ManagingConnection.h"
#include "../Cache/Cache.h"

class HttpRequest : public ManagingConnection
{
	constexpr static size_t BUFF_SIZE = 1024;
	unsigned char buffer[BUFF_SIZE];

	Cache& cache;
	bool readRequest = false,
		eof = false;
	
	std::string request;

	void parseRequest();

public:
	HttpRequest(int _clientFd, ConnectionManager& _manager, Cache& _cache)
		: ManagingConnection(_clientFd, _manager), cache(_cache)
	{
		subscribedEvents = POLLIN;
	}

	void eventTriggeredCallback(short events) override;

	~HttpRequest()
	{
		if (finished)
		{
			fprintf(stderr, "close fd\n");
			close(sockFd);
		}
	}
};
