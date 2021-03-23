#include "ch10_videof0_component.h"

void print_location(int loc=-1)
{
    static int i=0;
    if (loc >= 0) i = loc;
    printf("location %d\n", i++);
}

Ch10Status Ch10VideoF0Component::Parse(const uint8_t*& data)
{
    /****/print_location(0);
    ParseElements(csdw_element_vector_, data);
    /****/print_location();
    Ch10VideoF0HeaderFormat csdw(**(csdw_element_.element));
    /****/print_location();

    uint32_t subpacket_size = GetSubPacketSize((*csdw_element_.element)->IPH);

    uint16_t subpacket_count = GetSubPacketCount(subpacket_size);

    if (subpacket_size == 0)
    {
        return Ch10Status::VIDEOF0_NONINTEGER_SUBPKT_COUNT;
    }

    subpacket_absolute_times_.resize(subpacket_count, 0);

    // Set time element 0 to the main packet time in case intra-packet time headers are not present
    subpacket_absolute_times_.push_back( ctx_->CalculateAbsTimeFromRTCNanoseconds(ctx_->rtc) );
    
    // If intra-packet headers are present, calculate absolute
    // time for each subpacket
    // if (csdw.IPH)
    // {
    //     uint64_t subpacket_time;
    //     uint8_t* subpacket_ptr = (uint8_t*)data; 
    //     uint64_t* rtc_ptr;
    //     /*************/ print_location(0);
    //     for (int i = 0; i < subpacket_count; i++)
    //     {
    //         if (csdw.IPH)  
    //         {
    //             rtc_ptr = (uint64_t*) subpacket_ptr;
    //             subpacket_absolute_times_[i] = ctx_->CalculateAbsTimeFromRTCFormat(*rtc_ptr, *(rtc_ptr+1));
    //         }

    //         subpacket_ptr += subpacket_size;
    //     }
    // }

   // ctx_->videof0_pq_writer_.append_data(transport_stream_TS, ch10td_ptr_->doy_, 
	// 	ch10hd_ptr_->channel_id_, data_fmt_ptr_, subpkt_unit_count, transport_stream_data);
    return Ch10Status::OK;
}

uint32_t Ch10VideoF0Component::GetSubPacketSize(bool iph)
{
    uint16_t subpacket_size = TransportStream_UNIT_SIZE;
    if (iph)  subpacket_size += ctx_->intrapacket_ts_size_;
    return subpacket_size;
}

uint32_t Ch10VideoF0Component::GetSubPacketCount(uint32_t subpacket_size)
{
    // // Check for integral number of packet components
    // bool is_complete = (ctx_->data_size % subpacket_size == 0);
    // if (!is_complete) return 0;
    return 0xFFFF;
}