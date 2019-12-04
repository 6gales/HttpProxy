#pragma once
#include <vector>
#include <poll.h>

class FdPoller
{
//	std::vector<struct pollfd> subscribedFds;
//
//public:
//	
//	void registerFd(int fd, short flags)
//	{
//		struct pollfd poller;
//		poller.fd = fd;
//		poller.events = flags;
//		subscribedFds.push_back(poller);
//	}
//
//	int selectFds()
//	{
//		int selected;
//		do
//		{
//			int maxfd = -1;
//			for (size_t i = 0; i < subscribedFds.size(); i++)
//			{
//				FD_SET(subscribedFds[i], &readfs);
//				FD_SET(subscribedFds[i], &writefs);
//				maxfd = std::max(maxfd, subscribedFds[i]);
//			}
//
//			if (maxfd == -1)
//				throw std::runtime_error("no file descriptors to select from");
//
//			selected = select(maxfd + 1, &readfs, &writefs, NULL, NULL);
//
//		} while (selected == 0);
//
//		subscribedFds.clear();
//
//		return selected;
//	}
//
//	bool isSetRead(int fd)
//	{
//		return FD_ISSET(fd, &readfs);
//	}
//
//	bool isSetWrite(int fd)
//	{
//		return FD_ISSET(fd, &readfs);
//	}
};