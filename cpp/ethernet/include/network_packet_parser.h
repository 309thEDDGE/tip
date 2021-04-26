#ifndef NETWORK_PACKET_PARSER_H
#define NETWORK_PACKET_PARSER_H

/*
There is a special order in which the following three
headers must be included if winsock2.h is used. Tins
includes these headers via the npcap (Windows version of
libpcap) library. Without spdlog and/or Arrow headers
tins is fine but each of those libs also include winsock2.h
and something about the order or maybe a partial include of 
the required three below is left out in those other headers,
while the macros are defined to avoid repeat includes, 
such that the complete set, in the proper order, is not 
included in libtins. When libtins loads these files, the 
macros defined to avoid repeat includes somehow fails and 
winsock.h is loaded again and redefinition errors occur.

Include the following three headers prior to tins.h.
*/

#ifdef __WIN64
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#endif 

#include "tins/tins.h"
#include "ethernet_data.h"
#include <cstdint>
#include <cstdio>
#include <string>
#include <algorithm> // copy
#include <map>

class NetworkPacketParser
{
private:
	EthernetData* ethernet_data_ptr_;
	//std::vector<uint8_t> temp_payload_;
	//std::vector<uint8_t>& raw_payload_;
	//uint32_t raw_payload_size_;
	uint16_t pdu_type_;
	Tins::PDU* pdu_ptr_;
	Tins::RawPDU* raw_pdu_;
	Tins::UDP* udp_pdu_;
	Tins::IP* ip_pdu_;
	Tins::LLC* llc_pdu_;
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
	

	/************************************************************
						 Data Link Layer
	*************************************************************/
	// 802.3 (with total payload <= 1500), multiple sub-types possible.
	uint8_t ParseEthernet(const uint8_t* buffer, const uint32_t& tot_size, EthernetData* ed);

	// 802.2 LLC
	uint8_t ParseEthernetLLC();

	// Ethernet II (802.3 with total payload > 1500)
	uint8_t ParseEthernetII(const uint8_t* buffer, const uint32_t& tot_size, EthernetData* ed);

	/************************************************************
	                       Network Layer
	*************************************************************/
	uint8_t ParseIPv4();

	/************************************************************
						   Transport Layer
	*************************************************************/
	uint8_t ParseUDP();

	/************************************************************
						   Application Layer
	*************************************************************/
	// Very basic in its interaction with libtins. Provides a common
	// function for recording data.
	uint8_t ParseRaw();

	void PDUType();
	void PrintPDUTypeNotHandled();
};

#endif
