#pragma once

#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include "ManagingConnection.h"
#include "../Cache/Cache.h"

#define BUFF_SIZE 1024

class HttpRequest : public ManagingConnection
{
	unsigned char buffer[BUFF_SIZE];

	Cache &cache;
	bool readRequest,
		eof;

	std::string request;

	void parseRequest();

public:
	HttpRequest(int _clientFd, ConnectionManager &_manager, Cache &_cache)
		: ManagingConnection(_clientFd, _manager), cache(_cache)
	{
		readRequest = false;
		eof = false;
		subscribedEvents = POLLIN;
	}

	void eventTriggeredCallback(short events);

	~HttpRequest()
	{
		if ((eof && !readRequest) || closeForced)
		{
			fprintf(stderr, "close fd\n");
			close(sockFd);
		}
	}
};
