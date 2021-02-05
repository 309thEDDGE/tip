

#ifndef CH10_PACKET_HEADER_COMPONENT_H_
#define CH10_PACKET_HEADER_COMPONENT_H_

#include "ch10_packet_component.h"

class Ch10PacketHeaderFmt
{
	public:
	uint32_t  	sync				: 16;
	uint32_t  	chanID				: 16;
	uint32_t  	pkt_size;
	uint32_t  	data_size;
	uint32_t   	data_type_ver		: 8;
	uint32_t   	seq_num				: 8;
	uint32_t	checksum_existence	: 2;
	uint32_t	time_format			: 2;
	uint32_t 	overflow_err		: 1;
	uint32_t 	sync_err			: 1;
	uint32_t 	intrapkt_ts_source	: 1;
	uint32_t 	secondary_hdr		: 1;
	uint32_t 	data_type			: 8;
	uint32_t	rtc1;
	uint32_t 	rtc2				: 16;
	uint32_t 	checksum			: 16;
	
};

class Ch10PacketSecondaryHeaderBinWtFmt
{
public:
    uint16_t            : 16;
    uint16_t microsec;
    uint16_t low_order;
    uint16_t high_order;

};

class Ch10PacketSecondaryHeaderIEEE1588Fmt
{
public:
    uint32_t ns_word;
    uint32_t sec_word;
};

class Ch10PacketSecondaryHeaderERTCFmt
{
public:
    uint32_t lslw;
    uint32_t mslw;
};

class Ch10PacketSecondaryHeaderChecksum
{
public:
    uint16_t            : 16;
    uint16_t checksum; 
};

class Ch10PacketHeaderComponent : public Ch10PacketComponent
{

private:
    
    ElemPtrVec std_elems_vec_;
    ElemPtrVec secondary_binwt_elems_vec_;
    ElemPtrVec secondary_ieee_elems_vec_;
    ElemPtrVec secondary_ertc_elems_vec_;


public:
    Ch10PacketElement<Ch10PacketHeaderFmt> std_hdr_elem_;
    Ch10PacketElement<Ch10PacketSecondaryHeaderBinWtFmt> secondary_binwt_elem_;
    Ch10PacketElement<Ch10PacketSecondaryHeaderIEEE1588Fmt> secondary_ieee_elem_;
    Ch10PacketElement<Ch10PacketSecondaryHeaderERTCFmt> secondary_ertc_elem_;
    Ch10PacketElement<Ch10PacketSecondaryHeaderChecksum> secondary_checksum_elem_;
    Ch10PacketHeaderComponent() : Ch10PacketComponent(),
        std_hdr_elem_(), secondary_binwt_elem_(), secondary_ieee_elem_(),
        secondary_ertc_elem_(), secondary_checksum_elem_(),
        std_elems_vec_{dynamic_cast<Ch10PacketElementBase*>(&std_hdr_elem_)},
        secondary_binwt_elems_vec_{dynamic_cast<Ch10PacketElementBase*>(&secondary_binwt_elem_),
                                   dynamic_cast<Ch10PacketElementBase*>(&secondary_checksum_elem_)},
        secondary_ieee_elems_vec_{dynamic_cast<Ch10PacketElementBase*>(&secondary_ieee_elem_),
                                  dynamic_cast<Ch10PacketElementBase*>(&secondary_checksum_elem_)},
        secondary_ertc_elems_vec_{dynamic_cast<Ch10PacketElementBase*>(&secondary_ertc_elem_),
                                  dynamic_cast<Ch10PacketElementBase*>(&secondary_checksum_elem_)}
        {}
    bool Parse(const uint8_t*& data, uint64_t& loc) override;

};


#endif