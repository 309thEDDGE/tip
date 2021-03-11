#ifndef CH10_VIDEOF0_COMPONENT_H_
#define CH10_VIDEOF0_COMPONENT_H_

#include <cstdint>
#include "ch10_packet_component.h"
#include "ch10_videof0_header_format.h"

class Ch10VideoF0Component : public Ch10PacketComponent
{
private:
    Ch10PacketElement<Ch10VideoF0HeaderFormat> csdw_element_;
	ElemPtrVec csdw_element_vector_;

// Protected members are accessible to tests via inheritance
protected:
	std::vector<uint64_t> subpacket_absolute_times_;


public:
    const Ch10PacketElement<Ch10VideoF0HeaderFormat>& csdw_element;

    Ch10VideoF0Component(Ch10Context* const context) : Ch10PacketComponent(context),
        csdw_element(csdw_element_),
        csdw_element_vector_{
            dynamic_cast<Ch10PacketElementBase*>(&csdw_element_)}
        {}

    Ch10Status Parse(const uint8_t*& data) override;

    ~Ch10VideoF0Component() {}
};

#endif