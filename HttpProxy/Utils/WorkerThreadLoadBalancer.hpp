#pragma once
#include <vector>
#include <algorithm>
#include "AbstractLoadBalancer.h"
#include "../WorkerThreadData.h"

class WorkerThreadLoadBalancer : public AbstractLoadBalancer
{
	const std::vector<WorkerThreadData*> &datas;

public:
	WorkerThreadLoadBalancer(const std::vector<WorkerThreadData*> &_datas)
		: datas(_datas) {}

	void addClient(int sockFd) override
	{
		fprintf(stderr, "New client connected\n");

		auto it = std::min_element(datas.begin(), datas.end(),
			[](WorkerThreadData *a, WorkerThreadData *b)
			{
				return a->getLodaing() < b->getLodaing();
			});

		(*it)->enqueue(sockFd);
	}

};