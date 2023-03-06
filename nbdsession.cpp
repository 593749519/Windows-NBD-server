#include "nbdsession.h"
#include <iostream>

nbd_session::nbd_session(asio::ip::tcp::socket& socket, std::shared_ptr<DiskImage>& diskimage)
	: socket_(std::move(socket)), disk_image_(diskimage), fixed_new_style_(true)
	, export_name_("test_img")
	, export_size_(20LL * 1024 * 1024 * 1024)
{
	asio::ip::tcp::no_delay opt_1(true);
	socket_.set_option(opt_1);
	asio::socket_base::send_buffer_size opt_2(0x40000);
	socket_.set_option(opt_2);
	asio::socket_base::receive_buffer_size opt_3(0x40000);
	socket_.set_option(opt_3);
}

nbd_session::~nbd_session()
{
	stop();
}

void nbd_session::start()
{
	disk_image_->open();
	export_size_ = disk_image_->size();

	sendHandshake();
}

void nbd_session::stop()
{
	disk_image_->close();
	socket_.close();
}

void nbd_session::receiveCommand()
{
	auto self = shared_from_this();

	asio::async_read(socket_, asio::buffer(&request_, sizeof(request_)),
		[self](asio::error_code ec, std::size_t length) {
		if (!ec && length == sizeof(self->request_)) {
			self->handleRequest();			
		}
		else {
			self->handleError(ec);
		}
	});
}

void nbd_session::receiveFixedNewOptionRequest()
{
	auto self = shared_from_this();

	//nbd-client version 3.xx, opt count first
	asio::async_read(socket_, asio::buffer(&fixed_new_opt_header_,  sizeof(fixed_new_opt_header_)),
		[self](asio::error_code ec, std::size_t length)
	{
		if (!ec && length == sizeof(self->fixed_new_opt_header_))
		{
			asio::async_read(self->socket_, asio::buffer(&self->fixed_new_option_request_, FIELD_OFFSET(nbd_fixed_new_option_request, data)),
				[self](asio::error_code ec, std::size_t length)
			{
				if (!ec && length == FIELD_OFFSET(nbd_fixed_new_option_request, data)) {
					self->fixed_new_option_request_.option = htobe32(self->fixed_new_option_request_.option);
					self->fixed_new_option_request_.optlen = htobe32(self->fixed_new_option_request_.optlen);
					self->fixed_new_option_request_.data.resize(self->fixed_new_option_request_.optlen);
					asio::async_read(self->socket_, asio::buffer(&self->fixed_new_option_request_.data[0], self->fixed_new_option_request_.optlen),
						[&, self](asio::error_code ec, std::size_t length)
					{
						if (!ec && length == self->fixed_new_option_request_.optlen) {
							self->handleFixedNewOption(self->fixed_new_option_request_);
						}
						else {
							self->handleError(ec);
						}
					});
				}
				else {
					self->handleError(ec);
				}
			});
		} 
		else {
			self->handleError(ec);
		}
	});
}

void nbd_session::handleFixedNewOption(const nbd_fixed_new_option_request& fixed_opt_req)
{
	switch (fixed_opt_req.option) {
	case NBD_OPT_EXPORT_NAME:
		handleFixedNewExport();
		break;
	case NBD_OPT_LIST:
		break;
	case NBD_OPT_GO:
		handleFixedNewGo();
		break;
	case NBD_OPT_ABORT:
		break;
	}
}

void nbd_session::sendHandshake()
{
	nbd_handshake req;
	req.nbdmagic = htobe64(NBD_MAGIC);
	req.version = htobe64(NBD_VERSION);
	req.flags = htobe16(NBD_FLAG_FIXED_NEWSTYLE | NBD_FLAG_NO_ZEROES);

	auto self = shared_from_this();
	asio::async_write(socket_, asio::buffer(&req, sizeof(req)),
		[self](asio::error_code ec, std::size_t length)
	{
		if (!ec && length == sizeof(req))
		{
			self->receiveFixedNewOptionRequest();
		}
		else
		{
			self->handleError(ec);
		}
	});
}

void nbd_session::receiveFixedNewExportRequest()
{
	auto self = shared_from_this();
	asio::async_read(self->socket_, asio::buffer(&self->fixed_new_export_request_, sizeof(self->fixed_new_export_request_)),
		[self](asio::error_code ec, std::size_t length) {
		if (!ec && length == sizeof(self->fixed_new_export_request_)) {
			self->handleError(ec);
		}
		else {
			self->fixed_new_export_request_.option = htobe32(self->fixed_new_export_request_.option);
			self->fixed_new_export_request_.optlen = htobe32(self->fixed_new_export_request_.optlen);
			self->fixed_new_export_request_.data.resize(self->fixed_new_export_request_.optlen);
			asio::async_read(self->socket_, asio::buffer(&self->fixed_new_export_request_.data[0], self->fixed_new_export_request_.optlen),
				[self](asio::error_code ec, std::size_t length) {
				if (!ec && length == self->fixed_new_export_request_.optlen) {
					self->handleError(ec);
				}
				else {
					self->handleFixedNewExport();
				}
			});
		}
	});
}

void nbd_session::handleFixedNewExport()
{
	fixed_new_export_reply_.export_size = htobe64(export_size_);
	fixed_new_export_reply_.flags = NBD_FLAG_NO_ZEROES;

	auto self = shared_from_this();
	asio::async_write(socket_, asio::buffer(&fixed_new_export_reply_, sizeof(fixed_new_export_reply_)),
		[self](asio::error_code ec, std::size_t length) {
		if (!ec && length == sizeof(self->fixed_new_export_reply_)) {
			self->receiveCommand();
		}
		else {
			self->handleError(ec);
		}
	});
}

