
#ifndef CH10_PACKET_H_
#define CH10_PACKET_H_

#include "binbuff.h"
#include "ch10_context.h"
#include "ch10_packet_header_component.h"

// body
// footer

class Ch10Packet
{
private:
    uint64_t relative_pos_;
    Ch10PacketHeaderComponent header_;
    const uint8_t* data_ptr_;

    // Figure out how to keep a semi-permanent pointer to 
    // a previous packet's data, for keeping the RTC from
    // the TDP and the TDP parsed data.

    // Also, how to pass in or configure different parquet writer
    // classes. 

    // Hold references to BinBuff and Ch10Context to avoid 
    // the need to pass them into the Parse function each time
    // a packet is parsed. Focus on performance.
    // Non-const because both objects will need to be updated 
    // as packets are parsed.
    BinBuff& bb_;
    Ch10Context& ctx_;

    // Member variable to hold the BinBuff object function responses.
    uint8_t bb_response_;

    public:
    Ch10Packet(BinBuff& binbuff, Ch10Context& context) : relative_pos_(0), header_(),
        bb_(binbuff), ctx_(context), data_ptr_(nullptr), bb_response_(0) {}
    bool Parse();

};

#endif