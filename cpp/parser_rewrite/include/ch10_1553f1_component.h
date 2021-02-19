
#ifndef CH10_1553F1_COMPONENT_H_
#define CH10_1553F1_COMPONENT_H_

#include <cstdint>
#include "ch10_1553f1_msg_hdr_format.h"
#include "ch10_packet_component.h"

class MilStd1553F1CSDWFmt
{
public:
	uint32_t count : 24;
	uint32_t : 6;
	uint32_t ttb : 2;
};

class MilStd1553F1DataRTCTimeStampFmt
{
public:
	uint64_t ts1_ : 32;
	uint64_t ts2_ : 32;
};

/*
This class defines the structures/classes and methods
to parse Ch10 "Mil-Std-1553 Bus Data Packets, Format 1".
*/

class Ch101553F1Component : public Ch10PacketComponent
{
private:

	Ch10PacketElement<MilStd1553F1CSDWFmt> milstd1553f1_csdw_elem_;
	Ch10PacketElement<MilStd1553F1DataRTCTimeStampFmt> milstd1553f1_rtctime_elem_;
	Ch10PacketElement<MilStd1553F1DataHeaderFmt> milstd1553f1_data_hdr_elem_;

	ElemPtrVec milstd1553f1_csdw_elem_vec_;
	ElemPtrVec milstd1553f1_rtctime_data_hdr_elem_vec_;

	// Index of the 1553 message being parsed within the ch10
	// 1553 packet.
	uint32_t msg_index_;

	// Hold absolute time of current message in units of nanoseconds
	// since the epoch.
	uint32_t abs_time_; 

	//
	// Vars for parsing the 1553 message payloads
	//

	// Interpret the raw bytes as uint16_t words
	//const uint16_t* payload_ptr_;

	// 1 if payload word count from command word is greater than
	// the calculated payload word count from the message length,
	// 0 otherwise.
	//uint8_t is_payload_incomplete_;

	// Calculated payload word count from the message length,
	// subtracting the command and status words.
	//int8_t calc_payload_word_count_;


public:

	const Ch10PacketElement<MilStd1553F1CSDWFmt>& milstd1553f1_csdw_elem;
	const Ch10PacketElement<MilStd1553F1DataRTCTimeStampFmt>& milstd1553f1_rtctime_elem;
	const Ch10PacketElement<MilStd1553F1DataHeaderFmt>& milstd1553f1_data_hdr_elem;

	Ch101553F1Component(Ch10Context* const ch10ctx) : Ch10PacketComponent(ch10ctx),
		milstd1553f1_csdw_elem_vec_{ 
			dynamic_cast<Ch10PacketElementBase*>(&milstd1553f1_csdw_elem_) },
		milstd1553f1_rtctime_data_hdr_elem_vec_{
			dynamic_cast<Ch10PacketElementBase*>(&milstd1553f1_rtctime_elem_),
			dynamic_cast<Ch10PacketElementBase*>(&milstd1553f1_data_hdr_elem_) },
		milstd1553f1_csdw_elem(milstd1553f1_csdw_elem_),
		milstd1553f1_rtctime_elem(milstd1553f1_rtctime_elem_),
		milstd1553f1_data_hdr_elem(milstd1553f1_data_hdr_elem_),
		msg_index_(0), abs_time_(0)
	{}
	Ch10Status Parse(const uint8_t*& data, uint64_t& loc) override;

	/*
		Need to add input vectors for insert remote addrs to chanid map.
	*/
	Ch10Status ParseRTCTimeMessages(const uint32_t& msg_count, 
		const uint8_t*& data, uint64_t& loc);
};

#endif