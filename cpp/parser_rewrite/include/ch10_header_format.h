

#ifndef CH10_HEADER_FORMAT_H_
#define CH10_HEADER_FORMAT_H_

#include <cstdint>

class Ch10PacketHeaderFmt
{
public:
	uint32_t  	sync : 16;
	uint32_t  	chanID : 16;
	uint32_t  	pkt_size;
	uint32_t  	data_size;
	uint32_t   	data_type_ver : 8;
	uint32_t   	seq_num : 8;
	uint32_t	checksum_existence : 2;
	uint32_t	time_format : 2;
	uint32_t 	overflow_err : 1;
	uint32_t 	sync_err : 1;
	uint32_t 	intrapkt_ts_source : 1;
	uint32_t 	secondary_hdr : 1;
	uint32_t 	data_type : 8;
	uint32_t	rtc1;
	uint32_t 	rtc2 : 16;
	uint32_t 	checksum : 16;

};

#endif
