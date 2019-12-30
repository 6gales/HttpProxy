#include "InetUtils.h"
#include <stdexcept>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

struct sockaddr_in getAddr(std::string host, short port)
{
	struct sockaddr_in sockAddr;
	struct addrinfo *addr = NULL;

	memset(&sockAddr, 0, sizeof(sockAddr));

	if (!host.empty())
	{
		struct addrinfo hints;
		memset(&hints, 0, sizeof(addrinfo));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		
		int errorCode = getaddrinfo(host.c_str(), NULL, &hints, &addr);
		if (errorCode != 0)
		{
			throw std::invalid_argument(std::string("getaddrinfo: ") + gai_strerror(errorCode));
		}
		memcpy(&sockAddr, addr->ai_addr, sizeof(struct sockaddr));
		freeaddrinfo(addr);
    }
	else
	{
		sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	}
	
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_port = htons(port);

	return sockAddr;
}

int openRedirectedSocket(std::string addr, short port)
{
	struct sockaddr_in redirectAddr;
	try
	{
		redirectAddr = getAddr(addr, port);
	}
	catch (std::exception & e)
	{
		throw e;
	}

	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1)
	{
		throw std::runtime_error("socket failed");
	}
	if (fcntl(sock, F_SETFL, fcntl(sock, F_GETFL, 0) | O_NONBLOCK) == -1)
	{
		close(sock);
		throw std::runtime_error("fcnt: cannot make server socket nonblock");
	}
	if (connect(sock, (struct sockaddr *) & redirectAddr, sizeof(redirectAddr)))
	{
		throw std::runtime_error("redirecting failed");
	}
	return sock;
}

std::pair<std::string, short> parseHost(std::string host)
{
	short port = 0;
	size_t pos = host.find(':', 0);
	std::string url = host.substr(0, pos);

	if (pos != std::string::npos)
	{
		for (size_t i = pos + 1; i < host.size(); i++)
		{
			port += host[i] - '0';
			port *= 10;
				
		}
		port /= 10;
	}
	else
	{
		port = 80;
	}
	
	return std::make_pair(url, port);
}