#include "ch10_packet.h"

bool Ch10Packet::Parse()
{
    // Reset relative pos for the current ch10 packet.
    relative_pos_ = 0;

    // Set the data_ptr_ at the current position within the buffer.
    // This pointer will be incremented by Parse(...).
    data_ptr_ = bb_.Data();

    // parse header
    bool ret = header_.Parse(data_ptr_, relative_pos_);

    // Continue to parse depending on the context.
    status_ = ctx_.ContinueWithPacketType((*header_.std_hdr_elem_.element)->data_type);
    if (status_ == Ch10Status::PKT_TYPE_NO)
        return true;
    else if (status_ == Ch10Status::PKT_TYPE_EXIT)
        return false;

    // IF checksums ok then
    // parse body and footer

    // Advance binbuff index to end of entire packet.
    // bb.advance(relative_pos_);

    // Since I'm skipping body/footer parsing for now,
    // move the buffer and absolute poisition by the packet
    // size.
    bb_response_ = bb_.AdvanceReadPos((*header_.std_hdr_elem_.element)->pkt_size);
    if (bb_response_ != 0)
        return false;
    ctx_.UpdateAbsolutePosition(ctx_.absolute_position + (*header_.std_hdr_elem_.element)->pkt_size);

    return true;
}

Ch10Status Ch10Packet::ParseHeader()
{
    // Reset relative pos for the current ch10 packet.
    relative_pos_ = 0;

    // Check if there are sufficient bytes in the buffer to
    // interpret the entire header, assuming initially that 
    // that there is no secondary header.
    if (!bb_.BytesAvailable(header_.std_hdr_size_))
        return Ch10Status::BUFFER_LIMITED;

    // Set the data_ptr_ at the current position within the buffer.
    // This pointer will be incremented by Parse(...).
    data_ptr_ = bb_.Data();

    // parse header
    status_ = header_.Parse(data_ptr_, relative_pos_);
    if (status_ != Ch10Status::OK)
    {
        // if bad sync move a byte
        return status_;
    }

    // Check if all of the bytes in current packet are available
    // in the buffer.
    if (!bb_.BytesAvailable((*header_.std_hdr_elem_.element)->pkt_size))
        return Ch10Status::BUFFER_LIMITED;

    // Handle parsing of the secondary header. This is currently not 
    // implemented.
    status_ = header_.ParseSecondaryHeader(data_ptr_, relative_pos_);
    if (status_ != Ch10Status::OK)
    {
        // depending on return move the buffer index to end of packet
        return status_;
    }

    // Since I'm skipping body/footer parsing for now,
    // move the buffer and absolute poisition by the packet
    // size.
    // Move this to a function to be available to the various points
    // of return above.
    bb_response_ = bb_.AdvanceReadPos((*header_.std_hdr_elem_.element)->pkt_size);
    if (bb_response_ != 0)
        return Ch10Status::BUFFER_LIMITED;
    ctx_.UpdateAbsolutePosition(ctx_.absolute_position + (*header_.std_hdr_elem_.element)->pkt_size);

    // Continue to parse depending on the context.
    return ctx_.ContinueWithPacketType((*header_.std_hdr_elem_.element)->data_type);
}