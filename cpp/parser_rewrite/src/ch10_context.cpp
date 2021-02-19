#include "ch10_context.h"

Ch10Context::Ch10Context(const uint64_t& abs_pos, uint16_t id) : absolute_position_(abs_pos),
	absolute_position(absolute_position_),
	tdp_rtc_(0), tdp_rtc(tdp_rtc_), tdp_abs_time_(0), tdp_abs_time(tdp_abs_time_),
	searching_for_tdp_(false), found_tdp_(false), pkt_type_config_map(pkt_type_config_map_),
	pkt_size_(0), pkt_size(pkt_size_), data_size_(0), data_size(data_size_), abs_time_(0),
	abs_time(abs_time_), rtc_(0), rtc(rtc_), rtc_to_ns_(100), thread_id_(id), thread_id(thread_id_),
	tdp_valid_(false), tdp_valid(tdp_valid_), tdp_doy_(0), tdp_doy(tdp_doy_), found_tdp(found_tdp_),
	intrapkt_ts_src_(0), intrapkt_ts_src(intrapkt_ts_src_), time_format_(0), time_format(time_format_),
	channel_id_(0), channel_id(channel_id_), temp_rtc_(0), 
	chanid_remoteaddr1_map(chanid_remoteaddr1_map_), chanid_remoteaddr2_map(chanid_remoteaddr2_map_),
	chanid_commwords_map(chanid_commwords_map_), command_word1_(nullptr), command_word2_(nullptr)
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

void Ch10Context::UpdateContext(const uint64_t& abs_pos, const Ch10PacketHeaderFmt* const hdr_fmt_ptr)
{
	absolute_position_ = abs_pos;
	pkt_size_ = hdr_fmt_ptr->pkt_size;
	data_size_ = hdr_fmt_ptr->data_size;
	intrapkt_ts_src_ = hdr_fmt_ptr->intrapkt_ts_source;
	time_format_ = hdr_fmt_ptr->time_format;
	channel_id_ = hdr_fmt_ptr->chanID;

	rtc_ = ((uint64_t(hdr_fmt_ptr->rtc2) << 32) + uint64_t(hdr_fmt_ptr->rtc1)) * rtc_to_ns_;

	// If the channel ID to remote LRU address maps don't have a mapping for the
	// current channel id, then add it. 
	if (chanid_remoteaddr1_map_.count(channel_id_) == 0)
	{
		std::set<uint16_t> temp_set;
		chanid_remoteaddr1_map_[channel_id_] = temp_set;
		chanid_remoteaddr2_map_[channel_id_] = temp_set;

		std::set<uint32_t> temp_set2;
		chanid_commwords_map_[channel_id_] = temp_set2;
	}
	
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
		// RTC is already in units of nanoseconds.
		tdp_rtc_ = rtc_;

		// Store the tdp absolute time.
		tdp_abs_time_ = tdp_abs_time;

		// Store the tdp doy.
		tdp_doy_ = tdp_doy;

		// Indicate that the TDP has been found.
		found_tdp_ = true;
	}
}

uint64_t Ch10Context::CalculateAbsTimeFromRTCFormat(const uint32_t& rtc1,
	const uint32_t& rtc2)
{
	temp_rtc_ = ((uint64_t(rtc2) << 32) + uint64_t(rtc1)) * rtc_to_ns_;
	return tdp_abs_time_ + (temp_rtc_ - tdp_rtc_);
}

void Ch10Context::UpdateChannelIDToLRUAddressMaps(const uint32_t& chanid,
	const MilStd1553F1DataHeaderFmt* const data_header)
{
	// Set pointers for command words 1 and 2.
	const MilStd1553F1DataHeaderCommWordsFmt* comm_words =
		(const MilStd1553F1DataHeaderCommWordsFmt*)data_header;

	if (data_header->RR)
	{
		chanid_remoteaddr1_map_[chanid].insert(data_header->remote_addr1);
		chanid_remoteaddr2_map_[chanid].insert(data_header->remote_addr2);
		chanid_commwords_map_[chanid].insert(
			(uint32_t(comm_words->comm_word2) << 16) + comm_words->comm_word1);
	}
	else
	{
		chanid_remoteaddr1_map_[chanid].insert(data_header->remote_addr1);
		if (data_header->tx1)
			chanid_commwords_map_[chanid].insert(uint32_t(comm_words->comm_word1) << 16);
		else
			chanid_commwords_map_[chanid].insert(comm_words->comm_word1);
	}
}