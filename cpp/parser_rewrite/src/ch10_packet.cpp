#include "ch10_packet.h"

//bool Ch10Packet::Parse()
//{
//    // Reset relative pos for the current ch10 packet.
//    relative_pos_ = 0;
//
//    // Set the data_ptr_ at the current position within the buffer.
//    // This pointer will be incremented by Parse(...).
//    data_ptr_ = bb_.Data();
//
//    // parse header
//    bool ret = header_.Parse(data_ptr_, relative_pos_);
//
//    // Continue to parse depending on the context.
//    status_ = ctx_.ContinueWithPacketType((*header_.std_hdr_elem_.element)->data_type);
//    if (status_ == Ch10Status::PKT_TYPE_NO)
//        return true;
//    else if (status_ == Ch10Status::PKT_TYPE_EXIT)
//        return false;
//
//    // IF checksums ok then
//    // parse body and footer
//
//    // Advance binbuff index to end of entire packet.
//    // bb.advance(relative_pos_);
//
//    // Since I'm skipping body/footer parsing for now,
//    // move the buffer and absolute poisition by the packet
//    // size.
//    bb_response_ = bb_.AdvanceReadPos((*header_.std_hdr_elem_.element)->pkt_size);
//    if (bb_response_ != 0)
//        return false;
//    ctx_.UpdateAbsolutePosition(ctx_.absolute_position + (*header_.std_hdr_elem_.element)->pkt_size);
//
//    return true;
//}

Ch10Status Ch10Packet::AdvanceBufferAndAbsPosition(uint64_t byte_count)
{
    bb_response_ = bb_->AdvanceReadPos(byte_count);

    // If the buffer can't be advanced by the requested byte count, then 
    // the current packet can't be parsed in it's entirety. In this case,
    // do not update the absolute position because parsing will need to 
    // resume at the current absolute position later, in append mode.
    if (bb_response_ != 0)
        return Ch10Status::BUFFER_LIMITED;

    ctx_->UpdateAbsolutePosition(ctx_->absolute_position + byte_count);

    return Ch10Status::OK;
}

Ch10Status Ch10Packet::ParseHeader()
{
    // Reset relative pos for the current ch10 packet.
    relative_pos_ = 0;

    // Check if there are sufficient bytes in the buffer to
    // interpret the entire header, assuming initially that 
    // that there is no secondary header.
    if (!bb_->BytesAvailable(header_.std_hdr_size_))
        return Ch10Status::BUFFER_LIMITED;

    // Set the data_ptr_ at the current position within the buffer.
    // This pointer will be incremented by Parse(...).
    data_ptr_ = bb_->Data();

    // parse header
    status_ = header_.Parse(data_ptr_, relative_pos_);
    if (status_ != Ch10Status::OK)
    {
        // Ch10PacketHeaderComponent::Parse will return BAD_SYNC if 
        // the parsed sync word is not correct. If this occurs
        // then the data need to be searched until a correct sync 
        // word is found. Increment the buffer and absolute position
        // by a single byte and try again.
        if (status_ == Ch10Status::BAD_SYNC)
        {
            status_ = AdvanceBufferAndAbsPosition(1);

            // If the buffer can't be advanced, return this status.
            if (status_ == Ch10Status::BUFFER_LIMITED)
                return status_;

            // Otherwise indicate the bad sync status.
            return Ch10Status::BAD_SYNC;
        }

        // Handle other non-ok status after checksum comparison is implemented, etc.

        // By default, skip the parsing of the packet and move the buffer to the
        // beginning of the next packet. 
        else
        {
            status_ = AdvanceBufferAndAbsPosition((*header_.std_hdr_elem.element)->pkt_size);

            // If the buffer can't be advanced, return this status.
            if (status_ == Ch10Status::BUFFER_LIMITED)
                return status_;

            // Otherwise return a status indicating that the packet body should be 
            // skipped.
            return Ch10Status::PKT_TYPE_NO;
        }
    }

    // Check if all of the bytes in current packet are available
    // in the buffer. There is no need to continue parsing 
    if (!bb_->BytesAvailable((*header_.std_hdr_elem.element)->pkt_size))
        return Ch10Status::BUFFER_LIMITED;

    // Handle parsing of the secondary header. This is currently not 
    // completed. Data parsed in this step are not utilized.
    status_ = header_.ParseSecondaryHeader(data_ptr_, relative_pos_);
    if (status_ != Ch10Status::OK)
    {
        // Skip the entire packet if the secondary header does not parse correctly.
        // This may not be the best way to handle this situation. Amend if necessary.
        if (status_ == Ch10Status::INVALID_SECONDARY_HDR_FMT)
        {
            status_ = AdvanceBufferAndAbsPosition((*header_.std_hdr_elem.element)->pkt_size);

            // If the buffer can't be advanced, return this status.
            if (status_ == Ch10Status::BUFFER_LIMITED)
                return status_;

        }

        // Otherwise return a status indicating that the packet body should be 
            // skipped.
        return Ch10Status::PKT_TYPE_NO;
    }

    // Assuming errors have not occurred and the header and secondary header 
    // (if present) have been correctly parsed. Advance the buffer to the beginning
    // of the next packet.
    status_ = AdvanceBufferAndAbsPosition((*header_.std_hdr_elem.element)->pkt_size);
    if (status_ == Ch10Status::BUFFER_LIMITED)
        return status_;

    // Continue to parse depending on the context.
    return ctx_->ContinueWithPacketType((*header_.std_hdr_elem.element)->data_type);

}