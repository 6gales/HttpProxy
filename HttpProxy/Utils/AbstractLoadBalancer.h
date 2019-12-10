#pragma once

class AbstractLoadBalancer
{
public:
	virtual void addClient(int sockFd) = 0;
};