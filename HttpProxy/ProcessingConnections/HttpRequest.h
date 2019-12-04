#pragma once

#include <string>
#include <unistd.h>
#include <sys/types.h>
#include "../HttpContext.h"

class HttpRequest : public HttpContext::ProcessingState
{
	constexpr static size_t BUFF_SIZE = 1024;
	unsigned char buffer[BUFF_SIZE];

	bool readRequest = false,
		eof = false;
	
	std::string request;

	const int clientFd;

public:
	HttpRequest(int _clientFd) : clientFd(_clientFd) {}

	ProcessingState* process(HttpContext* context) override;

	HttpContext::ProcessingState* parseRequest(HttpContext *context);

	~HttpRequest()
	{
		if (finished)
		{
			fprintf(stderr, "close fd\n");
			close(clientFd);
		}
	}
};
