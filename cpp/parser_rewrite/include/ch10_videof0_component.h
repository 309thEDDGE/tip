#ifndef CH10_VIDEOF0_COMPONENT_H_
#define CH10_VIDEOF0_COMPONENT_H_

#include <cstdint>
#include "ch10_packet_component.h"
#include "ch10_videof0_format.h"

class Ch10VideoF0Component : public Ch10PacketComponent
{
private:
    Ch10PacketElement<VideoF0CSDWFmt> csdw_element;

public:
    Ch10VideoF0Component(Ch10Context* const ch10ctx) : Ch10PacketComponent(ch10ctx) {}

    Ch10Status Parse(const uint8_t*& data) override;

    ~Ch10VideoF0Component() {}
};

#endif