#ifndef CH10PKTHDRFMT_H
#define CH10PKTHDRFMT_H

#include <stdint.h>

/*class Ch10PacketHeaderFormat
{
	public:
	uint16_t  	sync;
	uint16_t  	chanID;
	uint32_t  	pkt_size;
	uint32_t  	data_size;
	uint8_t   	data_type_ver;
	uint8_t   	seq_num;
	uint8_t		checksum_existence	: 2;
	uint8_t	  	time_format			: 2;
	uint8_t 	overflow_err		: 1;
	uint8_t 	sync_err			: 1;
	uint8_t 	intrapkt_ts_source	: 1;
	uint8_t 	secondary_hdr		: 1;
	uint8_t 	data_type;
	uint32_t	rtc1;
	uint16_t 	rtc2;
	uint16_t 	checksum;
	
};*/

class Ch10PacketHeaderFormat
{
	public:
	uint32_t  	sync				: 16;
	uint32_t  	chanID				: 16;
	uint32_t  	pkt_size;
	uint32_t  	data_size;
	uint32_t   	data_type_ver		: 8;
	uint32_t   	seq_num				: 8;
	uint32_t	checksum_existence	: 2;
	uint32_t	time_format			: 2;
	uint32_t 	overflow_err		: 1;
	uint32_t 	sync_err			: 1;
	uint32_t 	intrapkt_ts_source	: 1;
	uint32_t 	secondary_hdr		: 1;
	uint32_t 	data_type			: 8;
	uint32_t	rtc1;
	uint32_t 	rtc2				: 16;
	uint32_t 	checksum			: 16;
	
};

enum class Ch10PacketHeaderStatus: uint8_t
{
	HEADER_EXCEEDS		= 1,
	SYNC_INCORRECT		= 2,
	CHECKSUM_INCORRECT 	= 3,
	BODY_EXCEEDS		= 4,
	PARSE_OK			= 5,
	DATA_CHECKSUM_INCORRECT = 6,
};

#endif

/*union Ch10PacketHeaderUnion
{
	Ch10PacketHeaderFormat fmt;
	uint8_t bytedata[sizeof(Ch10PacketHeaderFormat)];
};*/

