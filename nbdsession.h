#pragma once

#include <asio.hpp>
#include <string>
#include <vector>
#include "nbdpublic.h"
#include "diskimage.h"

class nbd_session : public std::enable_shared_from_this<nbd_session> {
public:
	nbd_session(asio::ip::tcp::socket& socket, std::shared_ptr<DiskImage>& disk);
	virtual ~nbd_session();

	void start();
	void stop();

private:
	void receiveCommand();
	void receiveFixedNewOptionRequest();
	void receiveFixedNewExportRequest();

	void sendHandshake();

	void handleFixedNewOption(const nbd_fixed_new_option_request& fixed_opt_req);
	void handleFixedNewGo();
	void handleFixedNewExport();
	void handleRequest();
	void handleReadRequest();
	void handleWriteRequest();
	void handleDisconnRequest();

	void handleError(const asio::error_code& error);
	void sendSimpleReply(uint32_t error);

	asio::ip::tcp::socket socket_;
	std::shared_ptr<DiskImage> disk_image_;
	
	nbd_request request_;
	std::vector<uint8_t> request_data_;
	size_t request_data_offset_;
	size_t request_total_size_;

	nbd_fixed_new_export_request fixed_new_export_request_;
	nbd_fixed_new_export_reply fixed_new_export_reply_;

	nbd_fixed_new_option_header fixed_new_opt_header_;
	nbd_fixed_new_option_request fixed_new_option_request_;

	std::string export_name_;
	uint64_t export_size_;
	bool fixed_new_style_;
};
