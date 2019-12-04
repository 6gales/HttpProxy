#pragma once
#include <string>
#include "netdb.h"

struct sockaddr_in getAddr(std::string host, int port);

int openRedirectedSocket(std::string addr, int port);

std::pair<std::string, int> parseHost(std::string host);