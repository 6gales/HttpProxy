#pragma once
#include <vector>
#include <pthread.h>
#include <sys/types.h>
#include "../Connections/ManagingConnection.h"
#include "../Utils/ConsistentVector.hpp"

class CacheEntry
{
	pthread_rwlock_t rwlock;

	bool isFull;
	bool hasWriter;
	ConsistentVector<char> record;

	std::vector<ManagingConnection*> readers;

	void wakeUpConnections();
	void waitData(ManagingConnection *reader);

public:
	CacheEntry() : record(1024000)
	{
		isFull = false;
		hasWriter = true;
		pthread_rwlock_init(&rwlock, NULL);
	}

	bool isCompleted();
	bool isInvalid();
	
	void setWriter(bool _hasWriter);

	size_t recordSize();

	ssize_t writeToRecord(int sockFd);

	ssize_t readFromRecord(ManagingConnection *reader, size_t offset);

	~CacheEntry()
	{
		pthread_rwlock_destroy(&rwlock);
	}
};