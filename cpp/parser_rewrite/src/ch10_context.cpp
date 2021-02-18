#include "ch10_context.h"

Ch10Context::Ch10Context(const uint64_t& abs_pos, uint16_t id) : absolute_position_(abs_pos),
	absolute_position(absolute_position_),
	tdp_rtc_(0), tdp_rtc(tdp_rtc_), tdp_abs_time_(0), tdp_abs_time(tdp_abs_time_),
	searching_for_tdp_(false), found_tdp_(false), pkt_type_config_map(pkt_type_config_map_),
	pkt_size_(0), pkt_size(pkt_size_), data_size_(0), data_size(data_size_), abs_time_(0),
	abs_time(abs_time_), rtc_(0), rtc(rtc_), rtc_to_ns_(100), thread_id_(id), thread_id(thread_id_),
	tdp_valid_(false), tdp_valid(tdp_valid_), tdp_doy_(0), tdp_doy(tdp_doy_), found_tdp(found_tdp_),
	intrapkt_ts_src_(0), intrapkt_ts_src(intrapkt_ts_src_), time_format_(0), time_format(time_format_)
{
	CreateDefaultPacketTypeConfig(pkt_type_config_map_);
}

Ch10Context::~Ch10Context()
{

}

void Ch10Context::SetSearchingForTDP(bool should_search)
{
	searching_for_tdp_ = should_search;
	found_tdp_ = false;
}

Ch10Status Ch10Context::ContinueWithPacketType(uint8_t data_type)
{
	// If the boolean searching_for_tdp_ is true then return false unless
	// the current packet is a "TMATS" (computer generated data, format 1) 
	// or TDP.
	if (searching_for_tdp_)
	{
		if (!found_tdp_)
		{
			if (data_type == static_cast<uint8_t>(Ch10PacketType::COMPUTER_GENERATED_DATA_F1))
			{
				printf("TMATS found!\n");
				return Ch10Status::PKT_TYPE_YES;
			}
			else if (data_type == static_cast<uint8_t>(Ch10PacketType::TIME_DATA_F1))
			{
				printf("TDP found!\n");
				found_tdp_ = true;
				return Ch10Status::PKT_TYPE_YES;
			}
			return Ch10Status::PKT_TYPE_NO;
		}
	}
	else
	{
		if (data_type == static_cast<uint8_t>(Ch10PacketType::TIME_DATA_F1))
		{
			return Ch10Status::PKT_TYPE_EXIT;
		}
	}

	// Check if the packet type is one that is configured to be parsed.
	//
	// NOt sure about the best way to do this. 
	
	return Ch10Status::PKT_TYPE_YES;
}

void Ch10Context::UpdateContext(const uint64_t& abs_pos, const uint32_t& pkt_size,
	const uint32_t& data_size, const uint32_t& rtc1, const uint32_t& rtc2, 
	uint8_t intrapkt_ts_source, uint8_t time_format)
{
	absolute_position_ = abs_pos;
	pkt_size_ = pkt_size;
	data_size_ = data_size;
	intrapkt_ts_src_ = intrapkt_ts_source;
	time_format_ = time_format;

	rtc_ = ((uint64_t(rtc2) << 32) + uint64_t(rtc1)) * rtc_to_ns_;
}

void Ch10Context::CreateDefaultPacketTypeConfig(std::unordered_map<Ch10PacketType, bool>& input)
{
	// Ensure that all elements defined in Ch10PacketType are present in the map 
	// and initialized to true, or turned on by default.
	input[Ch10PacketType::COMPUTER_GENERATED_DATA_F1] = true;
	input[Ch10PacketType::TIME_DATA_F1] = true;
	input[Ch10PacketType::MILSTD1553_F1] = true;
	input[Ch10PacketType::VIDEO_DATA_F0] = true;
}

void Ch10Context::SetPacketTypeConfig(const std::map<Ch10PacketType, bool>& user_config)
{
	// Loop over user map and only turn off or set to zero bits that correspond
	// to the packet types that are set to false in the user map.
	using MapIt = std::map< Ch10PacketType, bool>::const_iterator;
	for (MapIt it = user_config.cbegin(); it != user_config.cend(); ++it)
	{
		pkt_type_config_map_[it->first] = it->second;
	}

	// Regardless of current configuration after applying user config, set tmats
	// and time packets to true = on.
	pkt_type_config_map_[Ch10PacketType::COMPUTER_GENERATED_DATA_F1] = true;
	pkt_type_config_map_[Ch10PacketType::TIME_DATA_F1] = true;
}

void Ch10Context::UpdateWithTDPData(const uint64_t& tdp_abs_time, uint8_t tdp_doy,
	bool tdp_valid)
{
	tdp_valid_ = tdp_valid;

	// Do not update any values if tdp is not valid
	if (tdp_valid)
	{
		// This function should only be called by the TDP parser at 
		// the time it is parsed. If so, the RTC stored in this 
		// instance of Ch10Context is the TDP RTC. Re-assign it as such.
		tdp_rtc_ = rtc_;

		// Store the tdp absolute time.
		tdp_abs_time_ = tdp_abs_time;

		// Store the tdp doy.
		tdp_doy_ = tdp_doy;

		// Indicate that the TDP has been found.
		found_tdp_ = true;
	}
}