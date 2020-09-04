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
	printf("dst: %s, src: %s, length: %hu\n", dst_addr.to_string().c_str(), src_addr.to_string().c_str(),
		dot3.length());

	Tins::PDU* pdu = dot3.inner_pdu();
	pdu_type_ = static_cast<uint8_t>(pdu->pdu_type());
	PrintPDUType();

	// LLC
	if (pdu_type_ == 26)
	{
		Tins::LLC* llc = dynamic_cast<Tins::LLC*>(pdu);
		printf("dsap %hhu, ssap %hhu, type %hhu\n", llc->dsap(), llc->ssap(), llc->type());
		pdu = llc->inner_pdu();
		pdu_type_ = static_cast<uint16_t>(pdu->pdu_type());
		PrintPDUType();
		//Tins::RawPDU* raw = dynamic_cast<Tins::RawPDU*>(pdu);
		//printf("raw payload length %u\n", raw->payload_size());

	}
	

	return 0;
}

void NetworkPacketParser::PrintPDUType()
{
	printf("PDU type %hu - %s\n", pdu_type_, pdu_type_to_name_map_.at(pdu_type_).c_str());
}
//uint8_t 