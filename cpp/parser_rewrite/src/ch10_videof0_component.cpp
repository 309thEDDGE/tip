#include "ch10_videof0_component.h"

Ch10Status Ch10VideoF0Component::Parse(const uint8_t*& data)
{
    ParseElements(csdw_element_vector_, data);
    Ch10VideoF0HeaderFormat csdw(**(csdw_element_.element));

    // Store packet absolute time 
    //?

    // Determine subpacket size
    uint16_t subpacket_size = TransportStream_UNIT_SIZE;
    if (csdw.IPH)  subpacket_size += ctx_->intrapacket_ts_size_;;

    // Check for integral number of packet components
    uint16_t subpacket_count = ctx_->data_size / subpacket_size;
    bool is_complete = (ctx_->data_size % subpacket_size == 0);
    if (!is_complete) return Ch10Status::VIDEOF0_NONINTEGER_SUBPKT_COUNT;

    printf("Initializing local absolute_times vector\n");
   	std::vector<uint64_t> absolute_times(subpacket_count, 0);
    // printf("Setting class-level subpacket_absolute_times\n");
    subpacket_absolute_times_.resize(0);
    subpacket_absolute_times_.resize(subpacket_count, 0);
    printf("Calculating time\n");
    uint64_t TTT_time = ctx_->CalculateAbsTimeFromRTCNanoseconds(ctx_->rtc);
    printf("Setting element 0\n");
    subpacket_absolute_times_[0] = ctx_->CalculateAbsTimeFromRTCNanoseconds(ctx_->rtc);

    // for (size_t i =0; i < subpacket_count; i++)
    // {
    //     if (csdw.IPH)  absolute_times[i] = ctx_->CalculateAbsTimeFromRTCFormat()
    // }

    // if (absolute_times.size() == 0)
    // {
    //     uint64_t packet_absolute_time = ctx_->tdp_abs_time + ctx_->rtc - ctx_->tdp_rtc;
    //     absolute_times.push_back(packet_absolute_time);
    // }

   // ctx_->videof0_pq_writer_.append_data(transport_stream_TS, ch10td_ptr_->doy_, 
	// 	ch10hd_ptr_->channel_id_, data_fmt_ptr_, subpkt_unit_count, transport_stream_data);
    return Ch10Status::OK;
}