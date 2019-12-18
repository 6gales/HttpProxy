#pragma once
#include <sys/types.h>
#include <pthread.h>

template <class T>
class AtomicRefCounter
{
	pthread_mutex_t lock;
	size_t counter;
	T *ptr;

public:
	AtomicRefCounter(T *val)
	{
		ptr = val;
		pthread_mutex_init(&lock, NULL);
		counter = 1;
	}
	~AtomicRefCounter()
	{
		delete ptr;
		pthread_mutex_destroy(&lock);
	}

	size_t atomicIncrementAndGet();

	size_t atomicDecrementAndGet();

	size_t getCounter()
	{
		return counter;
	}

	T *getRef()
	{
		return ptr;
	}
};

template <class T>
class SharedPtr
{
	AtomicRefCounter<T> *refCounter;

public:
	SharedPtr()
	{
		refCounter = NULL;
	}
	SharedPtr(T *val)
	{
		refCounter = NULL;
		reset(val);
	}
	SharedPtr(const SharedPtr &another)
	{
		refCounter = another.refCounter;
		refCounter->atomicIncrementAndGet();
	}

	~SharedPtr()
	{
		reset(NULL);
	}

	T *get() const
	{
		return refCounter->getRef();
	}

	void reset(T *newPtr);

	T &operator*() const
	{
		return *refCounter->getRef();
	}
	T *operator->() const
	{
		return refCounter->getRef();
	}
};

template<class T>
void SharedPtr<T>::reset(T *newPtr)
{
	if (refCounter != NULL)
	{
		if (refCounter->atomicDecrementAndGet() == 0)
		{
			delete refCounter;
		}
		refCounter = NULL;
	}

	if (newPtr != NULL)
	{
		refCounter = new AtomicRefCounter<T>(newPtr);
	}
}

template<class T>
inline size_t AtomicRefCounter<T>::atomicIncrementAndGet()
{
	pthread_mutex_lock(&lock);
	size_t val = ++counter;
	pthread_mutex_unlock(&lock);

	return val;
}

template<class T>
size_t AtomicRefCounter<T>::atomicDecrementAndGet()
{
	pthread_mutex_lock(&lock);
	size_t val = --counter;
	pthread_mutex_unlock(&lock);

	return val;
}
