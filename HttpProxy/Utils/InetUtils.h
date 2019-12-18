#pragma once
#include <string>
#include "netdb.h"

struct sockaddr_in getAddr(std::string host, short port);

int openRedirectedSocket(std::string addr, short port);

std::pair<std::string, short> parseHost(std::string host);