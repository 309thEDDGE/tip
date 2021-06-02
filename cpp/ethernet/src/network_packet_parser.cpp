#include "network_packet_parser.h"

NetworkPacketParser::NetworkPacketParser() : pdu_type_(Tins::PDU::PDUType::UNKNOWN), 
pdu_ptr_(nullptr), raw_pdu_(nullptr), pdu_type_val_(999),
udp_pdu_(nullptr), ip_pdu_(nullptr), llc_pdu_(nullptr), ethernet_data_ptr_(nullptr),
dot3_(), eth2_()
{

}

bool NetworkPacketParser::Parse(const uint8_t* buffer, const uint32_t& length,
	EthernetData* eth_data)
{
	// Determine if 802.3 or Ethernet II
	try
	{
		dot3_ = Tins::Dot3(buffer, length);
		if (dot3_.length() > mtu_)
		{
			eth2_ = Tins::EthernetII(buffer, length);
			return ParseEthernetII(eth2_, eth_data);
		}
		return ParseEthernet(dot3_, eth_data);
	}
	catch (const Tins::malformed_packet& e)
	{
		SPDLOG_WARN("Error: {:s}", e.what());
		return false;
	}
	
	return true;
}

bool NetworkPacketParser::ParseEthernet(Tins::Dot3& dot3_pdu, EthernetData* const ed)
{
	
	ed->dst_mac_addr_ = dot3_pdu.dst_addr().to_string();
	ed->src_mac_addr_ = dot3_pdu.src_addr().to_string();
	ed->ethertype_ = dot3_pdu.length();
	/*printf("\nDot3 -- > dst: %s, src: %s, length: %hu\n", ethernet_data_ptr_->dst_mac_addr_.c_str(), 
		ethernet_data_ptr_->src_mac_addr_.c_str(), dot3.length());*/

	// Get inner pdu and check type.
	pdu_ptr_ = dot3_pdu.inner_pdu();
	//pdu_type_val_ = static_cast<uint16_t>(pdu_ptr_->pdu_type());
	//SPDLOG_INFO("Ethernet inner pdu is {:s}", pdu_type_to_name_map_.at(pdu_type_val_));
	return ParserSelector(pdu_ptr_, ed);
}

bool NetworkPacketParser::ParseEthernetLLC(Tins::LLC* llc_pdu, EthernetData* const ed)
{
	
	ed->dsap_ = llc_pdu->dsap();
	ed->ssap_ = llc_pdu->ssap();

	// See "Operational Modes" at https://en.wikipedia.org/wiki/IEEE_802.2
	ed->frame_format_ = llc_pdu->type();

	// The following two fields may only be relevant to I-format PDUs.
	ed->snd_seq_number_ = llc_pdu->send_seq_number();
	ed->rcv_seq_number_ = llc_pdu->receive_seq_number();

	/*printf("LLC: dsap %hhu, ssap %hhu, type %hhu, sendseqnum %hhu, rcvseqnum %hhu\n",
		ethernet_data_ptr_->dsap_, ethernet_data_ptr_->ssap_, 
		ethernet_data_ptr_->frame_format_, ethernet_data_ptr_->snd_seq_number_, 
		ethernet_data_ptr_->rcv_seq_number_);*/

	pdu_ptr_ = llc_pdu_->inner_pdu();
	return ParserSelector(pdu_ptr_, ed);
}

bool NetworkPacketParser::ParseEthernetII(Tins::EthernetII& ethii_pdu, EthernetData* const ed)
{
	// See possible Ethernet II types at https://en.wikipedia.org/wiki/Ethernet_frame

	ed->dst_mac_addr_ = ethii_pdu.dst_addr().to_string();
	ed->src_mac_addr_ = ethii_pdu.src_addr().to_string();
	ed->ethertype_ = ethii_pdu.payload_type();

	pdu_ptr_ = ethii_pdu.inner_pdu();
	//pdu_type_val_ = static_cast<uint16_t>(pdu_ptr_->pdu_type());
	//SPDLOG_INFO("EthernetII inner pdu is {:s}", pdu_type_to_name_map_.at(pdu_type_val_));
	return ParserSelector(pdu_ptr_, ed);
}

