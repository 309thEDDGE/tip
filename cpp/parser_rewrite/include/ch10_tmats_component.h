#ifndef CH10_TMATS_COMPONENT_H_
#define CH10_TMATS_COMPONENT_H_

#include <string>
#include <vector>
#include "ch10_packet_component.h"

class Ch10TMATSCSDWFmt
{
   public:
    uint32_t : 22;
    uint32_t frmt : 1;
    uint32_t srcc : 1;
    uint32_t ch10ver : 8;
};

/*
This class defines the structures/classes and methods
to parse Ch10 "Computer Generated Data, Format 1" or
TMATS data. 

The focus is on parsing and manipulating the data 
in the ch10 TMATS packet and not on maintaining state
or context relative to the ch10 as a whole.
*/

class Ch10TMATSComponent : public Ch10PacketComponent
{
   private:
    //
    // PacketElements to be parsed that reside in the
    // TMATS packet body.
    //
    Ch10PacketElement<Ch10TMATSCSDWFmt> tmats_csdw_elem_;

    // Vector of pointers to Ch10PacketElement, in this case
    // only tmats_csdw_elem_ to take adavantage of
    // Ch10PacketComponent::ParseElements.
    ElemPtrVec tmats_elems_vec_;

   public:
    const Ch10PacketElement<Ch10TMATSCSDWFmt>& tmats_csdw_elem;
    Ch10TMATSComponent(Ch10Context* const ch10ctx) : Ch10PacketComponent(ch10ctx),
                                                     tmats_elems_vec_{dynamic_cast<Ch10PacketElementBase*>(&tmats_csdw_elem_)},
                                                     tmats_csdw_elem(tmats_csdw_elem_) {}
    Ch10Status Parse(const uint8_t*& data,
                     std::vector<std::string>& tmats_vec);
};

#endif