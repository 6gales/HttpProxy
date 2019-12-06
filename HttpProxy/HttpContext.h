#pragma once
#include "FdRegistrar.hpp"
#include "Cache.h"

class HttpContext
{
public:
	class ProcessingState
	{
	protected:
		bool finished = false;

	public:
		virtual ProcessingState* process(HttpContext* context) = 0;

		virtual bool isFinished() { return finished; }

		virtual ~ProcessingState() {}
	};

private:
	ProcessingState* state;
	FdRegistrar& registrar;
	Cache& cache;

public:
	HttpContext(int sockfd, FdRegistrar& _registrar, Cache& _cache);

	void process()
	{
		ProcessingState* newState = state->process(this);
		if (newState != nullptr)
		{
			delete state;
			state = newState;
		}
	}

	bool isFinished() const { return state->isFinished(); }
	
	FdRegistrar& getRegistrar() const
	{
		return registrar;
	}

	Cache& getCahce() const
	{
		return cache;
	}
	
	~HttpContext()
	{
		delete state;
		state = nullptr;
	}
};

