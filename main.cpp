#include <iostream>
#include <memory>
#include <string>
#include <asio.hpp>
#include "NbdServerManager.h"

int main(int argc, char* argv[])
{
	if (argc < 3) {
		std::cerr << "Usage: " << argv[0] << " <port> <path1> [<path2> ...]" << std::endl;
		return 1;
	}

	try {
		asio::io_service io_service;
		NbdServerManager manager(io_service, std::atoi(argv[1]));

		for (int i = 2; i < argc; ++i) {
			manager.add_server(argv[i]);
		}

		manager.start();
	}
	catch (std::exception& e) {
		std::cerr << "Exception: " << e.what() << std::endl;
	}

	return 0;
}
