#ifndef CH10_VIDEOF0_COMPONENT_H_
#define CH10_VIDEOF0_COMPONENT_H_

#include <cstdint>
#include "ch10_packet_component.h"
#include "ch10_videof0_header_format.h"

class Ch10VideoF0Component : public Ch10PacketComponent
{
private:
    Ch10PacketElement<Ch10VideoF0HeaderFormat> csdw_element_;
	ElemPtrVec csdw_elemement_vector_;
    bool contains_intrapacket_headers_;

public:
    const Ch10PacketElement<Ch10VideoF0HeaderFormat>& csdw_element;

    Ch10VideoF0Component(Ch10Context* const ch10ctx) : Ch10PacketComponent(ch10ctx),
        csdw_element(csdw_element_),
        csdw_elemement_vector_{
            dynamic_cast<Ch10PacketElementBase*>(&csdw_element_)}
        {}

    Ch10Status Parse(const uint8_t*& data) override;

    ~Ch10VideoF0Component() {}
};

#endif