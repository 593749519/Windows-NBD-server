#pragma once

#include <windows.h>
#include <winioctl.h>
#include "diskimage.h"

class WindowsDiskImage : public DiskImage {
public:
	WindowsDiskImage(const std::string& file_path) : DiskImage(file_path), hFile_(INVALID_HANDLE_VALUE) {}

	virtual ~WindowsDiskImage() {
		close();
	}

	void open() override {
		hFile_ = CreateFileA(file_path_.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING, NULL);
		if (hFile_ == INVALID_HANDLE_VALUE) {
			throw std::runtime_error("Failed to open disk image");
		}
	}

	void close() override {
		if (hFile_ != INVALID_HANDLE_VALUE) {
			CloseHandle(hFile_);
			hFile_ = INVALID_HANDLE_VALUE;
		}
	}

	void flush() override {
		if (FlushFileBuffers(hFile_) == 0) {
			throw std::runtime_error("Failed to flush disk image");
		}
	}

	void trim(uint64_t offset, uint64_t length) override {
		//DWORD bytesReturned = 0;
		//DISK_SPACE_RESERVATION_PARAMS reserveParams = { 0 };
		//reserveParams.Version = DISK_SPACE_RESERVATION_VERSION;
		//reserveParams.Size = sizeof(DISK_SPACE_RESERVATION_PARAMS);
		//reserveParams.Action = DISK_SPACE_RESERVATION_ACTION_RELEASE;
		//reserveParams.ReservationReadOffset.QuadPart = offset;
		//reserveParams.ReservationReadSize.QuadPart = length;
		//if (DeviceIoControl(hFile_, IOCTL_DISK_SPACE_RESERVATION, &reserveParams, sizeof(reserveParams), NULL, 0, &bytesReturned, NULL) == 0) {
		//	throw std::runtime_error("Failed to trim disk image");
		//}
	}

	void discard(uint64_t offset, uint64_t length) override {
		//DWORD bytesReturned = 0;
		//DISK_SPACE_RESERVATION_PARAMS reserveParams = { 0 };
		//reserveParams.Version = DISK_SPACE_RESERVATION_VERSION;
		//reserveParams.Size = sizeof(DISK_SPACE_RESERVATION_PARAMS);
		//reserveParams.Action = DISK_SPACE_RESERVATION_ACTION_SHRINK;
		//reserveParams.ReservationReadOffset.QuadPart = offset;
		//reserveParams.ReservationReadSize.QuadPart = length;
		//if (DeviceIoControl(hFile_, IOCTL_DISK_SPACE_RESERVATION, &reserveParams, sizeof(reserveParams), NULL, 0, &bytesReturned, NULL) == 0) {
		//	throw std::runtime_error("Failed to discard disk image");
		//}
	}

	void cache(uint64_t offset, uint64_t length) override {

	}

	uint64_t size() const override {
		LARGE_INTEGER size;
		if (GetFileSizeEx(hFile_, &size) == 0) {
			throw std::runtime_error("Failed to get disk image size");
		}
		return size.QuadPart;
	}

	std::vector<std::uint8_t> read(uint64_t offset, std::size_t length) override {
		DWORD bytesRead = 0;
		std::vector<std::uint8_t> buffer(length);
		OVERLAPPED overlapped = { 0 };
		overlapped.Offset = (DWORD)offset;
		overlapped.OffsetHigh = (DWORD)(offset >> 32);
		if (ReadFile(hFile_, buffer.data(), (DWORD)length, &bytesRead, &overlapped) == 0 || bytesRead != length) {
			throw std::runtime_error("Failed to read from disk image");
		}
		return buffer;
	}

	void write(uint64_t offset, const std::vector<std::uint8_t>& data) override {
		DWORD num_bytes_written;
		OVERLAPPED overlapped = { 0 };
		overlapped.Offset = static_cast<DWORD>(offset);
		overlapped.OffsetHigh = static_cast<DWORD>(offset >> 32);
		if (!WriteFile(hFile_, data.data(), data.size(), &num_bytes_written, &overlapped)) {
			throw std::runtime_error("Failed to write to file.");
		}
	}


private:
	HANDLE hFile_;
};