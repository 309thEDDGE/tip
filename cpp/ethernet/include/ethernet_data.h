
#ifndef ETHERNET_DATA_H
#define ETHERNET_DATA_H

#include <string>
#include <vector>
#include <cstdint>

// Class for storing relevant Ethernet data, including payload and sub-packet
// header information.
class EthernetData
{
public:
	// Payload
	static const size_t mtu_ = 1500;
	std::vector<uint8_t> payload_;
	uint8_t* payload_ptr_;
	uint32_t payload_size_;

	// MAC
	std::string dst_mac_addr_;
	std::string src_mac_addr_;

	// 802.3 length or EthernetII ethertype
	uint16_t ethertype_; // EtherType

	// LLC
	// 0 = information, 1 = supervisory, 2 = unnumbered (verify this)
	// Also called control field.
	uint8_t frame_format_; 
	uint8_t dsap_;
	uint8_t ssap_;
	uint8_t snd_seq_number_;
	uint8_t rcv_seq_number_;

	// IP
	std::string dst_ip_addr_;
	std::string src_ip_addr_;
	uint16_t id_;
	uint8_t protocol_;
	uint16_t offset_;

	// UDP
	uint16_t dst_port_;
	uint16_t src_port_;

	// Methods
	EthernetData();

};

#endif