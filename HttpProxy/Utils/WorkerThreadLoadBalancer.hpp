#pragma once
#include <vector>
#include <algorithm>
#include <stdio.h>
#include "AbstractLoadBalancer.h"
#include "../WorkerThreadData.h"

class WorkerThreadLoadBalancer : public AbstractLoadBalancer
{
	const std::vector<WorkerThreadData*>& datas;

public:
	WorkerThreadLoadBalancer(const std::vector<WorkerThreadData*>& _datas)
		: datas(_datas) {}

	void addClient(int sockFd)
	{
		fprintf(stderr, "New client connected\n");

		size_t min = 0,
			minLoading = datas[0]->getLodaing();

		for (size_t i = 1; i < datas.size(); i++)
		{
			if (datas[i]->getLodaing() < minLoading)
			{
				min = i;
				minLoading = datas[i]->getLodaing();
			}
		}

		datas[min]->enqueue(sockFd);
	}

};