
#ifndef CH10_STATUS_H_
#define	CH10_STATUS_H_

#include <cstdint>

enum class Ch10Status : uint8_t
{
	OK,
	PKT_TYPE_YES,
	PKT_TYPE_NO,
	PKT_TYPE_EXIT,
	BAD_SYNC,
	BUFFER_LIMITED,
	INVALID_SECONDARY_HDR_FMT,
};

#endif