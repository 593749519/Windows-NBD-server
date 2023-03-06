#pragma once

class DiskImage {
public:
	DiskImage(const std::string& file_path) : file_path_(file_path) {};
	virtual ~DiskImage() {};

	virtual void open() = 0;
	virtual void close() = 0;
	virtual void flush() = 0;
	virtual void cache(uint64_t offset, uint64_t length) = 0;
	virtual void trim(uint64_t offset, uint64_t length) = 0;
	virtual void discard(uint64_t offset, uint64_t length) = 0;
	virtual uint64_t size() const = 0;

	virtual std::vector<std::uint8_t> read(uint64_t offset, std::size_t length) = 0;
	virtual void write(uint64_t offset, const std::vector<std::uint8_t>& data) = 0;

protected:
	std::string file_path_;
};
