#include "ch10_packet_component.h"

void Ch10PacketComponent::ParseElements(const ElemPtrVec& elems,
                                        const uint8_t*& data)
{
    // Iterate over each Ch10PacketElement and Set the data.
    for (elemit_ = elems.begin(); elemit_ != elems.end(); ++elemit_)
    {
        (*elemit_)->Set(data);

        // Increment the data pointer.
        data += (*elemit_)->size;
    }
}

Ch10Status Ch10PacketComponent::Parse(const uint8_t*& data)
{
    return Ch10Status::OK;
}

Ch10PacketComponent::~Ch10PacketComponent()
{
}