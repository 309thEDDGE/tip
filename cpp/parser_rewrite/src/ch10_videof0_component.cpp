#include "ch10_videof0_component.h"

Ch10Status Ch10VideoF0Component::Parse(const uint8_t*& data)
{
    // Parse channel-specific data word
    ParseElements(csdw_element_vector_, data);
    uint32_t subpacket_size = GetSubpacketSize((*csdw_element.element)->IPH);

    /*(signed)*/ int16_t subpacket_count = DivideExactInteger(
        ctx_->data_size - csdw_element.size, subpacket_size);
    if (subpacket_count <= 0)
    {
        SPDLOG_WARN(
            "({:02d}) subpacket_count <= 0 ({:d}), subpacket_size = {:d}, "
            "data_size = {:d}, CSDW element size = {:d}",
            ctx_->thread_id, subpacket_count, subpacket_size, ctx_->data_size,
            csdw_element.size);
        return Ch10Status::VIDEOF0_NONINTEGER_SUBPKT_COUNT;
    }

    if (subpacket_count > MAX_TransportStream_UNITS)
    {
        SPDLOG_ERROR(
            "({:02d}) subpacket_count = {:d}, greater than "
            "MAX_TransportStream_UNITS ({:d})",
            ctx_->thread_id, subpacket_count,
            MAX_TransportStream_UNITS);
        return Ch10Status::VIDEOF0_SUBPKT_COUNT_BIG;
    }

    // Parse subpackets and send one-by-one to the writer
    for (size_t i = 0; i < subpacket_count; i++)
    {
        status_ = ParseSubpacket(data, (*csdw_element.element)->IPH, i);
        if (status_ != Ch10Status::OK)
            return status_;

        ctx_->videof0_pq_writer->Append(subpacket_absolute_times_[i], ctx_->tdp_doy, ctx_->channel_id,
                                             **csdw_element.element, **video_payload_element_.element);
    }

    return Ch10Status::OK;
}

Ch10Status Ch10VideoF0Component::ParseSubpacket(const uint8_t*& data, bool iph,
                                                const size_t& subpacket_index)
{
    if (iph)
    {
        // Parse the time component of the intra - packet header.The
        // data pointer will updated to the position immediately following
        // the time bytes block. In the case of video data, format 0, the
        // intra-packet header consists of only the time block.
        status_ = ch10_time_.ParseIPTS(data, ipts_time_, ctx_->intrapkt_ts_src,
                                       ctx_->time_format);
        if (status_ != Ch10Status::OK)
            return status_;

        // Calculate the absolute time using data that were obtained
        // from the IPTS and TDP data that were previously recorded in
        // the context.
        subpacket_absolute_times_[subpacket_index] = ctx_->CalculateIPTSAbsTime(ipts_time_);
    }
    else
    {
        subpacket_absolute_times_[subpacket_index] = ctx_->GetPacketAbsoluteTimeFromHeaderRTC();
    }

    // Parse the transport stream packet
    ParseElements(video_element_vector_, data);

    return Ch10Status::OK;
}

uint32_t Ch10VideoF0Component::GetSubpacketSize(bool iph)
{
    uint16_t subpacket_size = TransportStream_UNIT_SIZE;
    if (iph) subpacket_size += ctx_->intrapacket_ts_size_;
    return subpacket_size;
}

int32_t Ch10VideoF0Component::DivideExactInteger(uint32_t size_of_whole, uint32_t size_of_part)
{
    int32_t result = -1;

    // Check for integral number of packet components
    bool is_complete = (size_of_whole % size_of_part == 0);
    if (is_complete) result = (int32_t)(size_of_whole / size_of_part);

    return result;
}
