#include "ch10_ethernetf0_component.h"

Ch10Status Ch10EthernetF0Component::Parse(const uint8_t*& data)
{
	// Parse the EthernetF0 CSDW
	ParseElements(ethernetf0_csdw_elem_vec_, data);

	// Check if the data is corrupt by filtering packets in 
	// which the frame_count from the CSDW is too large, or impractically
	// large. This value is a guess and subject to change.
	if ((*ethernetf0_csdw_elem_.element)->frame_count > max_frame_count_)
	{
		SPDLOG_WARN("Frame count ({:d}) > maximum allowed count ({:d})",
			(*ethernetf0_csdw_elem_.element)->frame_count, max_frame_count_);
		return Ch10Status::ETHERNETF0_FRAME_COUNT;
	}

	// Process each ethernet frame with its intra-packet header, including 
	// the time stamp and frame ID word.
	status_ = ParseFrames(*ethernetf0_csdw_elem_.element, ctx_, &ch10_time_, data);
	if (status_ != Ch10Status::OK)
		return status_;

	return Ch10Status::OK;
}

Ch10Status Ch10EthernetF0Component::ParseFrames(const EthernetF0CSDW* const csdw_ptr,
	Ch10Context* const ch10_context_ptr, Ch10Time* const ch10_time_ptr,
	const uint8_t*& data_ptr)
{
	// Iterate over all frame + IPH sub-packets
	for (frame_index_ = 0; frame_index_ < csdw_ptr->frame_count; frame_index_++)
	{
		// Parse the IPTS (buffer advanced automatically)
		status_ = ch10_time_ptr->ParseIPTS(data_ptr, ipts_time_,
			ch10_context_ptr->intrapkt_ts_src, ch10_context_ptr->time_format);
		if (status_ != Ch10Status::OK)
			return status_;

		// Calculate the absolute time.
		abs_time_ = ch10_context_ptr->CalculateIPTSAbsTime(ipts_time_);

		// Parse the frame ID word (buffer advanced automatically)
		ParseElements(ethernetf0_frameid_elem_vec_, data_ptr);

		// Parse the payload

		// Increment the data pointer by the size in bytes of 
		// the frame.
		data_ptr += (*ethernetf0_frameid_elem_.element)->data_length;
	}
	return Ch10Status::OK;
}