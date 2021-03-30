#include "ch10_videof0_component.h"

void print_location(int loc=-1)
{
    static int i=0;
    if (loc >= 0) i = loc;
    printf("location %d\n", i++);
}

Ch10Status Ch10VideoF0Component::Parse(const uint8_t*& data)
{
    ParseElements(csdw_element_vector_, data);
    const Ch10VideoF0HeaderFormat csdw = **csdw_element.element;
    uint32_t subpacket_size = GetSubpacketSize(csdw.IPH);

    /*(signed)*/int16_t subpacket_count = DivideExactInteger(ctx_->data_size, subpacket_size);
    if (subpacket_count <= 0) 
    {
        return Ch10Status::VIDEOF0_NONINTEGER_SUBPKT_COUNT;
    }

   // ctx_->videof0_pq_writer_.append_data(transport_stream_TS, ch10td_ptr_->doy_, 
	// 	ch10hd_ptr_->channel_id_, data_fmt_ptr_, subpkt_unit_count, transport_stream_data);

    // ctx_->videof0_pq_writer->append_data(subpacket_absolute_times_, ctx_->tdp_doy, ctx_->channel_id, 
    //     &csdw, subpacket_count, video_payload_element_.element);

    return Ch10Status::OK;
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

void Ch10VideoF0Component::ParseSubpacket(const uint8_t*& data, bool iph)
{
    subpacket_absolute_times_.push_back( ParseSubpacketTime(data, iph) );
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
