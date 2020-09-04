#ifndef NETWORK_PACKET_PARSER_H
#define NETWORK_PACKET_PARSER_H

#include <cstdint>
#include <cstdio>
#include <string>
//#include <map>
#include "tins/tins.h"

//enum class TinsPDUType : uint8_t
//{
//
//};

class NetworkPacketParser
{
private:
	const uint16_t* raw_payload_;
	uint16_t pdu_type_;
	const std::map<uint16_t, std::string> pdu_type_to_name_map_ = { {0, "RAW"}, {1, "ETHERNET_II"},
		{2,"IEEE802_3"}, {3,"RADIOTAP"}, {4, "DOT11"}, {5, "DOT11_ACK"}, {6, "DOT11_ASSOC_REQ"},
		{7, "DOT11_ASSOC_RESP"}, { 8, "DOT11_AUTH" }, { 9, "DOT11_BEACON" }, { 10, "DOT11_BLOCK_ACK" },
		{11, "DOT11_BLOCK_ACK_REQ"}, {12, "DOT11_CF_END"}, {13, "DOT11_DATA"}, {14, "DOT11_CONTROL"},
		{15, "DOT11_DEAUTH"}, {16, "OT11_DIASSOC"}, {17, "DOT11_END_CF_ACK"}, {18, "DOT11_MANAGEMENT"},
		{19, "DOT11_PROBE_REQ"}, {20, "DOT11_PROBE_RESP"}, {21, "DOT11_PS_POLL"}, {22, "DOT11_REASSOC_REQ"},
		{23, "DOT11_REASSOC_RESP"}, {24, "DOT11_RTS"}, {25, "DOT11_QOS_DATA"}, {26, "LLC"},
		{27, "SNAP"}, {28, "IP"}, {29, "ARP"}, {30, "TCP"}, {31, "UDP"}, {32, "ICMP"}, {33, "BOOTP"},
		{34, "DHCP"}, {35, "EAPOL"}, {36, "RC4EAPOL"}, {37, "RSNEAPOL"}, {38, "DNS"},
		{39, "LOOPBACK"}, {40, "IPv6"}, {41, "ICMPv6"}, {42, "SLL"}, {43, "DHCPv6"}, {44, "DOT1Q"},
		{45, "PPPOE"}, {46, "STP"}, {47, "PPI"}, {48, "IPSEC_AH"}, {49, "IPSEC_ESP"}, {50, "PKTAP"},
		{51, "MPLS"}, {999, "UNKNOWN"}, {1000, "USER_DEFINED_PDU"} };

public:
	NetworkPacketParser();

	// 802.3 (with total payload <= 1500), multiple sub-types possible.
	uint8_t ParseEthernet(const uint8_t* buffer, const uint32_t& tot_size);

	// 802.2 LLC
	uint8_t ParseEthernetLLC(const uint8_t* buffer, const uint32_t& tot_size);

	// Ethernet II (802.3 with total payload > 1500)
	uint8_t ParseEthernetII(const uint8_t* buffer, const uint32_t& tot_size);

	void PrintPDUType();
	void PrintPDUTypeNotHandled();
};

#endif
