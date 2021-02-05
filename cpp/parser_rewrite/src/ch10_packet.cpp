#include "ch10_packet.h"

bool Ch10Packet::Parse()
{
    // Reset relative pos
    relative_pos_ = 0;

    // Set the data_ptr_ at the current position within the buffer.
    // This pointer will be incremented by Parse(...).
    data_ptr_ = bb_.Data();

    // parse header
    bool ret = header_.Parse(data_ptr_, relative_pos_);

    // Continue to parse depending on the context.
    if (!ctx_.ContinueWithPacketType((*header_.std_hdr_elem_.element)->data_type))
        return true;

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