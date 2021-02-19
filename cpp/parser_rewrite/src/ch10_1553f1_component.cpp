#include "ch10_1553f1_component.h"

Ch10Status Ch101553F1Component::Parse(const uint8_t*& data, uint64_t& loc)
{
	// Parse the 1553F1 CSDW
	ParseElements(milstd1553f1_csdw_elem_vec_, data, loc);

	// Is the data corrupted such that the msg_count is too large?
	// This check may be superfluous because it was originally implemented
	// prior to checking the data checksum and corruption issues may now
	// be caught earlier. If this check never fails, remove it to boost
	// efficiency.
	if ((*milstd1553f1_csdw_elem_.element)->count > max_message_count_)
	{
		printf("Ch101553F1Component::Parse(): CSDW message count = %hu. Data may be corrupt!\n",
			(*milstd1553f1_csdw_elem_.element)->count);
		return Ch10Status::MILSTD1553_MSG_COUNT;
	}

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
		// message payload. The data pointer will be updated to
		// the byte immediately following the header, which is the 
		// first byte of the payload.
		ParseElements(milstd1553f1_rtctime_data_hdr_elem_vec_, data, loc);

		// Calculate the absolute time using data that were obtained
		// from the TDP.
		abs_time_ = ctx_->CalculateAbsTimeFromRTCFormat(
			(*milstd1553f1_rtctime_elem_.element)->ts1_,
			(*milstd1553f1_rtctime_elem_.element)->ts2_);

		//ParsePayload, fill vars necessary for adding data to parquet
		// then call the append_data function of the parquet writer here.
		//ParsePayload()

		// Update channel ID to remote address maps and the channel ID to 
		// command words integer map.
		ctx_->UpdateChannelIDToLRUAddressMaps(ctx_->channel_id,
			*milstd1553f1_data_hdr_elem_.element);

		
	}

	return Ch10Status::OK;
}

Ch10Status Ch101553F1Component::ParsePayload(const uint8_t*& data,
	const MilStd1553F1DataHeaderFmt* const data_header)
{
	// Check if the data length is too long to make sense, i.e., if it exceeds
	// a max of (32 payload words + 2 command words + 2 status words) 
	// * 2 bytes per word = 72 bytes.
	if (data_header->length > max_byte_count_)
	{
		printf("Ch101553F1Component::ParsePayload(): payload length (%hu) > %hu\n",
			data_header->length, max_byte_count_);
		return Ch10Status::MILSTD1553_MSG_LENGTH;
	}

	expected_payload_word_count_ = GetWordCountFromDataHeader();

	return Ch10Status::OK;
}

uint16_t Ch101553F1Component::GetWordCountFromDataHeader(
	const MilStd1553F1DataHeaderFmt* const data_header)
{
	// test this function, implement in parsePayload
	return 0;
}