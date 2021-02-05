
#include "ch10_packet_header_component.h"

bool Ch10PacketHeaderComponent::Parse(const uint8_t*& data, uint64_t& loc)
{
    // Parse the standard header portion.
    ParseElements(std_elems_vec_, data, loc);

    // In actual code, pass in a BinBuff object in stead of a 
    // uint8_t* pointer to the data. The header packet component
    // will be in charge of checking if the body and footer
    // exceeds the remaining bytes left in the buffer. It
    // will do this before proceeding.
    // if((*std_hder_elem)->element.pkt_size > bb.bytes_remaining())
    //{ return false; }

    // If the secondary header bit is high, parse the
    // secondary header.
    // Write function for this:
    // ParseSecondaryHeader()

    return true;

}