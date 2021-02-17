
#ifndef CH10_PACKET_COMPONENT_H_
#define CH10_PACKET_COMPONENT_H_

#include <vector>
#include <memory>
#include "ch10_status.h"
#include "ch10_context.h"
#include "ch10_packet_element.h"

/*
Base class for ch10 packet header, body and footer.
*/
using ElemPtrVec = std::vector<Ch10PacketElementBase*>;

class Ch10PacketComponent
{
private:
    ElemPtrVec::const_iterator elemit_;

protected:
    Ch10Status status_;

    // Pointer to Ch10Context
    const Ch10Context* const ctx_;

public:
    Ch10PacketComponent(const Ch10Context* const ch10ctx) : status_(Ch10Status::NONE),
        ctx_(ch10ctx) {}
    void ParseElements(const ElemPtrVec& elems, const uint8_t*& data, 
        uint64_t& loc);
    virtual Ch10Status Parse(const uint8_t*& data, uint64_t& loc);
    virtual ~Ch10PacketComponent();
    
};

#endif

