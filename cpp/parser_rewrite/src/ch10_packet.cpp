#include "ch10_packet.h"


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

Ch10Status Ch10Packet::ManageHeaderParseStatus(const Ch10Status& status, 
    const uint64_t& pkt_size)
{
    if (status != Ch10Status::OK)
    {
        // Ch10PacketHeaderComponent::Parse will return BAD_SYNC if 
        // the parsed sync word is not correct. If this occurs
        // then the data need to be searched until a correct sync 
        // word is found. Increment the buffer and absolute position
        // by a single byte and try again.
        if (status == Ch10Status::BAD_SYNC)
        {
            status_ = AdvanceBufferAndAbsPosition(1);

            // If the buffer can't be advanced, return this status.
            if (status_ == Ch10Status::BUFFER_LIMITED)
                return status_;

            // Otherwise indicate the bad sync status.
            return Ch10Status::BAD_SYNC;
        }

        // TODO: Handle other non-ok status after checksum comparison is implemented, etc.

        // By default, skip the parsing of the packet and move the buffer to the
        // beginning of the next packet. 
        else
        {
            status_ = AdvanceBufferAndAbsPosition(pkt_size);

            // If the buffer can't be advanced, return this status.
            if (status_ == Ch10Status::BUFFER_LIMITED)
                return status_;

            // Otherwise return a status indicating that the packet body should be 
            // skipped.
            return Ch10Status::PKT_TYPE_NO;
        }
    }
    return Ch10Status::OK;
}

Ch10Status Ch10Packet::ManageSecondaryHeaderParseStatus(const Ch10Status& status, 
    const uint64_t& pkt_size)
{
    if (status != Ch10Status::OK)
    {
        // Skip the entire packet if the secondary header does not parse correctly.
        // This may not be the best way to handle this situation. Amend if necessary.
        //if (status == Ch10Status::INVALID_SECONDARY_HDR_FMT)
        status_ = AdvanceBufferAndAbsPosition(pkt_size);

        // If the buffer can't be advanced, return this status.
        if (status_ == Ch10Status::BUFFER_LIMITED)
            return status_;

        // Otherwise return a status indicating that the packet body should be 
        // skipped.
        return Ch10Status::PKT_TYPE_NO;
    }
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
    temp_pkt_size_ = (uint64_t)(*header_.std_hdr_elem.element)->pkt_size;
    status_ = ManageHeaderParseStatus(status_, temp_pkt_size_);
    if (status_ != Ch10Status::OK)
        return status_;

    // Check if all of the bytes in current packet are available
    // in the buffer. There is no need to continue parsing the secondary
    // header or the packet body if all of the data are not present in
    // the buffer.
    if (!bb_->BytesAvailable(temp_pkt_size_))
        return Ch10Status::BUFFER_LIMITED;

    // Whether the header (possibly secondary header) have been parsed correctly,
    // move the absolute position and buffer position to the beginning of the next
    // ch10 header, which ought to occur after pkt_size bytes. Configuring these
    // values now instead of waiting until after the body has been parsed will allow
    // skipping to next header if other problems occur, for example by 'continue'ing
    // in the main loop if a certain status is returned. The various parsers needed
    // to parse the packet body will use data_ptr_ and relative_pos_ to track position.
    status_ = AdvanceBufferAndAbsPosition(temp_pkt_size_);
    if (status_ == Ch10Status::BUFFER_LIMITED)
        return status_;

    // Check the data checksum now that it's known the buffer enough
    // bytes remaining to fully parse the packet. Do not proceed if the
    // data checksum is invalid.


    // Handle parsing of the secondary header. This is currently not 
    // completed. Data parsed in this step are not utilized.
    status_ = header_.ParseSecondaryHeader(data_ptr_, relative_pos_);
    status_ = ManageSecondaryHeaderParseStatus(status_, temp_pkt_size_);
    if (status_ != Ch10Status::OK)
        return status_;

    // Continue to parse depending on the context.
    return ctx_->ContinueWithPacketType((*header_.std_hdr_elem.element)->data_type);

}