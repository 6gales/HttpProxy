#pragma once
#include <string>
#include "ManagingConnection.h"

#define BUFF_SIZE 1024

class Cache;

class HttpRequest : public ManagingConnection
{
	unsigned char buffer[BUFF_SIZE];

	Cache &cache;
	bool readRequest,
		eof;

	std::string request;

	void parseRequest();

public:
	HttpRequest(int _clientFd, ConnectionManager &_manager, Cache &_cache);

	void eventTriggeredCallback(short events);

	~HttpRequest();
};
