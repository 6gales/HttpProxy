#include "ServerSocket.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdexcept>
#include <poll.h>
#include <errno.h>
#include "../Utils/InetUtils.h"

ServerSocket::ServerSocket(short port, AbstractLoadBalancer *_loadBalancer)
	: AbstractConnection(socket(AF_INET, SOCK_STREAM, 0)), loadBalancer(_loadBalancer)
{
	if (sockFd == -1)
	{
		throw std::runtime_error(std::string("socket: ") + strerror(errno));
	}
	
	int opt = 1;
	setsockopt(sockFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in listenaddr = getAddr("", port);

	if (bind(sockFd, reinterpret_cast<struct sockaddr *>(&listenaddr), sizeof(listenaddr)))
	{
		close(sockFd);
		throw std::runtime_error(std::string("bind: ") + strerror(errno));
	}

	if (listen(sockFd, SOMAXCONN))
	{
		close(sockFd);
		throw std::runtime_error(std::string("listen: ") + strerror(errno));
	}
	if (fcntl(sockFd, F_SETFL, fcntl(sockFd, F_GETFL, 0) | O_NONBLOCK) == -1)
	{
		close(sockFd);
		throw std::runtime_error("fcnt: cannot make server socket nonblock");
	}

	subscribedEvents = POLLIN;
}

int ServerSocket::acceptConnection() const
{
	sockaddr_in clientAddr;
	socklen_t addrlen = sizeof(clientAddr);
	int clientSock = accept(sockFd, reinterpret_cast<sockaddr*>(&clientAddr), &addrlen);
	if (clientSock < 0)
	{
		throw std::runtime_error(std::string("accept: ") + strerror(errno));
	}
	fcntl(clientSock, F_SETFL, fcntl(clientSock, F_GETFL, 0) | O_NONBLOCK);
	return clientSock;
}

void ServerSocket::eventTriggeredCallback(short events)
{
	if (canRead(events))
	{
		loadBalancer->addClient(acceptConnection());
	}
}