#include "HttpContext.h"
#include "ProcessingConnections/HttpRequest.h"

HttpContext::HttpContext(int sockFd, FdRegistrar& _registrar, Cache& _cache)
	: registrar(_registrar), cache(_cache)
{
	state = new HttpRequest(sockFd);
}
