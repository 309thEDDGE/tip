#include "network_packet_parser.h"

NetworkPacketParser::NetworkPacketParser()
{

}

uint8_t NetworkPacketParser::ParseEthernetDot3(const uint8_t* buffer, const uint32_t& tot_size)
{
	Tins::Dot3 dot3(buffer, tot_size);
	Tins::HWAddress<6> dst_addr = dot3.dst_addr();
	Tins::HWAddress<6> src_addr = dot3.src_addr();
	printf("dst: %s, src: %s\n", dst_addr.to_string().c_str(), src_addr.to_string().c_str());

	return 0;
}