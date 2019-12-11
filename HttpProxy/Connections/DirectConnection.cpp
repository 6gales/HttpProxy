#include "DirectConnection.h"
#include <cstring>
#include <netdb.h>
#include <stdexcept>

void DirectConnection::eventTriggeredCallback(short events)
{
	if (canRead(events))
	{
		ssize_t bytesRead = fromThis->writeToRecord(this);
		if (bytesRead < 0 && errno != EWOULDBLOCK)
		{
			throw std::runtime_error(std::string("recv: ") + strerror(errno));
		}
		if (0 == bytesRead)
		{
			subscribedEvents = POLLOUT;
		}
	}
	if (canWrite(sockFd))// && buffer.hasData()))
	{
		ssize_t bytesWrote = toThis->readFromRecord(this);
		if (bytesWrote < 0 && errno != EWOULDBLOCK)
		{
			throw std::runtime_error(std::string("send: ") + strerror(errno));
		}
	}

	if (fromThis->isCompleted() && toThis->isCompleted())
	{
		finished = true;
	}
}

void DirectConnection::suspendFromPoll()
{
	//turn off writing ability
	subscribedEvents &= ~POLLOUT;
	manager.subscriptionChanged();
}

void DirectConnection::restoreToPoll()
{
	//restore writing ability
	subscribedEvents |= POLLOUT;
	manager.subscriptionChanged();
}

void DirectConnection::disableRead()
{
	subscribedEvents &= ~POLLIN;
	manager.subscriptionChanged();
}

void DirectConnection::enableRead()
{
	subscribedEvents |= POLLIN;
	manager.subscriptionChanged();
}

ssize_t ConnectionBuffer::writeToRecord(DirectConnection* writer)
{
	int wIndex = 0;
	size_t freeSpace = BUFF_SIZE - writeOffset[wIndex];
	if (writeOffset[wIndex] == BUFF_SIZE)
	{
		wIndex = 1;
		writeOffset[wIndex] = 0;
		freeSpace = readOffset;
		if (writeOffset[wIndex] == readOffset)
		{
			writer->disableRead();
			waitingWriter = writer;
			errno = EWOULDBLOCK;
			return -1;
		}
	}

	ssize_t bytesRead = recv(writer->getFd(), buffer + writeOffset[wIndex], freeSpace, 0);

	if (bytesRead == -1)
	{
		return bytesRead;
	}
	
	writeOffset[wIndex] += bytesRead;

	if (bytesRead == 0)
	{
		isFull = true;
	}
	wakeUpConnections();

	return bytesRead;
}

ssize_t ConnectionBuffer::readFromRecord(ManagingConnection* reader)
{
	if (writeOffset[0] == readOffset)
	{
		if (!isFull)
		{
			waitData(reader);
		}
		
		return 0;
	}

	ssize_t bytesWrote = send(reader->getFd(), buffer + readOffset, writeOffset[0] - readOffset, 0);

	if (bytesWrote == -1)
	{
		return bytesWrote;
	}

	if (writeOffset[0] == BUFF_SIZE && readOffset == BUFF_SIZE)
	{
		writeOffset[0] = writeOffset[1];
		readOffset = 0;
	}

	if (waitingWriter != nullptr)
	{
		waitingWriter->enableRead();
		waitingWriter = nullptr;
	}

	return bytesWrote;
}

void ConnectionBuffer::wakeUpConnections()
{
	if (waitingReader != nullptr)
	{
		waitingReader->restoreToPoll();
		waitingReader = nullptr;
	}
}

void ConnectionBuffer::waitData(ManagingConnection* reader)
{
	reader->suspendFromPoll();
	waitingReader = reader;
}

bool ConnectionBuffer::isCompleted()
{
	return isFull && readOffset == writeOffset[0];
}