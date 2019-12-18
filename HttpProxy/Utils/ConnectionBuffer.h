#pragma once
#include <sys/types.h>
#include <vector>
#include <string>
#include "../Connections/ManagingConnection.h"

class ConnectionBuffer
{
	constexpr static size_t BUFF_SIZE = 4096;

	bool isFull = false;
	std::vector<char> buffer;

	ManagingConnection *waitingReader = nullptr,
		*waitingWriter = nullptr;

	size_t writeOffset[2],
		readOffset = 0;

	void wakeUpReader();
	void suspendReader(ManagingConnection *reader);
	void wakeUpWriter();
	void suspendWriter(ManagingConnection *reader);

public:
	ConnectionBuffer() : buffer(BUFF_SIZE)
	{
		writeOffset[0] = 0;
		writeOffset[1] = 0;
	}

	ConnectionBuffer(std::string initialData) : buffer(std::max(initialData.size(), BUFF_SIZE))
	{
		writeOffset[0] = initialData.size();
		writeOffset[1] = 0;

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