bool NetworkPacketParser::ParseIPv4(Tins::IP* ip_pdu, EthernetData* const ed)
{
	
	ed->src_ip_addr_ = ip_pdu->src_addr().to_string();
	ed->dst_ip_addr_ = ip_pdu->dst_addr().to_string();

	ed->id_ = ip_pdu->id();
	ed->protocol_ = ip_pdu->protocol();
	ed->offset_ = ip_pdu->fragment_offset();

	pdu_ptr_ = ip_pdu->inner_pdu();
	//SPDLOG_INFO("IP inner pdu is {:s}", pdu_type_to_name_map_.at(pdu_type_val_));

	/*printf("IPv4: srcaddr %s, dstaddr %s, id %hu, protocol %hhu, offset %hu\n",
		ethernet_data_ptr_->src_ip_addr_.c_str(), ethernet_data_ptr_->dst_ip_addr_.c_str(), 
		ethernet_data_ptr_->id_, ethernet_data_ptr_->protocol_, ethernet_data_ptr_->offset_);*/
	return ParserSelector(pdu_ptr_, ed);
}

bool NetworkPacketParser::ParseUDP(Tins::UDP* udp_pdu, EthernetData* const ed)
{
	
	ed->src_port_ = udp_pdu->sport();
	ed->dst_port_ = udp_pdu->dport();
	/*printf("UDP: src port %hu, dst port %hu\n", ethernet_data_ptr_->src_port_, 
		ethernet_data_ptr_->dst_port_);*/

	pdu_ptr_ = udp_pdu->inner_pdu();
	SPDLOG_INFO("UDP inner pdu is {:s}", pdu_type_to_name_map_.at(pdu_type_val_));
	return ParserSelector(pdu_ptr_, ed);
}

bool NetworkPacketParser::ParseRaw(Tins::RawPDU* raw_pdu, EthernetData* const ed)
{
	// Check payload length.
	ed->payload_size_ = raw_pdu->payload_size();

	//if (ethernet_data_ptr_->payload_size_ > EthernetData::mtu_)
	//{
	//	/*printf("NetworkPacketParser::ParseRaw(): Payload size exceeds MTU: %u\n",
	//		ethernet_data_ptr_->payload_size_);*/
	//	return 1;
	//}

	//// This copy is inefficient because the payload will be copied again into
	//// the vector from which the payloads will be eventually read into parquet.
	//// Much better to hold a pointer to the final payload vector, somehow retrieved from
	//// the ethernet parquet writing class, in the EthernetData object. Then data
	//// can be written/copied once.
	//std::copy(raw_pdu_->payload().begin(), raw_pdu_->payload().end(), 
	//	ethernet_data_ptr_->payload_ptr_);

	return true;
}

void NetworkPacketParser::PDUType()
{
	if (pdu_ptr_ != nullptr)
	{
		pdu_type_val_ = static_cast<uint16_t>(pdu_ptr_->pdu_type());
	}
	else
		pdu_type_val_ = 999;
	//printf("PDU type %hu = %s\n", pdu_type_val_, pdu_type_to_name_map_.at(pdu_type_val_).c_str());
}

bool NetworkPacketParser::ParserSelector(Tins::PDU* pdu_ptr, EthernetData* const ed)
{
	// If there is no inner_pdu(), the call will return a null pointer, which
	// may be passed to this function.
	if (pdu_ptr == nullptr)
	{
		printf("null pointer\n");
		return false;
	}

	switch (pdu_ptr->pdu_type())
	{
	case Tins::PDU::PDUType::LLC:
		llc_pdu_ = dynamic_cast<Tins::LLC*>(pdu_ptr);
		return ParseEthernetLLC(llc_pdu_, ed);
		break;
	case Tins::PDU::PDUType::RAW:
		raw_pdu_ = dynamic_cast<Tins::RawPDU*>(pdu_ptr);
		return ParseRaw(raw_pdu_, ed);
		break;
	case Tins::PDU::PDUType::IP:
		ip_pdu_ = dynamic_cast<Tins::IP*>(pdu_ptr);
		return ParseIPv4(ip_pdu_, ed);
		break;
	case Tins::PDU::PDUType::UDP:
		udp_pdu_ = dynamic_cast<Tins::UDP*>(pdu_ptr);
		return ParseUDP(udp_pdu_, ed);
		break;
	default:
		pdu_type_val_ = static_cast<uint16_t>(pdu_ptr->pdu_type());
		SPDLOG_WARN("PDU type not handled: {:s}",
			pdu_type_to_name_map_.at(pdu_type_val_));
		break;
	}

	return true;
}

void NetworkPacketParser::PrintPDUTypeNotHandled()
{
	printf("\n !!!!!! PDU type NOT HANDLED, %hu = %s\n", pdu_type_val_, 
		pdu_type_to_name_map_.at(pdu_type_val_).c_str());
}

