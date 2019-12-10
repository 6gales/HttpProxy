#pragma once
#include <vector>
#include <pthread.h>
#include <sys/types.h>
#include "../Connections/ManagingConnection.h"

class CacheEntry
{
	pthread_rwlock_t rwlock;

	bool isFull;
	std::vector<char> record;

	std::vector<ManagingConnection*> readers;

	void wakeUpConnections();

public:
	CacheEntry()
	{
		pthread_rwlock_init(&rwlock, NULL);
	}

	bool isCompleted();

	size_t recordSize();

	void waitData(ManagingConnection* reader);

	ssize_t writeToRecord(int sockFd);

	ssize_t readFromRecord(ManagingConnection* reader, size_t offset);

	~CacheEntry()
	{
		pthread_rwlock_destroy(&rwlock);
	}
};