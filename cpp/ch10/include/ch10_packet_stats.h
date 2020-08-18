#ifndef CH10PACKETSTATS_H
#define CH10PACKETSTATS_H

#include <cstdint>

class PacketStats
{

public:
	uint32_t time_data_count;
	uint32_t milstd_1553pkt_count;
	uint32_t video_data_count;
	uint32_t discrete_data_count;
	uint32_t analog_data_count;
	
	PacketStats() : time_data_count(0), milstd_1553pkt_count(0), video_data_count(0),
		discrete_data_count(0), analog_data_count(0) {}

};

#endif