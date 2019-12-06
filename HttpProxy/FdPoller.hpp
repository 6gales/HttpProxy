#pragma once
#include <vector>
#include <poll.h>
#include <map>
#include <functional>

class FdPoller
{
	std::vector<struct pollfd> subscribedFds;

public:
	
	void registerFd(int fd, short flags)
	{
		struct pollfd poller;
		poller.fd = fd;
		poller.events = flags;
		subscribedFds.push_back(poller);
	}

	void removeFd(int fd)
	{
		for (size_t i = 0; i < subscribedFds.size(); i++)
		{
			if (subscribedFds[i].fd == fd)
			{
				subscribedFds[i] = subscribedFds[subscribedFds.size() - 1];
				subscribedFds.pop_back();
				break;
			}
		}
	}

	int pollFds(std::unordered_map<int, std::function<void(short)>> &callbacks)
	{
		int polled;
		do
		{
			polled = poll(subscribedFds.data(), subscribedFds.size(), NULL);

		} while (polled == 0);

		if (polled > 0)
		{
			for (size_t i = 0; i < subscribedFds.size(); i++)
			{
				auto it = callbacks.find(subscribedFds[i].fd);
				if (it != callbacks.end())
				{
					it->second(subscribedFds[i].revents);
				}
			}
		}

//		subscribedFds.clear();

		return polled;
	}
};