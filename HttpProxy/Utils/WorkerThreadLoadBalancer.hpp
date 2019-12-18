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

		auto it = std::min_element(datas.begin(), datas.end(),
			[](WorkerThreadData *a, WorkerThreadData *b)
			{
				min = i;
				minLoading = datas[i]->getLodaing();
			}
		}

		datas[min]->enqueue(sockFd);
	}

};