#include "network_packet_parser.h"

NetworkPacketParser::NetworkPacketParser() : pdu_type_(0), pdu_ptr_(nullptr), raw_pdu_(nullptr),
udp_pdu_(nullptr), ip_pdu_(nullptr), llc_pdu_(nullptr), ethernet_data_ptr_(nullptr)
{

}

bool NetworkPacketParser::Parse(const uint8_t* buffer, const uint32_t& length,
	EthernetData* eth_data)
{
	Tins::Dot3 dot3;
	try
	{
		dot3 = Tins::Dot3(buffer, length);
	}
	catch (const Tins::malformed_packet& e)
	{
		SPDLOG_INFO("PDU type: {:s}", pdu_type_to_name_map_.at(pdu_type_));
		return false;
	}
	pdu_type_ = static_cast<uint16_t>(dot3.pdu_type());
	SPDLOG_INFO("PDU type: {:s}", pdu_type_to_name_map_.at(pdu_type_));
	return true;
}

uint8_t NetworkPacketParser::ParseEthernet(const uint8_t* buffer, const uint32_t& tot_size,
	EthernetData* ed)
{
	ethernet_data_ptr_ = ed;
	// Looks like libtins handles non-EthernetII packets the same and ought
	// to be parsed as Dot3 in general, then inner_pdu used to get LLC, SNAP,
	// etc. 

	Tins::Dot3 dot3(buffer, tot_size);
	ethernet_data_ptr_->dst_mac_addr_ = dot3.dst_addr().to_string();
	ethernet_data_ptr_->src_mac_addr_ = dot3.src_addr().to_string();
	/*printf("\nDot3 -- > dst: %s, src: %s, length: %hu\n", ethernet_data_ptr_->dst_mac_addr_.c_str(), 
		ethernet_data_ptr_->src_mac_addr_.c_str(), dot3.length());*/

	// Get inner pdu and check type.
	pdu_ptr_ = dot3.inner_pdu();
	PDUType();

	// LLC
	if (pdu_type_ == 26)
	{
		return ParseEthernetLLC();
	}
	// SNAP
	else if (pdu_type_ == 27)
	{
		PrintPDUTypeNotHandled();
	}
	// Raw
	else if (pdu_type_ == 0)
	{
		PrintPDUTypeNotHandled();
	}
	else
	{
		printf("\n\nCase not handled:\n");
		PrintPDUTypeNotHandled();
		return 1;
	}

	return 0;
}

uint8_t NetworkPacketParser::ParseEthernetLLC()
{
	llc_pdu_ = dynamic_cast<Tins::LLC*>(pdu_ptr_);
	ethernet_data_ptr_->dsap_ = llc_pdu_->dsap();
	ethernet_data_ptr_->ssap_ = llc_pdu_->ssap();

	// See "Operational Modes" at https://en.wikipedia.org/wiki/IEEE_802.2
	ethernet_data_ptr_->frame_format_ = llc_pdu_->type();
	ethernet_data_ptr_->snd_seq_number_ = llc_pdu_->send_seq_number();
	ethernet_data_ptr_->rcv_seq_number_ = llc_pdu_->receive_seq_number();
	/*printf("LLC: dsap %hhu, ssap %hhu, type %hhu, sendseqnum %hhu, rcvseqnum %hhu\n",
		ethernet_data_ptr_->dsap_, ethernet_data_ptr_->ssap_, 
		ethernet_data_ptr_->frame_format_, ethernet_data_ptr_->snd_seq_number_, 
		ethernet_data_ptr_->rcv_seq_number_);*/

	pdu_ptr_ = llc_pdu_->inner_pdu();
	PDUType();

	if (pdu_type_ == 0)
	{
		return ParseRaw();
	}
	else
	{
		PrintPDUTypeNotHandled();
		return 1;
	}

	return 0;
}

