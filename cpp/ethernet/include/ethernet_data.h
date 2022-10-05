
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
    // Maximum length of a standard 802.3 frame. Note that an EthernetII
    // frame has ethertype in what is typically the length field
    // and specifies that the type will be greater than 1500. It
    // gives a way of indicating EthernetII types packets and does
    // not actually indicate the length of the packet, the maximum
    // of which remains at 1500.
    static const size_t mtu_ = 1500;

    // Maximum practical length of ethernet frame, not to be used
    // to check if 802.3 or EthernetII.
    static const size_t max_eth_frame_size_ = 1518;

    // Maximum TCP segment length, often call the MSS or Maximum
    // Segment Size:
    // Max Eth frame size - typical IPv4 header (20) - typical TCP header (32,
    // but could be 20 if older)
    // Using 32 bits because this is likely the data type that will be returned
    // because the TCP options spec specifies 4 bytes.
    static const uint32_t max_tcp_payload_size_ = max_eth_frame_size_ - 20 - 32;

    // Maximum UDP length:
    // Max Eth frame size - typical IPv4 header (20) - typical UDP header (8)
    // The length field is 2 bytes. Here we use 4 bytes to simplify
    // passing of maximum lengths into functions.
    static const uint32_t max_udp_payload_size_ = max_eth_frame_size_ - 20 - 8;

    // Payload size needs to accommodate TCP or UDP, possibly more types
    // in the future.
    static const uint32_t max_payload_size_ = (max_tcp_payload_size_ > max_udp_payload_size_ ? max_tcp_payload_size_ : max_udp_payload_size_);

    // Payload
    std::vector<uint8_t> payload_;
    uint8_t* payload_ptr_;
    uint32_t payload_size_;

    // MAC
    std::string dst_mac_addr_;
    std::string src_mac_addr_;

    // 802.3 length or EthernetII ethertype
    uint16_t ethertype_;  // EtherType

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

    // UDP/TCP
    uint16_t dst_port_;
    uint16_t src_port_;

    // Methods
    EthernetData();

    /*
	Re-assign member var values to defaults. To be used prior to 
	assignment of values during parsing to avoid confusion around 
	previously assigned values.
	*/
    void Reset();
};

#endif