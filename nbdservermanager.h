#pragma once

#include "nbdserver.h"
#include "windowsdiskimage.h"

class NbdServerManager {
public:
	NbdServerManager(asio::io_service& io_service, unsigned short port)
		: io_service_(io_service), acceptor_(io_service, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
	{}

	void add_server(const std::string& path) {
		auto disk_image = std::make_shared<WindowsDiskImage>(path);
		auto server = std::make_shared<nbdserver>(io_service_, disk_image);
		server->start(acceptor_);
		servers_.push_back(server);
	}

	void start() {
		io_service_.run();
	}

private:
	asio::io_service& io_service_;
	asio::ip::tcp::acceptor acceptor_;
	std::vector<std::shared_ptr<nbdserver>> servers_;
};
