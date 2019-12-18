#pragma once
#include <sys/types.h>
#include <vector>
#include <string>
#include "../Connections/ManagingConnection.h"
#include <stdio.h>

#define CONN_BUFF_SIZE 4096

class ConnectionBuffer
{
	bool isFull;
	std::vector<char> buffer;

	ManagingConnection *waitingReader,
		*waitingWriter;

	size_t writeOffset[2],
		readOffset;

	void wakeUpReader();
	void suspendReader(ManagingConnection *reader);
	void wakeUpWriter();
	void suspendWriter(ManagingConnection *reader);

public:
	ConnectionBuffer() : buffer(CONN_BUFF_SIZE)
	{
		isFull = false;
		waitingReader = NULL;
		waitingWriter = NULL;

		writeOffset[0] = 0;
		writeOffset[1] = 0;
		readOffset = 0;
	}

	ConnectionBuffer(std::string initialData)
		: buffer(std::max(initialData.size(), (size_t)CONN_BUFF_SIZE))
	{
		isFull = false;
		waitingReader = NULL;
		waitingWriter = NULL;

		writeOffset[0] = initialData.size();
		writeOffset[1] = 0;
		readOffset = 0;

		for (size_t i = 0; i < initialData.size(); i++)
		{
			buffer[i] = initialData[i];
		}
	}

	void finish()
	{
		isFull = true;
		wakeUpReader();
	}

	bool isCompleted();

	ssize_t writeToRecord(ManagingConnection *writer);

	ssize_t readFromRecord(ManagingConnection *reader);

	~ConnectionBuffer()
	{
		fprintf(stderr, "Connection buffer destructor\n");
	}
};