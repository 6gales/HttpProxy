#pragma once
#include <vector>
#include "../HttpContext.h"

class ProcessingStatesUnion : public HttpContext::ProcessingState
{
	std::vector<ProcessingState*> states;

public:
	ProcessingStatesUnion(std::initializer_list<ProcessingState*> _states) : states(_states)
	{}

	ProcessingState* process(HttpContext* context) override
	{
		finished = true;
		for (size_t i = 0; i < states.size(); i++)
		{
			ProcessingState* newState = states[i]->process(context);
			if (newState != nullptr)
			{
				delete states[i];
				states[i] = newState;
			}

			finished = finished && states[i]->isFinished();
		}
		return nullptr;
	}

	~ProcessingStatesUnion()
	{
		for (size_t i = 0; i < states.size(); i++)
		{
			delete states[i];
			states[i] = nullptr;
		}
	}
};