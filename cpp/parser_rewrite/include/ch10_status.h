
#ifndef CH10_STATUS_H_
#define	CH10_STATUS_H_

#include <cstdint>

enum class Ch10Status : uint8_t
{
	NONE,
	OK,
	PKT_TYPE_YES,
	PKT_TYPE_NO,
	PKT_TYPE_EXIT,
	BAD_SYNC,
	BUFFER_LIMITED,
	INVALID_SECONDARY_HDR_FMT,
	CHECKSUM_NOT_PRESENT,
	CHECKSUM_NOT_HANDLED,
	CHECKSUM_TRUE,
	CHECKSUM_FALSE,
	TDP_NONE,
	MILSTD1553_TS_NOT_HANDLED,
	MILSTD1553_MSG_COUNT,
	MILSTD1553_MSG_LENGTH,
	VIDEOF0_NONINTEGER_SUBPKT_COUNT
};

#endif