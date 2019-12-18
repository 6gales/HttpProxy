#include "ConnectionBuffer.h"
#include <errno.h>
#include <netdb.h>
#include <stdio.h>

ssize_t ConnectionBuffer::writeToRecord(ManagingConnection *writer)
{
	int wIndex = 0;
	size_t freeSpace = buffer.capacity() - writeOffset[wIndex];
	if (freeSpace == 0)
	{
		wIndex = 1;
		writeOffset[wIndex] = 0;
		freeSpace = readOffset;
		if (writeOffset[wIndex] == readOffset)
		{
			suspendWriter(writer);
			errno = EWOULDBLOCK;
			return -1;
		}
	}

	ssize_t bytesRead = recv(writer->getFd(), buffer.data() + writeOffset[wIndex], freeSpace, 0);

	if (bytesRead == -1)
	{
		return bytesRead;
	}

	writeOffset[wIndex] += bytesRead;

	if (bytesRead == 0)
	{
		isFull = true;
	}
	wakeUpReader();

	return bytesRead;
}

ssize_t ConnectionBuffer::readFromRecord(ManagingConnection *reader)
{
	if (writeOffset[0] == readOffset)
	{
		if (!isFull)
		{
			suspendReader(reader);
		}

		return 0;
	}

	ssize_t bytesWrote = send(reader->getFd(), buffer.data() + readOffset, writeOffset[0] - readOffset, 0);

	if (bytesWrote == -1)
	{

		return bytesWrote;
	}

	readOffset += bytesWrote;
	if (writeOffset[0] == buffer.capacity() && readOffset == buffer.capacity())
	{
		writeOffset[0] = writeOffset[1];
		writeOffset[1] = 0;
		readOffset = 0;
	}

	wakeUpWriter();

	return bytesWrote;
}

void ConnectionBuffer::wakeUpReader()
{
	if (waitingReader != NULL)
	{
		fprintf(stderr, "Waking up reader %x\n", waitingReader);
		waitingReader->enableWrite();
		waitingReader = NULL;
	}
}

void ConnectionBuffer::wakeUpWriter()
{
	if (waitingWriter != NULL)
	{
		fprintf(stderr, "Waking up writer %x\n", waitingWriter);
		waitingWriter->enableRead();
		waitingWriter = NULL;
	}
}

void ConnectionBuffer::suspendReader(ManagingConnection *reader)
{
	reader->disableWrite();
	waitingReader = reader;
}

void ConnectionBuffer::suspendWriter(ManagingConnection *writer)
{
	writer->disableRead();
	waitingWriter = writer;
}

bool ConnectionBuffer::isCompleted()
{
	return isFull && readOffset == writeOffset[0];
}