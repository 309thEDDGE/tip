
#include "ch10_packet_header_component.h"

Ch10Status Ch10PacketHeaderComponent::Parse(const uint8_t*& data, uint64_t& loc)
{
    // Parse the standard header portion.
    ParseElements(std_elems_vec_, data, loc);

    // If the sync pattern is not correct, then this is an invalid header.
    if ((*std_hdr_elem_.element)->sync != sync_)
        return Ch10Status::BAD_SYNC;

    // Calculate and compare checksum.
    

    return Ch10Status::OK;

}

Ch10Status Ch10PacketHeaderComponent::ParseSecondaryHeader(const uint8_t*& data, uint64_t& loc)
{
    // Only parse if the secondary header option is enabled.
    if ((*std_hdr_elem_.element)->secondary_hdr == 1)
    {
        printf("Ch10PacketHeaderComponent::ParseSecondaryHeader(): Secondary header not handled!\n");
        switch ((*std_hdr_elem_.element)->time_format)
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
            printf("!!! Ch10PacketHeaderComponent::ParseSecondaryHeader(): Invalid secondary header format!\n");
        }
        }
    }
    return Ch10Status::OK;
}