#include "network_packet_parser.h"

NetworkPacketParser::NetworkPacketParser() : raw_pdu_(nullptr),
udp_pdu_(nullptr), ip_pdu_(nullptr), llc_pdu_(nullptr), dot3_(), eth2_(), 
tcp_pdu_(nullptr), max_payload_size_(0), max_payload_size(max_payload_size_),
parse_result_(false), pcap_output_enabled_(false), 
pcap_output_enabled(pcap_output_enabled_)
{

}

bool NetworkPacketParser::Parse(const uint8_t* buffer, const uint32_t& length,
	EthernetData* eth_data, const uint32_t& channel_id)
{
	// Default to greatest payload size
	max_payload_size_ = EthernetData::max_payload_size_;

	// Determine if 802.3 or Ethernet II
	try
	{
		dot3_ = Tins::Dot3(buffer, length);
		if (dot3_.length() > EthernetData::mtu_)
		{
			eth2_ = Tins::EthernetII(buffer, length);
			SPDLOG_DEBUG("Parsing EthernetII");
			parse_result_ = ParseEthernetII(eth2_, eth_data);

			if (pcap_output_enabled_)
			{
				WritePcapPacket(pcap_base_path_, channel_id, Tins::PDU::PDUType::ETHERNET_II,
					pcap_writer_map_, dynamic_cast<Tins::PDU*>(&eth2_));
			}
		}
		else
		{
			SPDLOG_DEBUG("Parsing 802.3");
			parse_result_ = ParseEthernet(dot3_, eth_data);

			if (pcap_output_enabled_)
			{
				WritePcapPacket(pcap_base_path_, channel_id, Tins::PDU::PDUType::DOT3,
					pcap_writer_map_, dynamic_cast<Tins::PDU*>(&dot3_));
			}
		}

		//if (!parse_result_)
		//{
		//	/*std::vector<uint8_t> buff(buffer, buffer + length);*/
		//	//SPDLOG_DEBUG("buff size {:d}, length {:d}", int(buff.size()), length);
		//	SPDLOG_DEBUG("Ethernet buffer data: {:Xs}", spdlog::to_hex(buffer, buffer+length));
		//}
	}
	catch (const Tins::malformed_packet& e)
	{
		// Sometimes frame payloads from the ch10 can be "payload only",
		// which includes data from the payload portion of the 802.3 or 
		// EthernetII packet and not the MAC and Ethertype header or
		// the trailing CRC.
		/*try 
		{
			Tins::LLC llc(buffer, length);
			return ParseEthernetLLC(&llc, eth_data);
		}
		catch (const Tins::malformed_packet& e)
		{
			SPDLOG_WARN("Failed to interpret as LLC buffer: {:s}", e.what());
		}

		try
		{
			Tins::IP ip(buffer, length);
			return ParseIPv4(&ip, eth_data);
		}
		catch (const Tins::malformed_packet& e)
		{
			SPDLOG_WARN("Failed to interpret as IP buffer: {:s}", e.what());
		}*/

		SPDLOG_WARN("Error: {:s}", e.what());
		/*std::vector<uint8_t> buff(buffer, buffer + length);
		SPDLOG_DEBUG("buff size {:d}, length {:d}", int(buff.size()), length);
		SPDLOG_DEBUG("Ethernet buffer data: {:Xsn}", spdlog::to_hex(buff.begin(), buff.end()));*/
		return false;
	}
	
	return true;
}

bool NetworkPacketParser::ParseEthernet(Tins::Dot3& dot3_pdu, EthernetData* const ed)
{
	ed->dst_mac_addr_ = dot3_pdu.dst_addr().to_string();
	ed->src_mac_addr_ = dot3_pdu.src_addr().to_string();
	ed->ethertype_ = dot3_pdu.length();

	return ParserSelector(dot3_pdu.inner_pdu(), ed);
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

	return ParserSelector(llc_pdu->inner_pdu(), ed);
}

bool NetworkPacketParser::ParseEthernetII(Tins::EthernetII& ethii_pdu, EthernetData* const ed)
{
	// See possible Ethernet II types at https://en.wikipedia.org/wiki/Ethernet_frame

	ed->dst_mac_addr_ = ethii_pdu.dst_addr().to_string();
	ed->src_mac_addr_ = ethii_pdu.src_addr().to_string();
	ed->ethertype_ = ethii_pdu.payload_type();
	
	return ParserSelector(ethii_pdu.inner_pdu(), ed);
}

