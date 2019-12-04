#include "HttpRequest.h"
#include <netinet/in.h>
#include "EstablishedConnection.h"
#include "ProcessingStatesUnion.hpp"
#include "ErrorConnection.hpp"
#include "CachingConnection.h"
#include "CachedConnection.h"
#include "../InetUtils.h"
#include "../ThirdParty/picohttpparser.h"

HttpContext::ProcessingState* HttpRequest::process(HttpContext *context)
{
	if (context->getRegistrar().isSetRead(clientFd) && !eof)
	{
		ssize_t bytesRead = recv(clientFd, buffer, BUFF_SIZE, 0);
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
			if (request.capacity() < request.size() + bytesRead)
			{
				request.resize(request.size() + bytesRead);
			}

			for (size_t i = 0; i < bytesRead; i++)
			{
				request.push_back(buffer[i]);
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
		return parseRequest(context);
	}


	if (eof && !readRequest)
	{
		finished = true;
	}
	else
	{
		context->getRegistrar().registerFd(clientFd, FdRegistrar::Options::Read);
	}

	return nullptr;
}

HttpContext::ProcessingState* HttpRequest::parseRequest(HttpContext* context)
{
	const char* path;
	const char* method;
	int minorVersion;
	size_t headerNum = 100;
	struct phr_header headers[headerNum];
	size_t prevbuflen = 0,
		methodLen,
		pathLen;
	
	phr_parse_request(request.c_str(), request.size(),
		&method, &methodLen,
		&path, &pathLen,
		&minorVersion, headers, &headerNum, request.size());
	
	if (minorVersion != 0)
	{
		context->getRegistrar().registerFd(clientFd, FdRegistrar::Options::Write);
		return new ErrorConnection(clientFd, "HTTP/1.0 505 HTTP Version Not Supported\r\n\r\n");
	}

	if (!strncmp(method, "GET", 3))
	{
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
			context->getRegistrar().registerFd(clientFd, FdRegistrar::Options::Write);
			return new ErrorConnection(clientFd, "HTTP/1.0 404 Not Found\r\n\r\n");
		}

		std::pair<std::map<std::string, std::vector<char>>::iterator, bool> cacheEntry
			= context->getCahce().createIfNotExists(request);
		if (cacheEntry.second)
		{

			int servFd = openRedirectedSocket(urlPort.first, urlPort.second);

			context->getRegistrar().registerFd(clientFd, FdRegistrar::Options::Write);
			context->getRegistrar().registerFd(servFd, FdRegistrar::Options::Both);

			return new ProcessingStatesUnion({
				new CachingConnection(servFd, cacheEntry.first),
				new CachedConnection(clientFd, cacheEntry.first)
			});

		}
		else
		{
			context->getRegistrar().registerFd(clientFd, FdRegistrar::Options::Write);
			return new CachedConnection(clientFd, cacheEntry.first);
		}
	}
	else
	{
		fprintf(stderr, "Error connection 501\n");
		context->getRegistrar().registerFd(clientFd, FdRegistrar::Options::Write);
		return new ErrorConnection(clientFd, "HTTP/1.0 501 Not Implemented\r\n\r\n");
	}
}