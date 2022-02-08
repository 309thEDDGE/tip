#include "ch10_arinc429f0_component.h"

Ch10Status Ch10429F0Component::Parse(const uint8_t*& data)
{
    // Parse the 429F0 CSDW
    ParseElements(arinc429f0_csdw_elem_vec_, data);

    // Process each ARINC 429 with its intra-packet header which includes
    // the Gap Time from start of packet.
    status_ = ParseMessages((*arinc429f0_csdw_elem.element)->count, data);
    if (status_ != Ch10Status::OK)
        return status_;

    return Ch10Status::OK;
}

Ch10Status Ch10429F0Component::ParseMessages(const uint32_t& msg_count, const uint8_t*& data)
{
    // store the sum of gap times from IPDH's within the packet
    uint64_t total_gap_time = 0;

    // Iterate over arinc 429 messages (headers + data words) within the packet
    //uint16_t length = 0;
    for (msg_index_ = 0; msg_index_ < msg_count; msg_index_++)
    {
        // Parse the intra-packet data header and each
        // message payload. The data pointer will be updated to
        // the byte immediately following the current ARINC 429
        // header/word, which is the first byte of the next message.
        ParseElements(arinc429f0_data_hdr_wrd_elem_vec_, data);

        arinc429f0_msg_fmt_ptr_ =
            (const ARINC429F0MsgFmt*)(*arinc429f0_data_hdr_wrd_elem_.element);

        // Check for parity error
        if (arinc429f0_msg_fmt_ptr_->PE != 0)
            return Ch10Status::ARINC429F0_PARITY_ERROR;

        // Check for format error
        if (arinc429f0_msg_fmt_ptr_->FE != 0)
            return Ch10Status::ARINC429F0_FORMAT_ERROR;

        // Ensure Gap Time (units of 0.1 micro sec) from the IPDH is
        // not too large (<= 100 ms).
        if (arinc429f0_msg_fmt_ptr_->gap > 1000000)
            return Ch10Status::ARINC429F0_GAP_TIME_ERROR;

        total_gap_time += arinc429f0_msg_fmt_ptr_->gap;

        // update labels and bus numbers maps for metadata output
        ctx_->UpdateARINC429Maps(ctx_->channel_id, arinc429f0_msg_fmt_ptr_);

        // Calculate the absolute time using data that were obtained
        // from the intra packet headers' gap time and ch10 packet time.
        abs_time_ = ctx_->Calculate429WordAbsTime(total_gap_time);

        // Append parsed data to the file.
        ctx_->arinc429f0_pq_writer->Append(abs_time_, ctx_->tdp_doy, 
                                            arinc429f0_msg_fmt_ptr_, ctx_->channel_id);

    }

    return Ch10Status::OK;
}