bool NetworkPacketParser::ParseIPv4(Tins::IP* ip_pdu, EthernetData* const ed)
{
	
	ed->src_ip_addr_ = ip_pdu->src_addr().to_string();
	ed->dst_ip_addr_ = ip_pdu->dst_addr().to_string();

	ed->id_ = ip_pdu->id();
	ed->protocol_ = ip_pdu->protocol();
	ed->offset_ = ip_pdu->fragment_offset();

	return ParserSelector(ip_pdu->inner_pdu(), ed);
}

bool NetworkPacketParser::ParseUDP(Tins::UDP* udp_pdu, EthernetData* const ed)
{
	ed->src_port_ = udp_pdu->sport();
	ed->dst_port_ = udp_pdu->dport();

	// Set max payload size
	max_payload_size_ = EthernetData::max_udp_payload_size_;

	return ParserSelector(udp_pdu->inner_pdu(), ed);
}

bool NetworkPacketParser::ParseTCP(Tins::TCP* tcp_pdu, EthernetData* const ed)
{
	ed->dst_port_ = tcp_pdu->dport();
	ed->src_port_ = tcp_pdu->sport();

	// Set max payload size
	max_payload_size_ = EthernetData::max_tcp_payload_size_;

	return ParserSelector(tcp_pdu->inner_pdu(), ed);
}

bool NetworkPacketParser::ParseRaw(Tins::RawPDU* raw_pdu, EthernetData* const ed,
	const uint32_t& max_pload_size)
{
	ed->payload_size_ = raw_pdu->payload_size();

	/*
	It's not clear at this time if storing the pointer is sufficient
	to locate the data. When a pointer to a buffer is initially passed
	to the Parse method and a Tins PDU object is constructed. It's not
	clear if the cascading containers (i.e., PDUs) are constructed all at
	once or on each inner_pdu call and whether the final raw data payload
	is lifted from the buffer into a payload_type (vector<uint8_t>) that
	is allocated on the heap or a vector is constructed such that it's first
	element coincides with the location in memory of the original buffer 
	raw payload. If the former scenario is true, then storing a pointer is 
	meaningless because the Tins object which holds the data will go out
	of scope as soon as Parse returns to the original caller. In that
	case, the final payload data will need to be copied.

	The test NetworkPacketParserTest.ParseRawHeaderAndPayloadInChain
	has shown that the RawPDU payload is not somehow constructed
	on top of the original memory location of the payload from 
	from the buffer. Instead it is copied into the RawPDU.payload()
	vector. Because of this, I must actually copy the payload data
	from the RawPDU into my EthernetData object.
	*/
	//ed->payload_ptr_ = raw_pdu->payload().data();

	// Check if the payload size exceeds the default mtu.
	if (ed->payload_size_ > max_pload_size)
	{
		SPDLOG_WARN("Payload size ({:d}) > max payload size ({:d})", ed->payload_size_,
			max_pload_size);
		return false;
	}

	// Copy the payload into the EthernetData payload vector
	std::copy(raw_pdu->payload().data(), raw_pdu->payload().data() + ed->payload_size_,
		ed->payload_ptr_);

	// Return true instead of calling ParserSelector on the inner_pdu because
	// there can be no inner_pdu for a RawPDU.
	return true;
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

	SPDLOG_DEBUG("Parsing {:s}", pdu_type_to_name_map_.at(
		static_cast<uint16_t>(pdu_ptr->pdu_type())));

	switch (pdu_ptr->pdu_type())
	{
	case Tins::PDU::PDUType::LLC:
		llc_pdu_ = dynamic_cast<Tins::LLC*>(pdu_ptr);
		return ParseEthernetLLC(llc_pdu_, ed);
		break;
	case Tins::PDU::PDUType::RAW:
		raw_pdu_ = dynamic_cast<Tins::RawPDU*>(pdu_ptr);
		return ParseRaw(raw_pdu_, ed, max_payload_size_);
		break;
	case Tins::PDU::PDUType::IP:
		ip_pdu_ = dynamic_cast<Tins::IP*>(pdu_ptr);
		return ParseIPv4(ip_pdu_, ed);
		break;
	case Tins::PDU::PDUType::UDP:
		udp_pdu_ = dynamic_cast<Tins::UDP*>(pdu_ptr);
		return ParseUDP(udp_pdu_, ed);
		break;
	case Tins::PDU::PDUType::TCP:
		tcp_pdu_ = dynamic_cast<Tins::TCP*>(pdu_ptr);
		return ParseTCP(tcp_pdu_, ed);
		break;
	default:
		SPDLOG_WARN("PDU type not handled: {:s}",
			pdu_type_to_name_map_.at(static_cast<uint16_t>(pdu_ptr->pdu_type())));
		break;
	}

	return true;
}

