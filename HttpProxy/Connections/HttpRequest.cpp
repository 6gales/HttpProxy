#include "HttpRequest.h"
#include <netinet/in.h>
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

void HttpRequest::suspendFromPoll()
{
}

void HttpRequest::restoreToPoll()
{
}

void HttpRequest::parseRequest()
{
	const char* path;
	const char* method;
	int minorVersion;
	size_t headerNum = 100;
	struct phr_header headers[headerNum];
	size_t methodLen,
		pathLen;

	phr_parse_request(request.c_str(), request.size(),
		&method, &methodLen,
		&path, &pathLen,
		&minorVersion, headers, &headerNum, request.size());

	//coonection header
	std::pair<std::string, int> urlPort;
	urlPort.second = -1;
	for (size_t i = 0; i < headerNum; i++)
	{
		if (!strncmp(headers[i].name, "Host",
			headers[i].name_len < 4 ? headers[i].name_len : 4))
		{
			urlPort = parseHost(std::string(headers[i].value, headers[i].value_len));
			break;
		}
	}
	//if no HOST param
	if (urlPort.second < 0)
	{
		manager.addConnection(new ErrorConnection(sockFd, "HTTP/1.0 404 Not Found\r\n\r\n"));
		return;
	}

	int servFd = openRedirectedSocket(urlPort.first, urlPort.second);

	if (minorVersion != 0 || !strncmp(method, "GET", 3))
	{
		ConnectionBuffer* clientToServ = new ConnectionBuffer(),
			* servToClient = new ConnectionBuffer();
		manager.addConnection(new DirectConnection(sockFd, clientToServ, servToClient, manager));
		manager.addConnection(new DirectConnection(servFd, servToClient, clientToServ, manager));
		//manager.addConnection(new ErrorConnection(sockFd, "HTTP/1.0 505 HTTP Version Not Supported\r\n\r\n"));
		return;
	}

	//	if (!strncmp(method, "GET", 3))
	//	{
	bool isInserted = cache.createIfNotExists(request);
	CacheEntry& entry = cache.getEntry(request);

	if (isInserted)
	{

		manager.addConnection(new CachingConnection(servFd, request, entry));
		manager.addConnection(new CachedConnection(sockFd, manager, entry));
		return;
	}
	else
	{
		manager.addConnection(new CachedConnection(sockFd, manager, entry));
		return;
	}
	//}
	//else
	//{
	//	fprintf(stderr, "Error connection 501\n");
	//	manager.addConnection(new ErrorConnection(sockFd, "HTTP/1.0 501 Not Implemented\r\n\r\n"));
	//	return;
	//}
}