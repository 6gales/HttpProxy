#pragma once
#include <string>
#include "netdb.h"

struct sockaddr_in getAddr(const std::string &host, short port);

int openRedirectedSocket(const std::string &addr, short port);

std::pair<std::string, short> parseHost(const std::string &host);