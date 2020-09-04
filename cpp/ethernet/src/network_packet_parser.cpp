#include "network_packet_parser.h"

NetworkPacketParser::NetworkPacketParser() : pdu_type_(0)
{

}

uint8_t NetworkPacketParser::ParseEthernetDot3(const uint8_t* buffer, const uint32_t& tot_size)
{
	
	Tins::Dot3 dot3(buffer, tot_size);

	// Get Data layer mac addresses.
	Tins::HWAddress<6> dst_addr = dot3.dst_addr();
	Tins::HWAddress<6> src_addr = dot3.src_addr();
	printf("dst: %s, src: %s\n", dst_addr.to_string().c_str(), src_addr.to_string().c_str());

	pdu_type_ = static_cast<uint8_t>(dot3.inner_pdu()->pdu_type());
	printf("Inner pdu type: %hu, %s\n", pdu_type_, pdu_type_to_name_map_.at(pdu_type_).c_str());

	// UDP
	if (pdu_type_ == 26)
	{
		//Tins::UDP* udp = dot3.inner_pdu();
		//printf("dport is %hu\n", udp->dport());
	}

	return 0;
}

//uint8_t 