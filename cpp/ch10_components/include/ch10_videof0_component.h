#ifndef CH10_VIDEOF0_COMPONENT_H_
#define CH10_VIDEOF0_COMPONENT_H_

#include <cstdint>
#include <vector>
#include "ch10_packet_component.h"
#include "ch10_videof0_header_format.h"
#include "ch10_time.h"

class Ch10VideoF0RTCTimeStampFmt
{
   public:
    uint64_t ts1_ : 32;
    uint64_t ts2_ : 32;
};

class Ch10VideoF0Component : public Ch10PacketComponent
{
   private:
    Ch10PacketElement<Ch10VideoF0HeaderFormat> csdw_element_;
    ElemPtrVec csdw_element_vector_;

    Ch10PacketElement<video_datum[TransportStream_DATA_COUNT]> video_payload_element_;

    Ch10Time ch10_time_;

    // Temporary holder for IPTS time
    uint64_t ipts_time_;

    // Protected members are accessible to tests via inheritance
   protected:
    std::vector<uint64_t> subpacket_absolute_times_;
    ElemPtrVec video_element_vector_;

   public:
    const Ch10PacketElement<Ch10VideoF0HeaderFormat>& csdw_element;
    const std::vector<uint64_t>& subpacket_absolute_times;

    Ch10VideoF0Component(Ch10Context* const context) : Ch10PacketComponent(context), ch10_time_(), ipts_time_(0), subpacket_absolute_times_(MAX_TransportStream_UNITS), subpacket_absolute_times(subpacket_absolute_times_), csdw_element(csdw_element_), csdw_element_vector_{dynamic_cast<Ch10PacketElementBase*>(&csdw_element_)}, video_element_vector_{dynamic_cast<Ch10PacketElementBase*>(&video_payload_element_)}
    {
    }

    ~Ch10VideoF0Component() {}

    virtual Ch10Status Parse(const uint8_t*& data);

    /*
    Parse a video subpacket. Handles video packets of both variants, 
    those with or without intra-packet headers (IPH).
        Set video_element_vector_ to point to <data>.
        Set <supbacket_absolute_times_[]> at the specified index
        to the subpacket time.

    Args:
        data - pointer to a video subpacket
        iph - true if the subpacket contains a time header
        pkt_index - the index where to store the time

    Return 
        Ch10Status::OK if no problems, otherwise a different Ch10Status code.
    */
    Ch10Status ParseSubpacket(const uint8_t*& data, bool iph, const size_t& subpacket_index);

    /*
    Return the subpacket size in bytes, including IPH if present.

    Args:
        iph - true if the subpackets contain time headers

    Returns:
        size of supackets in bytes
    */
    uint32_t GetSubpacketSize(bool iph);

    /*
    Calculate the number of times an integer <whole> can be divided exactly
    by <divisor>.  If <whole> is not divisible by <divisor>, return -1.

    Args:
        whole - the integer to be divided
        divisor - an integer to divide <whole> by

    Returns:
        The number divisions if <whole> is divisible by <divisor>
        -1 if <whole> is not divisible by <divisor>
    */
    /*(signed)*/ int32_t DivideExactInteger(uint32_t whole, uint32_t divisor);
};

#endif