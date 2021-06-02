#include "ethernet_data.h"

EthernetData::EthernetData() : payload_(max_payload_size_, 0), 
	payload_ptr_(payload_.data()),
dst_mac_addr_(""), src_mac_addr_(""), ethertype_(0), frame_format_(UINT8_MAX),
dsap_(UINT8_MAX), ssap_(UINT8_MAX), snd_seq_number_(UINT8_MAX), rcv_seq_number_(UINT8_MAX),
dst_ip_addr_(""), src_ip_addr_(""), id_(UINT16_MAX), protocol_(UINT8_MAX), offset_(UINT8_MAX),
dst_port_(0), src_port_(0), payload_size_(0)
{

}