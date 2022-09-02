
#ifndef CH10_TDP_COMPONENT_H_
#define CH10_TDP_COMPONENT_H_

#include <ctime>
#include "ch10_tdpf1_hdr_format.h"
#include "ch10_packet_component.h"

/*
This class defines the structures/classes and methods
to parse Ch10 "Time Data, Format 1" packets or
Time Data Packets (TDP).
*/

class Ch10TDPComponent : public Ch10PacketComponent
{
   private:
    //
    // PacketElements to be parsed that reside in the
    // "Time Data, Format 1" packet body.
    //
    Ch10PacketElement<TDF1CSDWFmt> tdp_csdw_elem_;
    Ch10PacketElement<TDF1DataIRIGFmt> tdp_irig_elem_;
    Ch10PacketElement<TDF1DataNonIRIGFmt> tdp_nonirig_elem_;

    // Vector of pointers to Ch10PacketElement to take adavantage of
    // Ch10PacketComponent::ParseElements.
    ElemPtrVec tdp_csdw_elem_vec_;
    ElemPtrVec tdp_irig_elem_vec_;
    ElemPtrVec tdp_nonirig_elem_vec_;

   public:
    const Ch10PacketElement<TDF1CSDWFmt>& tdp_csdw_elem;
    const Ch10PacketElement<TDF1DataIRIGFmt>& tdp_irig_elem;
    const Ch10PacketElement<TDF1DataNonIRIGFmt>& tdp_nonirig_elem;
    Ch10TDPComponent(Ch10Context* const ch10ctx) : Ch10PacketComponent(ch10ctx),
                                                   tdp_csdw_elem_vec_{dynamic_cast<Ch10PacketElementBase*>(&tdp_csdw_elem_)},
                                                   tdp_irig_elem_vec_{dynamic_cast<Ch10PacketElementBase*>(&tdp_irig_elem_)},
                                                   tdp_nonirig_elem_vec_{dynamic_cast<Ch10PacketElementBase*>(&tdp_nonirig_elem_)},
                                                   tdp_csdw_elem(tdp_csdw_elem_),
                                                   tdp_irig_elem(tdp_irig_elem_),
                                                   tdp_nonirig_elem(tdp_nonirig_elem_) {}
    virtual Ch10Status Parse(const uint8_t*& data);

    /*
	Compute absolute time with the assumption that the year is 1970 using 
	an instance of TDF1DataIRIGFmt as input.
	
	Args:
		irig_fmt	--> TDF1DataIRIGFmt instance with initialized values

	Return:
		Absolute time in nanoseconds since the epoch, which begins on 1/1/1970.
	*/
    uint64_t ComputeIRIGTime(const TDF1DataIRIGFmt* const irig_fmt);

    /*
	Compute absolute time with using
	an instance of TDF1DataNonIRIGFmt as input.

	Args:
		nonirig_fmt	--> TDF1DataNonIRIGFmt instance with initialized values

	Return:
		Absolute time in nanoseconds since the epoch, which begins on 1/1/1970.
	*/
    uint64_t ComputeNonIRIGTime(const TDF1DataNonIRIGFmt* const nonirig_fmt);
};

#endif