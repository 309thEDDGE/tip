#include "ch10_1553f1_component.h"

Ch10Status Ch101553F1Component::Parse(const uint8_t*& data)
{
    // Parse the 1553F1 CSDW
    ParseElements(milstd1553f1_csdw_elem_vec_, data);

    // Is the data corrupted such that the msg_count is too large?
    // This check may be superfluous because it was originally implemented
    // prior to checking the data checksum and corruption issues may now
    // be caught earlier. If this check never fails, remove it to boost
    // efficiency.
    if ((*milstd1553f1_csdw_elem_.element)->count > max_message_count_)
    {
        /*uint32_t count = (*milstd1553f1_csdw_elem_.element)->count;
		printf("Ch101553F1Component::Parse(): CSDW message count = %u. Data may be corrupt!\n",
			count);*/
        return Ch10Status::MILSTD1553_MSG_COUNT;
    }

    // Process each 1553 message with its intra-packet header, including
    // the time stamp and data header.
    status_ = ParseMessages((*milstd1553f1_csdw_elem_.element)->count, data);
    if (status_ != Ch10Status::OK)
        return status_;

    return Ch10Status::OK;
}

Ch10Status Ch101553F1Component::ParseMessages(const uint32_t& msg_count, const uint8_t*& data)
{
    // Iterate over messages
    //uint16_t length = 0;
    for (msg_index_ = 0; msg_index_ < msg_count; msg_index_++)
    {
        // Parse the time component of the intra-packet header. The
        // data pointer will updated to the position immediately following
        // the time bytes block.
        status_ = ch10_time_.ParseIPTS(data, ipts_time_, ctx_->intrapkt_ts_src,
                                       ctx_->time_format);
        if (status_ != Ch10Status::OK)
            return status_;

        // Calculate the absolute time using data that were obtained
        // from the IPTS.
        abs_time_ = ctx_->CalculateIPTSAbsTime(ipts_time_);

        // Parse the intra-packet data header prior to each
        // message payload. The data pointer will be updated to
        // the byte immediately following the header, which is the
        // first byte of the payload.
        ParseElements(milstd1553f1_ip_data_hdr_elem_vec_, data);

        // Parse the payload. This function also checks for payload inconsistencies
        // so it is useful to call before updating the channel ID to LRU address
        // maps in case the message is corrupted.
        milstd1553f1_data_hdr_commword_ptr_ =
            (const MilStd1553F1DataHeaderCommWordFmt*)(*milstd1553f1_data_hdr_elem_.element);
        status_ = ParsePayload(data, milstd1553f1_data_hdr_commword_ptr_);
        if (status_ != Ch10Status::OK)
            return status_;

        // Update channel ID to remote address maps and the channel ID to
        // command words integer map.
        ctx_->UpdateChannelIDToLRUAddressMaps(ctx_->channel_id,
                                              milstd1553f1_data_hdr_commword_ptr_);

        data += (*milstd1553f1_data_hdr_elem_.element)->length;

        // Append parsed data to the file.
        ctx_->milstd1553f1_pq_writer->Append(abs_time_, ctx_->tdp_doy,
                                             *milstd1553f1_csdw_elem_.element, milstd1553f1_data_hdr_commword_ptr_,
                                             payload_ptr_, ctx_->channel_id, calc_payload_word_count_, is_payload_incomplete_);
    }

    return Ch10Status::OK;
}

Ch10Status Ch101553F1Component::ParsePayload(const uint8_t*& data,
                                             const MilStd1553F1DataHeaderCommWordFmt* data_header)
{
    // Check if the data length is too long to make sense, i.e., if it exceeds
    // a max of (32 payload words + 2 command words + 2 status words)
    // * 2 bytes per word = 72 bytes.
    if (data_header->length > max_byte_count_)
    {
        uint16_t length = data_header->length;
        SPDLOG_WARN("({:02d}) 1553 message payload length ({:d}) greater than maximum ({:d})",
                    ctx_->thread_id, length, max_byte_count_);
        return Ch10Status::MILSTD1553_MSG_LENGTH;
    }

    expected_payload_word_count_ = GetWordCountFromDataHeader(data_header);

    // Set the payload pointer to the position of data pointer.
    payload_ptr_ = (const uint16_t*)data;

    // Calculate the message payload count from the message length.
    // We are interested in calculating the payload count to know if
    // it contains fewer words than expected from the command word.
    // In the case of BC to RT and RT to RT the series of data words
    // is followed by a status word. Because the 1553 data are temporally
    // scheduled, if the 1553 message is short one word it must be the
    // final word, or the status word for the two transfer formats mentioned.
    // Modify the subtracted word count (sum of command and status words)
    // below to compensate for the trailing status word. Ex: An RT to RT
    // message with a total payload of 70 bytes does not actually truncate
    // the data payload in the case of a 32-word expected payload,
    // 70/2 - 3 = 32 words. The fact that a RT to RT message with 32 payload
    // words is not 72 bytes (36 words = 2 comm + 2 status + 32 data) does
    // not matter because the data shorage, in this case one word, the final
    // status word, always occurs at the end of the payload.

    // The subtracted word count mods are:
    // * RT to RT: 4 --> 3
    // * BC to RT: 2 --> 1
    if (data_header->RR)
    {
        calc_payload_word_count_ = (data_header->length / 2) - 3;

        // Skip recieve, transmit and status words
        payload_ptr_ += 3;
    }
    else if (data_header->tx1)
    {
        calc_payload_word_count_ = (data_header->length / 2) - 2;

        // Skip transmit and status words
        payload_ptr_ += 2;
    }
    else
    {
        calc_payload_word_count_ = (data_header->length / 2) - 1;

        // Skip receive command word
        payload_ptr_ += 1;
    }

    if (calc_payload_word_count_ < expected_payload_word_count_)
        is_payload_incomplete_ = 1;
    else
    {
        calc_payload_word_count_ = expected_payload_word_count_;
        is_payload_incomplete_ = 0;
    }

    return Ch10Status::OK;
}

uint16_t Ch101553F1Component::GetWordCountFromDataHeader(
    const MilStd1553F1DataHeaderCommWordFmt* data_header)
{
    // If RT to RT message type, don't check for mode code.
    if (data_header->RR == 0)
    {
        // Check for mode code.
        if (data_header->sub_addr1 == 0 || data_header->sub_addr1 == 31)
        {
            // If mode code, a single data payload word is present if
            // the mode code, stored in the word count field, is > 15.
            if (data_header->word_count1 > 15)
                return 1;
            else
                return 0;
        }
    }

    if (data_header->word_count1 == 0)
        return 32;
    return data_header->word_count1;
}
