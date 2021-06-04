#ifndef CH10_ETHERNETF0_COMPONENT_H_
#define CH10_ETHERNETF0_COMPONENT_H_

#include <cstdint>
#include <set>
#include "ch10_ethernetf0_msg_hdr_format.h"
#include "ch10_packet_component.h"
#include "managed_path.h"
#include "ch10_time.h"
#include "ethernet_data.h"
#include "network_packet_parser.h"

/*
This class defines the structures/classes and methods
to parse Ch10 "Ethernet Data Packets, Format 0".
*/

class Ch10EthernetF0Component : public Ch10PacketComponent
{
private:
	// Data structures to be parsed from the packet. Time structure
	// is not present because it will be parsed by the Ch10Time class.
	Ch10PacketElement<EthernetF0CSDW> ethernetf0_csdw_elem_;
	Ch10PacketElement<EthernetF0FrameIDWord> ethernetf0_frameid_elem_;

	// Vector of objects which inherit from Ch10PacketElement. In this
	// each one only holds one each of the objects above. This vector
	// is the input type for Ch10PacketComponent::ParseElements.
	ElemPtrVec ethernetf0_csdw_elem_vec_;
	ElemPtrVec ethernetf0_frameid_elem_vec_;

	Ch10Time ch10_time_;

	// Temporary holder for IPTS time
	uint64_t ipts_time_;

	// Hold absolute time of current message in units of nanoseconds
	// since the epoch.
	uint64_t abs_time_;

	// Allocate the variable for indexing the frames in a packet
	// once instead of each call to ParseFrames
	uint16_t frame_index_;

	// Length of bytes in Ethernet frame payload
	uint32_t data_length_;

	// Ethernet/MAC frame maximum length, including 4-byte
	// CRC. Not sure if "MAC frames" as specified by the ch10
	// spec includes the preamble or start frame delimiter.
	// (Appears that physical layer components are not included.)
	// For now include MAC addrs through CRC = 1522 bytes. 
	// Not sure if that is correct for Ethernet II frames, but
	// may be given the information here:
	// ttps://en.wikipedia.org/wiki/Ethernet_frame
	const uint32_t mac_frame_max_length_ = 65000; // 1522;

	// Object in which to store parsed ethernet frame data.
	EthernetData eth_data_;
	EthernetData* eth_data_ptr_;

	// Ethernet parsing class
	NetworkPacketParser eth_frame_parser_;

public:
	const Ch10PacketElement<EthernetF0CSDW>& ethernetf0_csdw_elem;
	const Ch10PacketElement<EthernetF0FrameIDWord>& ethernetf0_frameid_elem;

	// Maximum allowed frame count within a ch10 Ethernet Format 0
	// packet. This is not per the spec but a guess to filter
	// corrupt packets. Subject to change.
	const uint16_t max_frame_count_;

	Ch10EthernetF0Component(Ch10Context* const ch10ctx) : Ch10PacketComponent(ch10ctx),
		ethernetf0_csdw_elem_vec_{
			dynamic_cast<Ch10PacketElementBase*>(&ethernetf0_csdw_elem_) },
		ethernetf0_frameid_elem_vec_{
			dynamic_cast<Ch10PacketElementBase*>(&ethernetf0_frameid_elem_) },
		max_frame_count_(1000), ch10_time_(), ipts_time_(0), abs_time_(0),
		frame_index_(0), ethernetf0_csdw_elem(ethernetf0_csdw_elem_),
		ethernetf0_frameid_elem(ethernetf0_frameid_elem_), data_length_(0),
		eth_data_(), eth_data_ptr_(&eth_data_), eth_frame_parser_()
	{}

	/*
	Parse the EthernetF0 packet that is expected to be at the
	location in the buffer currently pointed by 'data'.

	Args:
		data		--> Pointer to buffer at which an EthernetF0 
						Ch10 packet is expected to reside

	Return:
		Ch10Status::OK if no problems, otherwise a different Ch10Status code.
	*/
	Ch10Status Parse(const uint8_t*& data) override;

	/*
	Parse all ethernet frames in the ch10 packet, including the 
	intra-packet header prior to each frame, which indludes a time stamp
	and frame ID word.

	Args:
		cdsw_ptr		--> Pointer to previously parsed EthernetF0CSDW
		ch10_context_ptr--> Pointer to Ch10Context which conveys the ch10
							parse state
		npp_ptr			--> Pointer to NetworkPacketParser object. This 
							pointed object will be used to parse the 
							frame. Passing this in to facilitate mocking.
		ch10_time_ptr	--> Ch10Time object with which to calculate time
		data_ptr		--> Buffer location of the next unparsed bytes
							which ought to be immediately following the
							CDSW

	Return:
		Ch10Status::OK if no problems, otherwise a different Ch10Status code.
	*/
	Ch10Status ParseFrames(const EthernetF0CSDW* const csdw_ptr,
		Ch10Context* const ch10_context_ptr, NetworkPacketParser* npp_ptr,
		Ch10Time* const ch10_time_ptr, const uint8_t*& data_ptr);

	/*
	Enable pcap file writing. See method with same name in 
	NetworkPacketParser.

	Args:
		pq_output_file		--> The path for the thread-specific parquet
								file which is used to create the pcap
								output paths
	*/
	void EnablePcapOutput(const ManagedPath& pq_output_file);

};

#endif