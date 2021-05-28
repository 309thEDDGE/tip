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
#include <exception>
#include <cstdint>
#include <cstdio>
#include <string>
#include <algorithm> // copy
#include <map>
#include "spdlog/spdlog.h"

class NetworkPacketParser
{
private:
	EthernetData* ethernet_data_ptr_;
	//std::vector<uint8_t> temp_payload_;
	//std::vector<uint8_t>& raw_payload_;
	//uint32_t raw_payload_size_;

	// 802.3 maximum payload length
	static const size_t mtu_ = 1500;

	// Hold PDUType enum and integer value.
	Tins::PDU::PDUType pdu_type_;
	uint16_t pdu_type_val_;

	// Initial PDU types to create from Ch10 frame payloads
	Tins::Dot3 dot3_;
	Tins::EthernetII eth2_;

	// 
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

	/*
	Parse an Ethernet/MAC frame from a Ch10 Ethernet packet. 

	Args:
		buffer		--> Pointer to the memory location of the beginning of the
						frame. Ought to be the beginning of the MAC frame 
						which is the first byte in the ch10 frame payload
						after the frame ID word

		length		--> Length of the frame in bytes. Same value as the 
						frame ID word data_length field.
		eth_data	--> Pointer to EthernetData object, a generic object
						in which data and metadata for multiple types of 
						ethernet packets and sub-packets can be stored.
						Holds the output of the parsed packets.

	Return:
		True if no errors, false otherwise.
	*/
	virtual bool Parse(const uint8_t* buffer, const uint32_t& length, 
		EthernetData* eth_data);
	


	/************************************************************
						 Data Link Layer
	*************************************************************/

	/*
	802.3 (with total payload <= 1500), multiple sub-types possible.

	Args:
		dot3_pdu	--> 802.3 PDU
		eth_data	--> Pointer to EthernetData object, a generic object
						in which data and metadata for multiple types of
						ethernet packets and sub-packets can be stored.
						Holds the output of the parsed packets.

	Return:
		True if no errors, false otherwise.
	*/
	virtual bool ParseEthernet(Tins::Dot3& dot3_pdu, EthernetData* ed);

	
	/*
	Parse Tins::PDU::PDUType::LLC == 802.2 LLC type packet

	Args:
		llc_pdu		--> IP PDU
		eth_data	--> Pointer to EthernetData object, a generic object
						in which data and metadata for multiple types of
						ethernet packets and sub-packets can be stored.
						Holds the output of the parsed packets.

	Return:
		True if no errors, false otherwise.
	*/
	virtual bool ParseEthernetLLC(Tins::LLC* llc_pdu, EthernetData* ed);

	/*
	Ethernet II (802.3 with length/Ethertype >= 1536)

	Args:
		dot3_pdu	--> 802.3 PDU
		eth_data	--> Pointer to EthernetData object, a generic object
						in which data and metadata for multiple types of
						ethernet packets and sub-packets can be stored.
						Holds the output of the parsed packets.

	Return:
		True if no errors, false otherwise.
	*/
	virtual bool ParseEthernetII(Tins::EthernetII& ethii_pdu, EthernetData* ed);



	/************************************************************
	                       Network Layer
	*************************************************************/

	/*
	Parse Tins::PDU::PDUType::IP == IPv4 type packet

	Args:
		ip_pdu		--> IP PDU
		eth_data	--> Pointer to EthernetData object, a generic object
						in which data and metadata for multiple types of
						ethernet packets and sub-packets can be stored.
						Holds the output of the parsed packets.

	Return:
		True if no errors, false otherwise.
	*/
	virtual bool ParseIPv4(Tins::IP* ip_pdu, EthernetData* ed);



	/************************************************************
						   Transport Layer
	*************************************************************/

	/*
	Parse Tins::PDU::PDUType::UDP

	Args:
		udp_pdu		--> Tins::UDP PDU
		eth_data	--> Pointer to EthernetData object, a generic object
						in which data and metadata for multiple types of
						ethernet packets and sub-packets can be stored.
						Holds the output of the parsed packets.

	Return:
		True if no errors, false otherwise.
	*/
	bool ParseUDP(Tins::UDP* udp_pdu, EthernetData* ed);



	/************************************************************
						   Application Layer
	*************************************************************/

	/*
	Parse Tins::PDU::PDUType::RAW

	Args:
		raw_pdu		--> Tins::RawPDU
		eth_data	--> Pointer to EthernetData object, a generic object
						in which data and metadata for multiple types of
						ethernet packets and sub-packets can be stored.
						Holds the output of the parsed packets.

	Return:
		True if no errors, false otherwise.
	*/
	bool ParseRaw(Tins::RawPDU* raw_pdu, EthernetData* ed);

	///
	/// Helper functions
	/// 
	
	/*
	Selector for the next parser. In this context, next
	means the Tins::PDU::inner_pdu() of the current
	PDU being parsed. Handle the case in which there 
	is no inner pdu and the return pointer is nullptr.

	Args:
		pdu_ptr		-> Pointer to Tins::PDU
		eth_data	--> Pointer to EthernetData object, a generic object
						in which data and metadata for multiple types of
						ethernet packets and sub-packets can be stored.
						Holds the output of the parsed packets.

	Return:
		True if no errors, false otherwise.
	*/
	bool ParserSelector(const Tins::PDU const* pdu_ptr, EthernetData const* ed);

	void PDUType();
	void PrintPDUTypeNotHandled();
};

#endif
