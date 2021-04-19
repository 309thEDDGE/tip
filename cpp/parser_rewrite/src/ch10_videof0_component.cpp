#include "ch10_videof0_component.h"

Ch10Status Ch10VideoF0Component::Parse(const uint8_t*& data)
{
    // Parse channel-specific data word
    ParseElements(csdw_element_vector_, data);
    uint32_t subpacket_size = GetSubpacketSize((*csdw_element.element)->IPH);

    /*(signed)*/int16_t subpacket_count = DivideExactInteger(
        ctx_->data_size - csdw_element.size, subpacket_size);
    if (subpacket_count <= 0) 
    {
        SPDLOG_WARN("({:02d}) subpacket_count <= 0 ({:d}), subpacket_size = {:d}, "
            "data_size = {:d}, CSDW element size = {:d}",
            ctx_->thread_id, subpacket_count, subpacket_size, ctx_->data_size, 
            csdw_element.size);
        return Ch10Status::VIDEOF0_NONINTEGER_SUBPKT_COUNT;
    }

    if (subpacket_count > MAX_TransportStream_UNITS)
    {
        SPDLOG_ERROR("({:02d}) subpacket_count = {:d}, greater than "
            "MAX_TransportStream_UNITS ({:d})", ctx_->thread_id, subpacket_count,
            MAX_TransportStream_UNITS);
        return Ch10Status::VIDEOF0_SUBPKT_COUNT_BIG;
    }

    // Parse subpackets and send one-by-one to the writer
    for (int i = 0; i < subpacket_count; i++)
    {
        ParseSubpacket(data, (*csdw_element.element)->IPH, i);

        ctx_->videof0_pq_writer->append_data(subpacket_absolute_times_[i], ctx_->tdp_doy, ctx_->channel_id, 
            **csdw_element.element, **video_payload_element_.element);
    }
    
    return Ch10Status::OK;
}

void Ch10VideoF0Component::ParseSubpacket(const uint8_t*& data, bool iph, const int& pkt_index)
{
    
    subpacket_absolute_times_[pkt_index] = ParseSubpacketTime(data, iph);
    ParseElements(video_element_vector_, data);
}

uint64_t Ch10VideoF0Component::ParseSubpacketTime(const uint8_t*& data, bool iph)
{
    uint64_t time;
    if (iph)
    {
        ParseElements(time_stamp_element_vector_, data);
        time = ctx_->CalculateAbsTimeFromRTCFormat(
            (*time_stamp_element_.element)->ts1_, 
            (*time_stamp_element_.element)->ts2_);
    }
    else
    {
        time = ctx_->GetPacketAbsoluteTime();
    }
    return time;
}

uint32_t Ch10VideoF0Component::GetSubpacketSize(bool iph)
{
    uint16_t subpacket_size = TransportStream_UNIT_SIZE;
    if (iph)  subpacket_size += ctx_->intrapacket_ts_size_;
    return subpacket_size;
}

int32_t Ch10VideoF0Component::DivideExactInteger(uint32_t size_of_whole, uint32_t size_of_part)
{
    int32_t result = -1;

    // Check for integral number of packet components
    bool is_complete = (size_of_whole % size_of_part == 0);
    if (is_complete)   result = (int32_t) (size_of_whole / size_of_part);
    
    return result;
}
