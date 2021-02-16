
#ifndef CH10_CONTEXT_H_
#define CH10_CONTEXT_H_

#include <cstdint>
#include <cstdio>
#include <map>
#include <unordered_map>
#include <cmath>
#include "ch10_status.h"

enum class Ch10PacketType : uint8_t
{
	/*
	WARNING! Update CreatePacketTypeConfigReference with
	any packet types that are added to this enum! Also update
	relevant unit tests in ch10_context_u.cpp.
	*/
	NONE					   = 0xFF,
	COMPUTER_GENERATED_DATA_F1 = 0x01,
	TIME_DATA_F1               = 0x11,
	MILSTD1553_F1              = 0x19,
	VIDEO_DATA_F0              = 0x40,
};

class Ch10Context
{
private:

	// ID to control or generate thread-specific log output.
	uint16_t thread_id_;

	/// <summary>
	/// Key components of Ch10 context.
	/// </summary>
	uint64_t absolute_position_;
	uint64_t tdp_rtc_;
	uint64_t tdp_abs_time_;
	uint64_t rtc_;
	uint32_t pkt_size_;
	uint32_t data_size_;
	uint64_t abs_time_;

	// Conversion factor for relative time counter (RTC)
	// to nanoseconds.
	// Unit: count/ns
	const uint64_t rtc_to_ns_;
	
	// Reference map for packet type on/off state.
	// This map must include all elements of Ch10PacketType to
	// ensure that all types can be configured for on/off correctly.
	std::unordered_map<Ch10PacketType, bool> pkt_type_config_map_;

	bool searching_for_tdp_;
	bool found_tdp_;

public:
	const uint16_t& thread_id;
	const uint64_t& absolute_position;
	const uint64_t& tdp_rtc;
	const uint64_t& tdp_abs_time;
	const uint64_t& rtc;
	const uint32_t& pkt_size;
	const uint32_t& data_size;
	const uint64_t& abs_time;
	const std::unordered_map<Ch10PacketType, bool>& pkt_type_config_map;
	Ch10Context(const uint64_t& abs_pos, uint16_t id = 0);
	~Ch10Context();

	void SetSearchingForTDP(bool should_search);
	Ch10Status ContinueWithPacketType(uint8_t data_type);

	/*
	Update the members that are of primary importance for conveyance
	to the packet body parsers, including re-calculation of the current
	packet absolute time based on TDP abs time, RTC, and the current packet
	RTC.

	Args:

		abs_pos		--> absolute byte position within the Ch10
		pkt_size	--> Total ch10 packet size in bytes, from Ch10PacketHeaderFmt::pkt_size
		data_size	--> Size in bytes of ch10 packet body, from Ch10PacketHeaderFmt::data_size
		rtc1/2		--> Two components of RTC, from from Ch10PacketHeaderFmt::rtc1/2

	*/
	void UpdateContext(const uint64_t& abs_pos, const uint32_t& pkt_size,
		const uint32_t& data_size, const uint32_t& rtc1, const uint32_t& rtc2);


	void CreateDefaultPacketTypeConfig(std::unordered_map<Ch10PacketType, bool>& input);

	/*
	Use a user-input map of Ch10PacketType to bool to assemble the 
	pkt_type_config_map_.

	Args:

		user_config --> map of Ch10PacketType to bool. For the user-submitted
		example map, 
			{Ch10PacketType::MILSTD1553_F1 --> true},
			{Ch10PacketType::VIDEO_DATA_F0 --> false}
		1553 will be parsed and video (type f0) will not be parsed. TMATS
		(computer generated data, format 1) and time data packets (time data f1)
		cannot be turned off. Data types that are not configured will default
		to true.
	*/
	void SetPacketTypeConfig(const std::map<Ch10PacketType, bool>& user_config);
};


#endif