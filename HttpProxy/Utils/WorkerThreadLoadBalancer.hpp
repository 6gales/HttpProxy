#pragma once
#include <vector>
#include <algorithm>
#include <stdio.h>
#include "AbstractLoadBalancer.h"
#include "../WorkerThreadData.h"

class WorkerThreadLoadBalancer : public AbstractLoadBalancer
{
	const std::vector<WorkerThreadData*> &datas;

public:
	WorkerThreadLoadBalancer(const std::vector<WorkerThreadData*> &_datas)
		: datas(_datas) {}

	void addClient(int sockFd)
	{
		fprintf(stderr, "New client connected\n");

		size_t min = 0,
			minLoading = datas[0]->getLoading();
		for (size_t i = 0; i < datas.size(); i++)
		{
			size_t loading = datas[i]->getLoading();
			if (loading < minLoading)
			{
				min = i;
				minLoading = loading;
			}
		}

		datas[min]->enqueue(sockFd);
	}

};