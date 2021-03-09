#include "ch10_videof0_component.h"

Ch10Status Ch10VideoF0Component::Parse(const uint8_t*& data)
{
    ParseElements(csdw_elemement_vector_, data);
    Ch10VideoF0HeaderFormat csdw(**(csdw_element_.element));

    // Store packet absolute time 
    //?

    // Determine subpacket size
    uint16_t subpacket_size = TransportStream_UNIT_SIZE;
    if (csdw.IPH)  subpacket_size += ctx_->intrapacket_ts_size_;;

    // Check for integral number of packet components
    uint16_t packet_count = ctx_->data_size / subpacket_size;
    bool is_complete = (ctx_->data_size % subpacket_size == 0);
    if (!is_complete) return Ch10Status::VIDEOF0_NONINTEGER_SUBPKT_COUNT;

    // Check for too many elements

    // ctx_->videof0_pq_writer_.append_data(transport_stream_TS, ch10td_ptr_->doy_, 
	// 	ch10hd_ptr_->channel_id_, data_fmt_ptr_, subpkt_unit_count, transport_stream_data);
    return Ch10Status::OK;
}