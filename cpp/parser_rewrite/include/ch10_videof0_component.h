#ifndef CH10_VIDEOF0_COMPONENT_H_
#define CH10_VIDEOF0_COMPONENT_H_

#include <cstdint>
#include "ch10_packet_component.h"
#include "ch10_videof0_header_format.h"

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

    Ch10PacketElement<Ch10VideoF0RTCTimeStampFmt> time_stamp_element_;
    ElemPtrVec time_stamp_element_vector_;

    Ch10PacketElement<video_datum[TransportStream_DATA_COUNT]> video_payload_element_;

// Protected members are accessible to tests via inheritance
protected:
	std::vector<uint64_t> subpacket_absolute_times_;
    ElemPtrVec video_element_vector_;

public:
    const Ch10PacketElement<Ch10VideoF0HeaderFormat>& csdw_element;
    const std::vector<uint64_t>& subpacket_absolute_times;

    Ch10VideoF0Component(Ch10Context* const context) : 
        Ch10PacketComponent(context),
        subpacket_absolute_times_(MAX_TransportStream_UNITS),
        subpacket_absolute_times(subpacket_absolute_times_),
        csdw_element(csdw_element_),
        csdw_element_vector_{
            dynamic_cast<Ch10PacketElementBase*>(&csdw_element_)},
        time_stamp_element_vector_{
            dynamic_cast<Ch10PacketElementBase*>(&time_stamp_element_)},
        video_element_vector_{
            dynamic_cast<Ch10PacketElementBase*>(&video_payload_element_)}
        {}

    ~Ch10VideoF0Component() {}

    Ch10Status Parse(const uint8_t*& data) override;

    /*
    Parse the video subpacket pointed to by <data>.  
        Set video_element_vector_ to point to <data>.
        Set <supbacket_absolute_times_[]> at the specified index
        to the subpacket time.

    Args:
        data - pointer to a video subpacket
        iph - true if the subpacket contains a time header
        pkt_index - the index where to store the time
    */
    void ParseSubpacket(const uint8_t*& data, bool iph, const size_t& subpacket_index);
    
    /*
    Return the absolute time for the subpacket pointed to by <data>.
        If iph==true, read the subpacket time from the IPH and advance <data>.
        If iph is false, return the absolute time for the containing 
        packet, leaving <data> unchanged.

    Args:
        data - pointer to a video subpacket
        iph - true if the subpacket contains a time header
    */
    uint64_t ParseSubpacketTime(const uint8_t*& data, bool iph);

    /*
    Return the subpacket size in bytes, including IPH if present.

    Args:
        iph - true if the subpacket contains a time header

    Returns:
        size of supackets in bytes
    */
    uint32_t GetSubpacketSize(bool iph);

    /*
    Calculate the number of times an integer <whole> can be divided exactly
    by <divisor>.  If <whole> is not divisible by <divisor>, return -1.

    Args:
        whole - the integer to be divided
        divisor - integer to divide <whole> by

    Returns:
        -1 if <whole> is not divisible by <divisor>
        otherwise the number divisions 
    */
    /*(signed)*/ int32_t DivideExactInteger(uint32_t whole, uint32_t divisor);
};

#endif