void nbd_session::handleFixedNewGo()
{
	//nbd_fixed_new_go_reply go_reply = { 0 };
	//go_reply.magic = htobe64(NBD_OPTION_REPLY_MAGIC);
	//go_reply.opt = htobe32(NBD_OPT_GO);
	//go_reply.reply_type = htobe32(NBD_REP_ACK);
	//go_reply.datasize = htobe32(0);

	//auto self = shared_from_this();
	//asio::async_write(socket_, asio::buffer(&go_reply, sizeof(go_reply)),
	//	[self](asio::error_code ec, std::size_t bytesTransferred) {
	//	if (error) {
	//		self->handleError(error);
	//	}
	//	else {
	//		self->receiveCommand();
	//	}
	//});

	//return NBD_ENOTSUP on purpose for nbd-client to send old export name option request
	nbd_fixed_new_go_reply go_reply = { 0 };
	go_reply.magic = htobe64(NBD_OPTION_REPLY_MAGIC);
	go_reply.opt = htobe32(NBD_OPT_GO);
	go_reply.reply_type = htobe32(NBD_REP_ERR_UNSUP);
	go_reply.datasize = htobe32(0);

	auto self = shared_from_this();
	asio::async_write(socket_, asio::buffer(&go_reply, sizeof(go_reply)),
		[self](asio::error_code ec, std::size_t length) {
		if (ec) {
			self->handleError(ec);
		}
		else {
			self->receiveFixedNewExportRequest();
		}
	});
}

void nbd_session::handleRequest() {
	// Check that the received data is a valid request
	if (htobe32(request_.magic) != NBD_REQUEST_MAGIC) {
		std::cout << "Invalid request magic number" << std::endl;
		stop();
		return;
	}

	// Handle the request based on its type
	switch (htobe32(request_.type)) {
	case NBD_CMD_READ:
		handleReadRequest();
		break;
	case NBD_CMD_WRITE:
		handleWriteRequest();
		break;
	case NBD_CMD_DISC:
		handleDisconnRequest();
		break;
	default:
		break;
	}
}

void nbd_session::handleReadRequest()
{
	request_.from = htobe64(request_.from);
	request_.length = htobe32(request_.length);

	std::cout << "read from:" << request_.from << " count:" << request_.length << std::endl;

	if ((request_.from + request_.length) > export_size_) {
		sendSimpleReply(NBD_REP_ERR_INVALID);
		stop();
		return;
	}

	nbd_reply reply;
	memset(&reply, 0, sizeof(reply));
	reply.magic = htobe32(NBD_REPLY_MAGIC);
	reply.error = htobe32(0);
	reply.handle = request_.handle;

	request_data_ = disk_image_->read(request_.from, request_.length);
	if (request_data_.size() != request_.length) {
		std::cout << "read len: " << request_.length;
	}

	std::vector<asio::const_buffer> buffers = {
		asio::buffer(&reply, sizeof(reply)),
		asio::buffer(request_data_)
	};

	auto self = shared_from_this();
	asio::async_write(socket_, buffers,
		[self](asio::error_code ec, std::size_t length) {
		if (ec) {
			self->handleError(ec);
		}
		else {
			std::cout << "read sent: " << length << std::endl;
			self->receiveCommand();
		}
	});
}

void nbd_session::handleWriteRequest()
{
	request_.from = htobe64(request_.from);
	request_.length = htobe32(request_.length);

	std::cout << "write to:" << request_.from << " count:" << request_.length << std::endl;

	if ((request_.from + request_.length) > export_size_) {
		sendSimpleReply(NBD_REP_ERR_INVALID);
		stop();
		return;
	}

	auto self = shared_from_this();

	request_data_.resize(request_.length);
	asio::async_read(socket_, asio::buffer(request_data_),
		[self](asio::error_code ec, size_t length) {
		if (!ec && length == self->request_data_.size()) {

			self->disk_image_->write(self->request_.from, self->request_data_);

			nbd_reply reply;
			memset(&reply, 0, sizeof(reply));
			reply.magic = htobe32(NBD_REPLY_MAGIC);
			reply.error = htobe32(0);
			reply.handle = self->request_.handle;

			asio::async_write(self->socket_, asio::buffer(&reply, sizeof(reply)),
				[self](asio::error_code ec, std::size_t length) {
				if (ec) {
					self->handleError(ec);
				}
				else {
					self->receiveCommand();
				}
			});
		}
		else {
			self->handleError(ec);
		}
	});
}

void nbd_session::handleDisconnRequest()
{

}

void nbd_session::sendSimpleReply(uint32_t error)
{
	// Prepare simple reply
	nbd_simple_reply reply;
	reply.nbdmagic = htobe32(NBD_REPLY_MAGIC);
	reply.error = htobe32(error);

	auto self = shared_from_this();
	// Send simple reply
	asio::async_write(socket_, asio::buffer(&reply, sizeof(reply)),
		[self](asio::error_code ec, std::size_t length) {
		if (ec) {
			self->handleError(ec);
		}
		else {
			self->receiveCommand();
		}
	});
}

void nbd_session::handleError(const asio::error_code& error) {
	if (error != asio::error::operation_aborted) {
		std::cerr << "Error: " << error.message() << std::endl;
		stop();
	}
}