uint8_t NetworkPacketParser::ParseEthernetII(const uint8_t* buffer, const uint32_t& tot_size,
	EthernetData* ed)
{
	ethernet_data_ptr_ = ed;
	Tins::EthernetII eth(buffer, tot_size);
	ethernet_data_ptr_->dst_mac_addr_ = eth.dst_addr().to_string();
	ethernet_data_ptr_->src_mac_addr_ = eth.src_addr().to_string();
	ethernet_data_ptr_->payload_type_ = eth.payload_type();
	/*printf("\nEthII -- > dst: %s, src: %s, payload type: %hu\n", ethernet_data_ptr_->dst_mac_addr_.c_str(),
		ethernet_data_ptr_->src_mac_addr_.c_str(), ethernet_data_ptr_->payload_type_);*/
	pdu_ptr_ = eth.inner_pdu();
	PDUType();

	// See possible Ethernet II types at https://en.wikipedia.org/wiki/Ethernet_frame

	// IP/IPv4
	if (pdu_type_ == 28)
	{
		if (ethernet_data_ptr_->payload_type_ != 0x0800)
			printf("Wrong payload type (%hu) for pdu type (%hu)!\n", 
				ethernet_data_ptr_->payload_type_, pdu_type_);
		
		return ParseIPv4();
	}
	// ARP
	else if (pdu_type_ == 29)
	{
		PrintPDUTypeNotHandled();
		if (ethernet_data_ptr_->payload_type_ != 0x0806)
			printf("Wrong payload type (%hu) for pdu type (%hu)!\n", 
				ethernet_data_ptr_->payload_type_, pdu_type_);
	}
	// IPv6
	else if (pdu_type_ == 40)
	{
		PrintPDUTypeNotHandled();
		if (ethernet_data_ptr_->payload_type_ != 0x86DD)
			printf("Wrong payload type (%hu) for pdu type (%hu)!\n", 
				ethernet_data_ptr_->payload_type_, pdu_type_);
	}
	// IEEE 802.1Q
	else if (pdu_type_ == 44)
	{
		PrintPDUTypeNotHandled();
		if (ethernet_data_ptr_->payload_type_ != 0x8100)
			printf("Wrong payload type (%hu) for pdu type (%hu)!\n", 
				ethernet_data_ptr_->payload_type_, pdu_type_);
	}
	else
	{
		printf("\n\nCase not handled:\n");
		PrintPDUTypeNotHandled();
		return 1;
	}

	return 0;
}

uint8_t NetworkPacketParser::ParseIPv4()
{
	ip_pdu_ = dynamic_cast<Tins::IP*>(pdu_ptr_);
	ethernet_data_ptr_->src_ip_addr_ = ip_pdu_->src_addr().to_string();
	ethernet_data_ptr_->dst_ip_addr_ = ip_pdu_->dst_addr().to_string();

	ethernet_data_ptr_->id_ = ip_pdu_->id();
	ethernet_data_ptr_->protocol_ = ip_pdu_->protocol();
	ethernet_data_ptr_->offset_ = ip_pdu_->fragment_offset();

	/*printf("IPv4: srcaddr %s, dstaddr %s, id %hu, protocol %hhu, offset %hu\n",
		ethernet_data_ptr_->src_ip_addr_.c_str(), ethernet_data_ptr_->dst_ip_addr_.c_str(), 
		ethernet_data_ptr_->id_, ethernet_data_ptr_->protocol_, ethernet_data_ptr_->offset_);*/

	pdu_ptr_ = ip_pdu_->inner_pdu();
	PDUType();

	// UDP
	if (pdu_type_ == 31)
	{
		return ParseUDP();
	}
	else
	{
		PrintPDUTypeNotHandled();
		return 1;
	}

	return 0;
}

uint8_t NetworkPacketParser::ParseUDP()
{
	udp_pdu_ = dynamic_cast<Tins::UDP*>(pdu_ptr_);
	ethernet_data_ptr_->src_port_ = udp_pdu_->sport();
	ethernet_data_ptr_->dst_port_ = udp_pdu_->dport();
	/*printf("UDP: src port %hu, dst port %hu\n", ethernet_data_ptr_->src_port_, 
		ethernet_data_ptr_->dst_port_);*/

	pdu_ptr_ = udp_pdu_->inner_pdu();
	PDUType();

	// Raw
	if (pdu_type_ == 0)
	{
		return ParseRaw();
	}
	else
	{
		PrintPDUTypeNotHandled();
		return 1;
	}

	return 0;
}

uint8_t NetworkPacketParser::ParseRaw()
{
	raw_pdu_ = dynamic_cast<Tins::RawPDU*>(pdu_ptr_);

	// Check payload length.
	ethernet_data_ptr_->payload_size_ = raw_pdu_->payload_size();

	if (ethernet_data_ptr_->payload_size_ > EthernetData::mtu_)
	{
		/*printf("NetworkPacketParser::ParseRaw(): Payload size exceeds MTU: %u\n",
			ethernet_data_ptr_->payload_size_);*/
		return 1;
	}

	// This copy is inefficient because the payload will be copied again into
	// the vector from which the payloads will be eventually read into parquet.
	// Much better to hold a pointer to the final payload vector, somehow retrieved from
	// the ethernet parquet writing class, in the EthernetData object. Then data
	// can be written/copied once.
	std::copy(raw_pdu_->payload().begin(), raw_pdu_->payload().end(), 
		ethernet_data_ptr_->payload_ptr_);

	return 0;
}

void NetworkPacketParser::PDUType()
{
	pdu_type_ = static_cast<uint16_t>(pdu_ptr_->pdu_type());
	//printf("PDU type %hu = %s\n", pdu_type_, pdu_type_to_name_map_.at(pdu_type_).c_str());
}

void NetworkPacketParser::PrintPDUTypeNotHandled()
{
	printf("\n !!!!!! PDU type NOT HANDLED, %hu = %s\n", pdu_type_, 
		pdu_type_to_name_map_.at(pdu_type_).c_str());
}

