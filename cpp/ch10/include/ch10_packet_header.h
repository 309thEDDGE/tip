#ifndef CH10PKTHDR_H
#define CH10PKTHDR_H

#include "parse_context.h"
#include "ch10_packet_header_format.h"
#include "ch10.h"

class Ch10PacketHeader : public ParseContext<Ch10PacketHeaderFormat, Ch10PacketHeaderStatus>
{
	private:
	const uint16_t sync_value;
	//uint64_t rtc;
	const uint8_t checksum_size;
	std::vector<uint64_t> sync_locs;
	uint32_t sync_loc_index;
	uint32_t start_pos;
	void find_sync_locs();
	bool calc_checksum();
	bool calc_data_checksum();
	
	public:
	Ch10PacketHeader(BinBuff& buff, uint16_t ID) : ParseContext(buff, ID),
		sync_value(60197), checksum_size(2), sync_loc_index(0),
		start_pos(0) {}
	~Ch10PacketHeader();
	uint8_t Parse() override;
	uint32_t find_first_time_data_packet(bool append_mode);
	//void calc_rtc();
	
	void debug_info();
	uint8_t advance_to_next();
	uint8_t advance_to_next_sync_location();
	uint64_t& relative_time_counter();
	std::string status_desc();
	uint32_t& start_position();
};

#endif
