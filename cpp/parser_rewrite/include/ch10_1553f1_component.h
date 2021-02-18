
#ifndef CH10_1553F1_COMPONENT_H_
#define CH10_1553F1_COMPONENT_H_

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

class MilStd1553F1DataHeaderFmt
{
public:
	uint16_t : 3;
	uint16_t WE : 1; // invalid word error
	uint16_t SE : 1; // sync type error
	uint16_t WCE : 1; // word count error 
	uint16_t : 3;
	uint16_t TO : 1; // response time out
	uint16_t FE : 1; // format error 
	uint16_t RR : 1; // RT to RT transfer
	uint16_t ME : 1; // message error
	uint16_t bus_dir : 1; // 0 = msg from chan A, 1 = msg from chan B
	uint16_t : 0;
	uint16_t gap1 : 8; // time from command/data word to first and only status
	uint16_t gap2 : 8; // time from last data word and second status word
	uint16_t length : 16; // total bytes in the message (command, data, status)
	uint16_t word_count1 : 5; // command word, N transmitted/requested messages
	uint16_t sub_addr1 : 5; // command word, sub address location
	uint16_t tx1 : 1; // command word, message is for remote to transmit
	uint16_t remote_addr1 : 5; // command word, remote LRU addr.
	uint16_t word_count2 : 5; // command word, N transmitted/requested messages
	uint16_t sub_addr2 : 5; // command word, sub address location
	uint16_t tx2 : 1; // command word, message is for remote to transmit
	uint16_t remote_addr2 : 5; // command word, remote LRU addr.
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

	// Reference to a vector 

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
		msg_index_(0)
	{}
	Ch10Status Parse(const uint8_t*& data, uint64_t& loc) override;

	/*
		Need to add input vectors for insert remote addrs to chanid map.
	*/
	Ch10Status ParseRTCTimeMessages(const uint32_t& msg_count, 
		const uint8_t*& data, uint64_t& loc);
};

#endif