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
#include <vector>
#include <map>
#include <iterator>
#include "managed_path.h"
#include "spdlog/spdlog.h"
#include "spdlog/fmt/bin_to_hex.h"

class NetworkPacketParser
{

	/*
	A note about usage of const Tins::PDU* vs (non-const) Tins::PDU*:

	Getters for the various PDU types (example: Tins::IP, Tins::LLC) are not
	declared as const, therefore can't be used with const Tins::PDU* pointers.
	This is the reason why non-const pointers are passed to the various parsers
	instead of using the safe and obvious option of const pointers.

	*/
private:

	// Current maximum raw payload length.
	uint32_t max_payload_size_;

	// Initial PDU types to create from Ch10 frame payloads
	Tins::Dot3 dot3_;
	Tins::EthernetII eth2_;

	// Temporary pointers for the various PDU types. These
	// will be casted from the base class, Tins::PDU.
	Tins::RawPDU* raw_pdu_;
	Tins::UDP* udp_pdu_;
	Tins::TCP* tcp_pdu_;
	Tins::IP* ip_pdu_;
	Tins::LLC* llc_pdu_;

	// Create class var here to avoid creation each time Parse is called
	bool parse_result_;

	bool pcap_output_enabled_;
	ManagedPath pcap_base_path_;

	// Tins::PDU::PDUType enum value to string mapping
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
	const uint32_t& max_payload_size;
	const bool& pcap_output_enabled;

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
		EthernetData* eth_data, const uint32_t& channel_id);

	/*
	Enable pcap file writing. Pcap files are placed in the *_ethernet.parquet
	directory alongside the *_ethernet__xxx.parquet files. File names have a 
	leading underscore to avoid conflict with parquet file readers that ingest
	the entire parquet directory. One pcap file is written per thread, per
	channel ID.

	Args:
		pq_output_file		--> The path for the thread-specific parquet
								file which is used to create the pcap
								output paths

	Return:
		The base pcap output file path.
	*/
	ManagedPath EnablePcapOutput(const ManagedPath& pq_output_file);

	/*
	Write pcap packet relevant to a specific channel ID and Tins PDU
	type 

	Args:
			pcap_base_path		--> Pcap output path from which to build
									channel ID and packet specific output
									pcap paths
			channel_id			--> Channel ID with which to tag the output
									file
			pdu_type			--> Tins pdu type enum indicates the type
									of PacketWriter to which the packet 
									ought to be written

	*/
	void WritePcapPacket(const ManagedPath& pcap_base_path,
		const uint32_t& channel_id, Tins::PDU::PDUType pdu_type);

	/*
	Helper function to simplify testing. Create an output pcap
	path relevant to a specific channel ID and Tins PDU type.

	Args:
			pcap_base_path		--> Pcap output path from which to build
									channel ID and packet specific output
									pcap paths
			channel_id			--> Channel ID with which to tag the output
									file
			pdu_type			--> Tins pdu type enum indicates the type
									of PacketWriter to which the packet
									ought to be written
	*/
	ManagedPath CreateSpecificPcapPath(const ManagedPath& pcap_base_path,
		const uint32_t& channel_id, Tins::PDU::PDUType pdu_type);

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
	virtual bool ParseEthernet(Tins::Dot3& dot3_pdu, EthernetData* const ed);

	
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
	virtual bool ParseEthernetLLC(Tins::LLC* llc_pdu, EthernetData* const ed);

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
	virtual bool ParseEthernetII(Tins::EthernetII& ethii_pdu, EthernetData* const ed);



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
	virtual bool ParseIPv4(Tins::IP* ip_pdu, EthernetData* const ed);



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
	virtual bool ParseUDP(Tins::UDP* udp_pdu, EthernetData* const ed);

	/*
	Parse Tins::PDU::PDUType::TCP

	Args:
		tcp_pdu		--> Tins::TCP PDU
		eth_data	--> Pointer to EthernetData object, a generic object
						in which data and metadata for multiple types of
						ethernet packets and sub-packets can be stored.
						Holds the output of the parsed packets.

	Return:
		True if no errors, false otherwise.
	*/
	virtual bool ParseTCP(Tins::TCP* tcp_pdu, EthernetData* const ed);



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
	virtual bool ParseRaw(Tins::RawPDU* raw_pdu, EthernetData* const ed,
		const uint32_t& max_pload_size);



	/************************************************************
						   Helper Functions
	*************************************************************/

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
	virtual bool ParserSelector(Tins::PDU* pdu_ptr, EthernetData* const ed);
};

#endif
