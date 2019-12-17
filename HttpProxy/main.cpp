#include <iostream>
#include "HttpProxy.h"

int main(int argc, char **argv)
{
	if (argc == 2 && std::string(argv[1]) == "-h")
	{
		std::cout << "Usage: <threads> <lport>" << std::endl;
		return 0;
	}

	int threads = (argc > 1 ? std::atoi(argv[1]) : 1);
	short lport = static_cast<short>(argc > 2 ? std::atoi(argv[2]) : 8080);
	
	if (lport <= 0 || threads <= 0)
	{
		std::cerr << "Usage: <threads> <lport>" << std::endl;
		return 1;
	}
	
	HttpProxy proxy{ lport, static_cast<size_t>(threads) };
	proxy.run();
}