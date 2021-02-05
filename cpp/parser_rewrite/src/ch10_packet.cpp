#include "ch10_packet.h"

bool Ch10Packet::Parse(const uint8_t* data, const uint64_t& abs_pos)
{
    // Reset relative pos
    relative_pos_ = 0;

    // parse header
    bool ret = header_.Parse(data, relative_pos_);

    if (parse_header_only_)
        return true;

    // parse body and footer

    // Advance binbuff index to end of entire packet.
    // bb.advance(relative_pos_);

    return true;
}