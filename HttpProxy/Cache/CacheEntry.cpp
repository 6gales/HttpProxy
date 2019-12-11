#include "CacheEntry.h"
#include <netdb.h>
#include <fcntl.h>
#include <stdio.h>

ssize_t CacheEntry::writeToRecord(int sockFd)
{
	pthread_rwlock_wrlock(&rwlock);

	char buffer[4096];
	ssize_t bytesRead = recv(sockFd, buffer, 4096, 0);

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

ssize_t CacheEntry::readFromRecord(ManagingConnection* reader, size_t offset)
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
	
	int r = fcntl(reader->getFd(), F_GETFD);
	fprintf(stderr, "Fcntl = %d\n", r);
	if (r == -1)
	{
		fprintf(stderr, "Sock fd is invalid");
		return 0;
	}

	ssize_t bytesWrote = send(reader->getFd(), record.data() + offset, record.size() - offset, MSG_NOSIGNAL);

	pthread_rwlock_unlock(&rwlock);

	return bytesWrote;
}

void CacheEntry::wakeUpConnections()
{
	for (auto it = readers.begin(); it != readers.end(); ++it)
	{
		(*it)->restoreToPoll();
	}
	readers.clear();
}

void CacheEntry::waitData(ManagingConnection* reader)
{
	reader->suspendFromPoll();
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