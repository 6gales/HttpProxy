#include "HttpRequest.h"
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <netinet/in.h>
#include "../Cache/Cache.h"
#include "DirectConnection.h"
#include "ErrorConnection.hpp"
#include "CachingConnection.h"
#include "CachedConnection.h"
#include "../Utils/InetUtils.h"
#include "../ThirdParty/picohttpparser.h"

void HttpRequest::eventTriggeredCallback(short events)
{
	if (canRead(events) && !eof)
	{
		ssize_t bytesRead = recv(sockFd, buffer, BUFF_SIZE, 0);
		if (bytesRead < 0)
		{
			throw std::runtime_error(std::string("recv: ") + strerror(errno));
		}
		if (bytesRead == 0)
		{
			eof = true;
		}
		else
		{
			size_t previousSize = request.size();
			if (request.capacity() < previousSize + bytesRead)
			{
				request.resize(previousSize + bytesRead);
			}

			for (size_t i = 0; i < bytesRead; i++)
			{
				request[i + previousSize] = buffer[i];
			}

			if (request.size() >= 4)
			{
				size_t size = request.size();
				if (request[size - 4] == '\r' && request[size - 3] == '\n'
					&& request[size - 2] == '\r' && request[size - 1] == '\n')
				{
					readRequest = true;
				}
			}
		}
	}

	if (readRequest)
	{
		parseRequest();
		finished = true;
	}


	if (eof && !readRequest)
	{
		finished = true;
	}
}

void HttpRequest::parseRequest()
{
	const char *path;
	const char *method;
	int minorVersion;
	size_t headerNum = 100;
	struct phr_header headers[headerNum];
	size_t methodLen,
		pathLen;

	phr_parse_request(request.c_str(), request.size(),
		&method, &methodLen,
		&path, &pathLen,
		&minorVersion, headers, &headerNum, request.size());

	std::pair<std::string, short> urlPort;
	urlPort.second = -1;

	bool connectionInserted = false;
	std::string strMethod = std::string(method, methodLen);
	std::string http0Request = strMethod + " " + std::string(path, pathLen) + " HTTP/1.0\r\n";
	for (size_t i = 0; i < headerNum; i++)
	{
		std::string header(headers[i].name, headers[i].name_len);
		std::string value(headers[i].value, headers[i].value_len);

		if (header == "Host")
		{
			urlPort = parseHost(value);
		}
		else if (header == "Connection" || header == "Proxy-Connection")
		{
			if (connectionInserted)
			{
				continue;
			}

			header = "Connection";
			value = "close";
			connectionInserted = true;
		}

		http0Request += header + ": " + value + "\r\n";
	}
	http0Request += "\r\n";

	//if no HOST param
	if (urlPort.second < 0)
	{
		manager.addConnection(new ErrorConnection(sockFd, "HTTP/1.0 400 Bad Request\r\n\r\n"));
		return;
	}

	int servFd = -1;
	bool isGet = strMethod == "GET";
	bool isRedirectNeeded = !isGet;
	bool isInserted = false;
	CacheEntry *entry = NULL;

	if (isGet)
	{
		isInserted = cache.createIfNotExists(http0Request);
		isRedirectNeeded = isInserted;
		entry = cache.getEntry(http0Request);
	}

	if (isRedirectNeeded)
	{
		try
		{
			servFd = openRedirectedSocket(urlPort.first, urlPort.second);
		}
		catch (std::exception &e)
		{
			fprintf(stderr, "Error occured: %s\n", e.what());
			manager.addConnection(new ErrorConnection(sockFd, "HTTP/1.0 404 Not Found\r\n\r\n"));
			return;
		}
	}

	if (!isGet)
	{
		SharedPtr<ConnectionBuffer> clientToServ(new ConnectionBuffer(http0Request)),
			servToClient(new ConnectionBuffer());
		manager.addConnection(new DirectConnection(sockFd, clientToServ, servToClient, manager));
		manager.addConnection(new DirectConnection(servFd, servToClient, clientToServ, manager));
		return;
	}

	if (isInserted)
	{
		manager.addConnection(new CachingConnection(servFd, http0Request, entry));
	}
	manager.addConnection(new CachedConnection(sockFd, manager, entry));
	return;
}

HttpRequest::HttpRequest(int _clientFd, ConnectionManager &_manager, Cache &_cache)
	: ManagingConnection(_clientFd, _manager), cache(_cache)
{
	readRequest = false;
	eof = false;
	subscribedEvents = POLLIN;
}

HttpRequest::~HttpRequest()
{
	if ((eof && !readRequest) || closeForced)
	{
		fprintf(stderr, "close fd\n");
		close(sockFd);
	}
}