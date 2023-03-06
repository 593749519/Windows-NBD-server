#pragma once

#include <cstdint>

// Convert big-endian 16-bit unsigned integer to host byte order
#define be16toh(x) \
    ((std::uint16_t(x) << 8) | std::uint16_t(x >> 8))

// Convert big-endian 32-bit unsigned integer to host byte order
#define be32toh(x) \
    ((std::uint32_t(x) << 24) | \
     (std::uint32_t(x) << 8 & 0x00FF0000) | \
     (std::uint32_t(x) >> 8 & 0x0000FF00) | \
     (std::uint32_t(x) >> 24))

// Convert big-endian 64-bit unsigned integer to host byte order
#define be64toh(x) \
    ((std::uint64_t(x) << 56) | \
     (std::uint64_t(x) << 40 & 0x00FF000000000000) | \
     (std::uint64_t(x) << 24 & 0x0000FF0000000000) | \
     (std::uint64_t(x) << 8 & 0x000000FF00000000) | \
     (std::uint64_t(x) >> 8 & 0x00000000FF000000) | \
     (std::uint64_t(x) >> 24 & 0x0000000000FF0000) | \
     (std::uint64_t(x) >> 40 & 0x000000000000FF00) | \
     (std::uint64_t(x) >> 56))

// Serialize a 16-bit unsigned integer to big-endian byte order
#define htobe16(x) be16toh(x)

// Serialize a 32-bit unsigned integer to big-endian byte order
#define htobe32(x) be32toh(x)

// Serialize a 64-bit unsigned integer to big-endian byte order
#define htobe64(x) be64toh(x)

//NBD magic numbers
#define NBD_REQUEST_MAGIC		0x25609513
#define NBD_REPLY_MAGIC			0x67446698
#define NBD_OPTION_REPLY_MAGIC	0x3e889045565a9ULL

#define NBD_MAGIC				0x4e42444d41474943ULL /* ASCII "NBDMAGIC" */
#define NBD_OLD_VERSION			0x0000420281861253ULL
#define NBD_VERSION				0x49484156454f5054ULL /* ASCII "IHAVEOPT" */

//NBD flags
#define NBD_FLAG_FIXED_NEWSTYLE				(1 << 0)
#define NBD_FLAG_NO_ZEROES					(1 << 1)
#define NBD_FLAG_C_NO_FLUSH					(1 << 2)
#define NBD_FLAG_C_FUA						(1 << 3)
#define NBD_FLAG_C_NO_HOLE					(1 << 4)
#define NBD_FLAG_C_NBD_OPT					(1 << 5)
#define NBD_FLAG_C_DATA						(1 << 6)
#define NBD_FLAG_CAN_META_CONTEXT			(1 << 7)
#define NBD_FLAG_SEND_TRIM					(1 << 8)
#define NBD_FLAG_SEND_WRITE_ZEROES			(1 << 9)
#define NBD_FLAG_STARTTLS					(1L << 30)

#define NBD_MAX_NAME_LEN					4096

//NBD options
#define NBD_OPT_EXPORT_NAME        1
#define NBD_OPT_ABORT              2
#define NBD_OPT_LIST               3
#define NBD_OPT_STARTTLS           5
#define NBD_OPT_INFO               6
#define NBD_OPT_GO                 7
#define NBD_OPT_STRUCTURED_REPLY   8
#define NBD_OPT_LIST_META_CONTEXT  9
#define NBD_OPT_SET_META_CONTEXT   10

//NBD commands
#define NBD_CMD_READ              0
#define NBD_CMD_WRITE             1
#define NBD_CMD_DISC              2 /* Disconnect. */
#define NBD_CMD_FLUSH             3
#define NBD_CMD_TRIM              4
#define NBD_CMD_CACHE             5
#define NBD_CMD_WRITE_ZEROES      6
#define NBD_CMD_BLOCK_STATUS      7


// NBD error codes
#define NBD_REP_ACK					(1)	/**< ACK a request. Data: option number to be acked */
#define NBD_REP_SERVER				(2)	/**< Reply to NBD_OPT_LIST (one of these per server; must be followed by NBD_REP_ACK to signal the end of the list */
#define NBD_REP_INFO				(3)	/**< Reply to NBD_OPT_INFO */
#define NBD_REP_META_CONTEXT        (4)
#define NBD_REP_FLAG_ERROR			(1 << 31)	/** If the high bit is set, the reply is an error */
#define NBD_REP_ERR_UNSUP			(1 | NBD_REP_FLAG_ERROR)	/**< Client requested an option not understood by this version of the server */
#define NBD_REP_ERR_POLICY			(2 | NBD_REP_FLAG_ERROR)	/**< Client requested an option not allowed by server configuration. (e.g., the option was disabled) */
#define NBD_REP_ERR_INVALID			(3 | NBD_REP_FLAG_ERROR)	/**< Client issued an invalid request */
#define NBD_REP_ERR_PLATFORM		(4 | NBD_REP_FLAG_ERROR)	/**< Option not supported on this platform */
#define NBD_REP_ERR_TLS_REQD		(5 | NBD_REP_FLAG_ERROR)	/**< TLS required */
#define NBD_REP_ERR_UNKNOWN			(6 | NBD_REP_FLAG_ERROR)	/**< NBD_OPT_INFO or ..._GO requested on unknown export */
#define NBD_REP_ERR_BLOCK_SIZE_REQD	(8 | NBD_REP_FLAG_ERROR)	/**< Server is not willing to serve the export without the block size being negotiated */

#pragma pack(push, 1)

struct nbd_request {
	uint32_t magic;
	uint32_t type;
	uint64_t handle;
	uint64_t from;
	uint32_t length;
};

struct nbd_reply {
	uint32_t magic;
	uint32_t error;
	uint64_t handle;
};

struct nbd_simple_reply {
	uint32_t nbdmagic;
	uint32_t error;
};

struct nbd_handshake
{
	uint64_t nbdmagic;
	uint64_t version;
	uint16_t flags;
};

struct nbd_fixed_new_option_header
{
	uint32_t flags;
	uint64_t magic;
};

struct nbd_fixed_new_option_request
{
	uint32_t option;
	uint32_t optlen;
	std::vector<char> data;
};

struct nbd_fixed_new_export_request
{
	uint64_t magic;
	uint32_t option;
	uint32_t optlen;
	std::vector<char> data;
};

struct nbd_fixed_new_export_reply
{
	uint64_t export_size;	
	uint16_t flags;
};

struct nbd_fixed_new_go_reply {
	uint64_t magic;
	uint32_t opt;
	uint32_t reply_type;
	uint32_t datasize;
	char data[];
};


#pragma pack(pop)
