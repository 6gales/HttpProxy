#pragma once
#include <unistd.h>
#include <sys/types.h>
#include "ManagingConnection.h"

#define BUFF_SIZE 4096

class DirectConnection;

class ConnectionBuffer
{
	bool isFull;
	char buffer[BUFF_SIZE];

	ManagingConnection* waitingReader;
	DirectConnection* waitingWriter;

	size_t writeOffset[2],
		readOffset;

	void wakeUpConnections();
	void waitData(ManagingConnection* reader);

public:
	ConnectionBuffer()
	{
		isFull = false;
		writeOffset[0] = 0;
		writeOffset[1] = 0;
		readOffset = 0;
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

	void eventTriggeredCallback(short events);

	void suspendFromPoll();
	
	void restoreToPoll();

	void disableRead();

	void enableRead();

	~DirectConnection()
	{
		delete fromThis;
		close(sockFd);
	}
};