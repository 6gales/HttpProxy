#pragma once
#include <netdb.h>
#include <vector>
#include <stdexcept>
#include <algorithm>

class FdRegistrar
{
public:
	enum class Options { Read, Write, Both };

private:
	fd_set readfs,
		writefs;

	std::vector<std::pair<int, Options>> subscribedFds;

public:
	FdRegistrar()
	{
		FD_ZERO(&readfs);
		FD_ZERO(&writefs);
	}

	void registerFd(int fd, Options option)
	{
		subscribedFds.push_back(std::make_pair(fd, option));
	}

	int selectFds()
	{
		int selected;
		do
		{
			FD_ZERO(&readfs);
			FD_ZERO(&writefs);

			int maxfd = -1;
			for (size_t i = 0; i < subscribedFds.size(); i++)
			{
				switch (subscribedFds[i].second)
				{
				case Options::Read:
					FD_SET(subscribedFds[i].first, &readfs);
					break;

				case Options::Write:
					FD_SET(subscribedFds[i].first, &writefs);
					break;

				default:
					FD_SET(subscribedFds[i].first, &readfs);
					FD_SET(subscribedFds[i].first, &writefs);
				}

				maxfd = std::max(maxfd, subscribedFds[i].first);
			}

			if (maxfd == -1)
				throw std::runtime_error("no file descriptors to select from");

			selected = select(maxfd + 1, &readfs, &writefs, NULL, NULL);

		} while (selected == 0);

		subscribedFds.clear();

		return selected;
	}

	bool isSetRead(int fd)
	{
		return FD_ISSET(fd, &readfs);
	}

	bool isSetWrite(int fd)
	{
		return FD_ISSET(fd, &readfs);
	}
};