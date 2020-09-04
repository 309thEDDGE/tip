#include "network_packet_parser.h"

NetworkPacketParser::NetworkPacketParser() : pdu_type_(0), raw_payload_(nullptr)
{

}

uint8_t NetworkPacketParser::ParseEthernet(const uint8_t* buffer, const uint32_t& tot_size)
{
	// Looks like libtins handles non-EthernetII packets the same and ought
	// to be parsed as Dot3 in general, then inner_pdu used to get LLC, SNAP,
	// etc. 

	// Get the first two bytes of the payload. Have to skip both MAC addresses (6 bytes each),
	// and EtherType/Length (2 bytes) = 14 bytes.
	raw_payload_ = (const uint16_t*)buffer + 14;

	// Novell Raw IEEE 802.3
	if (*raw_payload_ == 0xffff)
	{
		printf("Raw 802.3\n");
	}
	// IEEE 802.2 SNAP
	else if (*raw_payload_ == 0xaaaa)
	{
		printf("802.2 SNAP\n");
	}
	// IEEE 802.2 LLC
	else
	{
		printf("802.2 LLC\n");
		return ParseEthernetLLC(buffer, tot_size);
	}
	return 0;
}

uint8_t NetworkPacketParser::ParseEthernetLLC(const uint8_t* buffer, const uint32_t& tot_size)
{
	Tins::LLC llc(buffer, tot_size);
	Tins::PDU* pdu = llc.inner_pdu();
	pdu_type_ = static_cast<uint16_t>(pdu->pdu_type());
	PrintPDUType();
	return 0;

	// Get Data layer mac addresses. 
	// Note: No IP, TCP or UDP packets found via dot3.find_pdu.
	//Tins::HWAddress<6> dst_addr = dot3.dst_addr();
	//Tins::HWAddress<6> src_addr = dot3.src_addr();
	//printf("\nDot3 -- > dst: %s, src: %s, length: %hu\n", dst_addr.to_string().c_str(), src_addr.to_string().c_str(),
	//	dot3.length());

	//Tins::PDU* pdu = dot3.inner_pdu();
	//pdu_type_ = static_cast<uint16_t>(pdu->pdu_type());
	////PrintPDUType();

	//// LLC
	//if (pdu_type_ == 26)
	//{
	//	Tins::LLC* llc = dynamic_cast<Tins::LLC*>(pdu);
	//	printf("dsap %hhu, ssap %hhu, type %hhu\n", llc->dsap(), llc->ssap(), llc->type());
	//	pdu = llc->inner_pdu();
	//	pdu_type_ = static_cast<uint16_t>(pdu->pdu_type());
	//	PrintPDUType();
	//	Tins::RawPDU* raw = dynamic_cast<Tins::RawPDU*>(pdu);
	//	const std::vector<uint8_t>& payload = raw->payload();
	//	printf("payload:\n");
	//	for (std::vector<uint8_t>::const_iterator it = payload.begin(); it != payload.end(); ++it)
	//	{
	//		printf("%hhu,", *it);
	//	}
	//	//printf("raw payload length %u\n", raw->payload_size());

	//}
	//else
	//	PrintPDUTypeNotHandled();

	return 0;
}

uint8_t NetworkPacketParser::ParseEthernetII(const uint8_t* buffer, const uint32_t& tot_size)
{
	Tins::EthernetII eth(buffer, tot_size);
	Tins::HWAddress<6> dst_addr = eth.dst_addr();
	Tins::HWAddress<6> src_addr = eth.src_addr();
	printf("\nEthII -- > dst: %s, src: %s, payloat type: %hu\n", dst_addr.to_string().c_str(), src_addr.to_string().c_str(),
		eth.payload_type());
	Tins::PDU* pdu = eth.inner_pdu();
	pdu_type_ = static_cast<uint16_t>(pdu->pdu_type());
	//PrintPDUType();

	// IP
	if (pdu_type_ == 28)
	{

	}
	else
		PrintPDUTypeNotHandled();

	return 0;
}

void NetworkPacketParser::PrintPDUType()
{
	printf("PDU type %hu = %s\n", pdu_type_, pdu_type_to_name_map_.at(pdu_type_).c_str());
}

void NetworkPacketParser::PrintPDUTypeNotHandled()
{
	printf("\n !!!!!! PDU type NOT HANDLED, %hu = %s\n", pdu_type_, 
		pdu_type_to_name_map_.at(pdu_type_).c_str());
}
//uint8_t 