
#include "ch10_packet_header_component.h"

Ch10Status Ch10PacketHeaderComponent::Parse(const uint8_t*& data, uint64_t& loc)
{
    // Parse the standard header portion.
    ParseElements(std_elems_vec_, data, loc);

    // If the sync pattern is not correct, then this is an invalid header.
    if ((*std_hdr_elem_.element)->sync != sync_)
        return Ch10Status::BAD_SYNC;

    // Calculate and compare checksum.
    // TODO


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

    return Ch10Status::OK;

}

Ch10Status Ch10PacketHeaderComponent::ParseSecondaryHeader(const uint8_t*& data, uint64_t& loc)
{
    // Only parse if the secondary header option is enabled.
    if ((*std_hdr_elem_)->secondary_hdr == 1)
    {
        printf("Ch10PacketHeaderComponent::ParseSecondaryHeader(): Secondary header not handled!\n");
        switch ((*std_hdr_elem_)->time_format)
        {
        case 0:
        {
            ParseElements(secondary_binwt_elems_vec_, data, loc);
        }
        case 1: 
        {
            ParseElements(secondary_ieee_elems_vec_, data, loc);
        }
        case 2:
        {
            ParseElements(secondary_ertc_elems_vec_, data, loc);
        }
        default:
        {
            return Ch10Status::INVALID_SECONDARY_HDR_FMT;
        }
        }
    }
    return Ch10Status::OK;
}