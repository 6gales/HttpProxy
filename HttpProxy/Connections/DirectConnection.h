#pragma once
#include <unistd.h>
#include <sys/types.h>
#include "ManagingConnection.h"
#include "../Cache/CacheEntry.h"

class DirectConnection;

class ConnectionBuffer
{
	constexpr static size_t BUFF_SIZE = 4096;

	bool isFull = false;
	char buffer[BUFF_SIZE];

	ManagingConnection* waitingReader;
	DirectConnection* waitingWriter;

	size_t writeOffset[2],
		readOffset = 0;

	void wakeUpConnections();
	void waitData(ManagingConnection* reader);

public:
	ConnectionBuffer()
	{
		writeOffset[0] = 0;
		writeOffset[1] = 0;
	}

	bool isCompleted();

	ssize_t writeToRecord(DirectConnection* writer);

	ssize_t readFromRecord(ManagingConnection* reader);
};

class DirectConnection : public ManagingConnection
{
	ConnectionBuffer *fromThis, *toThis;

public:
	DirectConnection(int _sockFd, ConnectionBuffer* _fromThis,
		ConnectionBuffer* _toThis, ConnectionManager& manager)
		: ManagingConnection(_sockFd, manager)
	{
		fromThis = _fromThis;
		toThis = _toThis;
		subscribedEvents = POLLIN | POLLOUT;
	}

	void eventTriggeredCallback(short events) override;

	void suspendFromPoll() override;
	
	void restoreToPoll() override;

	void disableRead();

	void enableRead();

	~DirectConnection()
	{
		delete fromThis;
		close(sockFd);
	}
};