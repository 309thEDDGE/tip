
#ifndef CH10_CONTEXT_H_
#define CH10_CONTEXT_H_

#include <cstdint>
#include <cstdio>
#include <map>
#include <cmath>
#include "ch10_status.h"

enum class Ch10PacketType : uint8_t
{
	/*
	WARNING! Update CreatePacketTypeConfigReference with
	any packet types that are added to this enum! Also update
	relevant unit tests in ch10_context_u.cpp.
	*/
	COMPUTER_GENERATED_DATA_F1 = 0x01,
	TIME_DATA_F1               = 0x11,
	MILSTD1553_F1              = 0x19,
	VIDEO_DATA_F0              = 0x40,
};

class Ch10Context
{
private:
	uint64_t absolute_position_;
	uint64_t tdp_rtc_;
	uint64_t tdp_abs_time_;
	
	// Reference map for packet type on/off state.
	// This map must include all elements of Ch10PacketType to
	// ensure that all types can be configured for on/off correctly.
	const std::map<Ch10PacketType, uint64_t> pkt_type_config_reference_map_;

	bool searching_for_tdp_;
	bool found_tdp_;

	// Hold a bit representation of the packet type config for
	// comparison when choosing to parse a packet body.
	// This is initialized with uint64_max, so all packets are
	// initially "turned on". The user must call SetPacketTypeConfig
	// to configure.
	uint64_t pkt_type_config_;

public:
	const uint64_t& absolute_position;
	const uint64_t& tdp_rtc;
	const uint64_t& tdp_abs_time;
	const std::map<Ch10PacketType, uint64_t>& pkt_type_config_reference_map;
	const uint64_t& pkt_type_config;
	Ch10Context(const uint64_t& abs_pos);
	~Ch10Context();

	void SetSearchingForTDP(bool should_search);
	Ch10Status ContinueWithPacketType(uint8_t data_type);
	void UpdateAbsolutePosition(uint64_t new_absolute_pos);
	void CreatePacketTypeConfigReference(std::map<Ch10PacketType, uint64_t>& input);

	/*
	Use a user-input map of Ch10PacketType to bool and the bit values 
	associated with each Ch10PacketType to assemble the pkt_type_config_
	integer.

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