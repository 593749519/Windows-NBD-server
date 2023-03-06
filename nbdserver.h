#pragma once

#include <iostream>
#include <string>
#include <memory>
#include <functional>
#include <asio.hpp>
#include "diskimage.h"
#include "nbdsession.h"

class nbdserver : public std::enable_shared_from_this<nbdserver> {
public:
	nbdserver(asio::io_service& io_service, const std::shared_ptr<DiskImage>& disk_image)
		: io_service_(io_service), socket_(io_service), disk_image_(disk_image)
	{}

	void start(asio::ip::tcp::acceptor& acceptor) {
		acceptor_ = &acceptor;
		do_accept();
	}

private:
	void do_accept() {
		auto self = shared_from_this();
		acceptor_->async_accept(self->socket_, [self](const asio::error_code& error) {
			if (!error) {
				// Create a new session and start it
				std::make_shared<nbd_session>(self->socket_, self->disk_image_)->start();

				// Start accepting new connections
				self->do_accept();
			}
		});
	}

	asio::io_service& io_service_;
	asio::ip::tcp::acceptor* acceptor_;
	asio::ip::tcp::socket socket_;
	std::shared_ptr<DiskImage> disk_image_;
};


