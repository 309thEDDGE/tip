#include "ethernet_data.h"

const size_t EthernetData::mtu_;
const size_t EthernetData::max_eth_frame_size_;
const uint32_t EthernetData::max_tcp_payload_size_;
const uint32_t EthernetData::max_udp_payload_size_;
const uint32_t EthernetData::max_payload_size_;

EthernetData::EthernetData() : payload_(max_payload_size_, 0), 
	payload_ptr_(payload_.data()),
dst_mac_addr_(""), src_mac_addr_(""), ethertype_(0), frame_format_(UINT8_MAX),
dsap_(UINT8_MAX), ssap_(UINT8_MAX), snd_seq_number_(UINT8_MAX), rcv_seq_number_(UINT8_MAX),
dst_ip_addr_(""), src_ip_addr_(""), id_(UINT16_MAX), protocol_(UINT8_MAX), offset_(UINT8_MAX),
dst_port_(0), src_port_(0), payload_size_(0)
{

}

void EthernetData::Reset()
{
	// The payload vector does not need to be resized or zeroed because
	// the count of valid payload values is copied into the final vector
	// from which arrow reads for creating the parquet file.
	payload_size_ = 0;

	dst_mac_addr_.clear();
	src_mac_addr_.clear();
	ethertype_ = 0;
	frame_format_ = 255; // 0 = information so use different value
	dsap_ = 0;
	ssap_ = 0;
	snd_seq_number_ = 0;
	rcv_seq_number_ = 0;
	dst_ip_addr_.clear();
	src_ip_addr_.clear();
	id_ = 0;
	protocol_ = 0;
	offset_ = 0;
	dst_port_ = 0;
	src_port_ = 0;
}