ManagedPath NetworkPacketParser::EnablePcapOutput(const ManagedPath& pq_output_file)
{
	ManagedPath file_name("_" + pq_output_file.filename().RawString());
	pcap_base_path_ = pq_output_file.parent_path().CreatePathObject(file_name,
		".pcap");

	pcap_output_enabled_ = true;

	return pcap_base_path_;
}

ManagedPath NetworkPacketParser::CreateSpecificPcapPath(const ManagedPath& pcap_base_path,
	const uint32_t& channel_id, Tins::PDU::PDUType pdu_type)
{
	ManagedPath output;
	
	char buff[100];
	std::snprintf(buff, 100, "_%s_chanid%02u.pcap", 
		pdu_type_to_name_map_.at(static_cast<uint16_t>(pdu_type)).c_str(), channel_id);
	std::string extension(buff);

	output = pcap_base_path.parent_path().CreatePathObject(
		pcap_base_path, extension);

	return output;
}

void NetworkPacketParser::WritePcapPacket(const ManagedPath& pcap_base_path,
	const uint32_t& channel_id, Tins::PDU::PDUType pdu_type, 
	std::unordered_map<uint32_t, std::unordered_map<
	Tins::PDU::PDUType, std::shared_ptr<Tins::PacketWriter>>>& pcap_writer_map, 
	Tins::PDU* pdu)
{
	if (AddPacketWriterToMap(pcap_base_path, channel_id, pdu_type, pcap_writer_map))
	{
		// Write the packet
		pcap_writer_map[channel_id][pdu_type]->write(pdu);
	}
}

bool NetworkPacketParser::AddPacketWriterToMap(const ManagedPath& pcap_base_path,
	const uint32_t& channel_id, Tins::PDU::PDUType pdu_type,
	std::unordered_map<uint32_t, std::unordered_map<
	Tins::PDU::PDUType, std::shared_ptr<Tins::PacketWriter>>>& pcap_writer_map)
{
	// Check if a writer for the given channel_id and pdu_type
	// already exists.
	if (pcap_writer_map.count(channel_id) == 0)
	{
		// Add an empty map for the given channel_id and create
		// a writer, then add it for the given pdu_type.
		std::unordered_map<Tins::PDU::PDUType, 
			std::shared_ptr<Tins::PacketWriter>> temp_pdu_writer_map;
		pcap_writer_map[channel_id] = temp_pdu_writer_map;
	}
	
	if (pcap_writer_map[channel_id].count(pdu_type) == 0)
	{
		ManagedPath pcap_path = CreateSpecificPcapPath(pcap_base_path, channel_id,
			pdu_type);

		switch (pdu_type)
		{
		case Tins::PDU::PDUType::DOT3:
			pcap_writer_map[channel_id][pdu_type] = std::make_shared<Tins::PacketWriter>(
				pcap_path.string(), Tins::DataLinkType<Tins::Dot3>());
			break;
		case Tins::PDU::PDUType::ETHERNET_II:
			pcap_writer_map[channel_id][pdu_type] = std::make_shared<Tins::PacketWriter>(
				pcap_path.string(), Tins::DataLinkType<Tins::EthernetII>());
			break;
		default:
			SPDLOG_WARN("No PacketWriter creation routine defined for PDU type {:s}",
				pdu_type_to_name_map_.at(static_cast<uint16_t>(pdu_type)));
			return false;
			break;
		}
	}
	return true;
}
