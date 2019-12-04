#pragma once
#include <string>
#include <netdb.h>
#include <list>
#include "ServerSocket.h"
#include "FdRegistrar.hpp"
#include "Cache.h"
#include "HttpContext.h"

class HttpProxy
{
	const ServerSocket servSock;

	FdRegistrar registrar;
	Cache cache;

	std::list<HttpContext*> contexts;

public:
	HttpProxy(int lport, int threads) : servSock(lport) {}

	void run();
};