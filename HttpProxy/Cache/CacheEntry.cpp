#include "CacheEntry.h"
#include <netdb.h>
#include <fcntl.h>
#include <stdio.h>

ssize_t CacheEntry::writeToRecord(int sockFd)
{
	pthread_rwlock_wrlock(&rwlock);

	char buffer[1024000];
	ssize_t bytesRead = recv(sockFd, buffer, 1024000, 0);

	if (bytesRead > 0)
	{
		for (size_t i = 0; i < bytesRead; i++)
		{
			record.push_back(buffer[i]);
		}

		wakeUpConnections();
	}
	else if (bytesRead == 0)
	{
		isFull = true;
//		record.shrink_to_fit();
		wakeUpConnections();
	}

	pthread_rwlock_unlock(&rwlock);
	
	return bytesRead;
}

ssize_t CacheEntry::readFromRecord(ManagingConnection *reader, size_t offset)
{
	pthread_rwlock_rdlock(&rwlock);

	if (record.size() == offset)
	{
		if (!isFull)
		{
			waitData(reader);
		}
		pthread_rwlock_unlock(&rwlock);
		return 0;
	}

	ssize_t bytesWrote = send(reader->getFd(), record.data() + offset, record.size() - offset, 0);

	if (record.size() == offset + bytesWrote)
	{
		if (!isFull)
		{
			waitData(reader);
		}
		pthread_rwlock_unlock(&rwlock);
		return bytesWrote;
	}

	pthread_rwlock_unlock(&rwlock);

	return bytesWrote;
}

void CacheEntry::wakeUpConnections()
{
	for (size_t i = 0; i < readers.size(); i++)
	{
		readers[i]->enableWrite();
	}
	readers.clear();
}

void CacheEntry::waitData(ManagingConnection *reader)
{
	reader->disableWrite();
	readers.push_back(reader);
}

size_t CacheEntry::recordSize()
{
	pthread_rwlock_rdlock(&rwlock);

	size_t size = record.size();

	pthread_rwlock_unlock(&rwlock);

	return size;
}

bool CacheEntry::isCompleted()
{
	pthread_rwlock_rdlock(&rwlock);

	bool status = isFull;

	pthread_rwlock_unlock(&rwlock);

	return status;
}