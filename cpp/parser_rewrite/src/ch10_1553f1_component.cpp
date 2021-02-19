#include "ch10_1553f1_component.h"

Ch10Status Ch101553F1Component::Parse(const uint8_t*& data, uint64_t& loc)
{
	// Parse the 1553F1 CSDW
	ParseElements(milstd1553f1_csdw_elem_vec_, data, loc);

	// Process each 1553 message. If the packet header intrapkt_ts_src is zero,
	// then process each message in the current packet with time stamp format
	// that conforms to the 48-bit RTC standard. Otherwise process each 8-byte
	// time stamp according to the packet header secondary time format (not implemented).
	if (ctx_->intrapkt_ts_src == 0)
	{
		status_ = ParseRTCTimeMessages((*milstd1553f1_csdw_elem_.element)->count,
			data, loc);
	}
	else
	{	
		printf("Ch101553F1Component::Parse(): Time stamp not handled!\n");
		return Ch10Status::MILSTD1553_TS_NOT_HANDLED;
		// TODO
		// switch on Ch10Context::time_format type, utilize other ParseXXTimeMessages
		// functions, which have not been created yet.
	}

	return Ch10Status::OK;
}

Ch10Status Ch101553F1Component::ParseRTCTimeMessages(const uint32_t& msg_count, 
	const uint8_t*& data, uint64_t& loc)
{
	// Iterate over messages
	for (msg_index_ = 0; msg_index_ < msg_count; msg_index_++)
	{
		// Parse the intra-packet time and header prior to each
		// message payload.
		ParseElements(milstd1553f1_rtctime_data_hdr_elem_vec_, data, loc);

		// Calculate the absolute time using data that were obtained
		// from the TDP.
		abs_time_ = ctx_->CalculateAbsTimeFromRTCFormat(
			(*milstd1553f1_rtctime_elem_.element)->ts1_,
			(*milstd1553f1_rtctime_elem_.element)->ts2_);

		// Update channel ID to remote address maps and the channel ID to 
		// command words integer map.
		ctx_->UpdateChannelIDToLRUAddressMaps(channel_id_,
			*milstd1553f1_data_hdr_elem_.element);

		// Fix tests related to the above function, check comm words map.
	}

	return Ch10Status::OK;
}