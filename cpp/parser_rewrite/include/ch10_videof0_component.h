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

// Protected members are accessible to tests via inheritance
protected:
	std::vector<uint64_t> subpacket_absolute_times_;


public:
    const Ch10PacketElement<Ch10VideoF0HeaderFormat>& csdw_element;

    Ch10VideoF0Component(Ch10Context* const context) : 
        Ch10PacketComponent(context),
        csdw_element(csdw_element_),
        csdw_element_vector_{
            dynamic_cast<Ch10PacketElementBase*>(&csdw_element_)},
        time_stamp_element_vector_{
            dynamic_cast<Ch10PacketElementBase*>(&time_stamp_element_)}
        {}

    ~Ch10VideoF0Component() {}

    Ch10Status Parse(const uint8_t*& data) override;
    Ch10Status ParseSubpacket(const uint8_t*& data, bool iph);
    uint64_t ParseSubpacketTime(const uint8_t*& data, bool iph);

    /*
    Return the subpacket size in bytes, including IPH if present.

    Args:
        iph - whether the packet contains intrapacket time headers

    Returns:
        size of supackets in bytes
    */
    uint32_t GetSubpacketSize(bool iph);

    /*
    Calculate the number of subpackets given a subpcket size and the total
    size of the collection of subpackets.

    Args:
        size_of_whole - size of the data block containing N subpackets
        size_of_part - size of each subpacket within a data block
        Note: any unit can be used for size

    Returns:
        The integer number of parts the whole can be divided exactly into or
        -1 if the whole is not divisible by the part size
    */
    /*(signed)*/ int32_t DivideExactInteger(uint32_t size_of_whole, uint32_t size_of_part);
};

#endif