
#ifndef CH10_PACKET_H_
#define CH10_PACKET_H_

#include "ch10_packet_header.h"
// body
// footer

class Ch10Packet
{
private:
    uint64_t relative_pos_;
    uint64_t absolute_pos_; 
    bool append_mode_;
    bool parse_header_only_;
    Ch10PacketHeader header_;

    // Figure out how to keep a semi-permanent pointer to 
    // a previous packet's data, for keeping the RTC from
    // the TDP and the TDP parsed data.

    // Also, how to pass in or configure different parquet writer
    // classes. 

    public:
    Ch10Packet() : relative_pos_(0), append_mode_(false), 
        parse_header_only_(false), header_() {}
    bool Parse(const uint8_t* data, const uint64_t& abs_pos);

};

#endif