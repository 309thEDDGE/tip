
#ifndef CH10_PACKET_COMPONENT_H_
#define CH10_PACKET_COMPONENT_H_

#include <vector>
#include <memory>
#include "managed_path.h"
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

    // Set to true if path has been set. Useful for testing.
    bool out_path_is_set_;

protected:
    Ch10Status status_;

    // Pointer to Ch10Context
    Ch10Context* const ctx_;

    // Copy the output path object for use in logs, etc.
    ManagedPath out_path_;

    

public:
    const bool& out_path_is_set;
    Ch10PacketComponent(Ch10Context* const ch10ctx) : status_(Ch10Status::NONE),
        ctx_(ch10ctx), out_path_is_set(out_path_is_set_) {}
    void ParseElements(const ElemPtrVec& elems, const uint8_t*& data, 
        uint64_t& loc);
    virtual Ch10Status Parse(const uint8_t*& data, uint64_t& loc);
    virtual void SetOutputPath(const ManagedPath& mpath);
    virtual ~Ch10PacketComponent();
    
};

#endif

