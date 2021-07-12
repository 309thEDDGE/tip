

#ifndef CH10_PACKET_HEADER_COMPONENT_H_
#define CH10_PACKET_HEADER_COMPONENT_H_

#include "ch10_header_format.h"
#include "ch10_packet_component.h"
#include "ch10_time.h"

class Ch10PacketSecondaryHeaderChecksum
{
   public:
    uint16_t : 16;
    uint16_t checksum;
};

/*
Handle all parsing related to the structures or classes of bits
described class that is not related or required information for
the ch10 context. All ch10-context related data will be handled
by the  Ch10Packet class.
*/

class Ch10PacketHeaderComponent : public Ch10PacketComponent
{
   private:
    // Packet elements, i.e., bit interpretations, to be parsed out of a
    // Ch10 packet header. Not all elements will be utilized for ever packet
    // header. Most ch10 packets have headers which only have the standard
    // bit representation, which is described by Ch10PacketHeaderFmt and
    // parsed by std_hdr_elem_. The other elements are optional and related
    // the possible presence of a packet secondary header, which is indicated
    // via the secondary_hdr flag in Ch10PacketHeaderFmt.
    Ch10PacketElement<Ch10PacketHeaderFmt> std_hdr_elem_;
    Ch10PacketElement<Ch10PacketSecondaryHeaderChecksum> secondary_checksum_elem_;

    // Vectors of pointers to Ch10PacketElementBase. Used to hold pointers
    // to objects derived from the base class such that multiple elements
    // can be parsed with a single call to Ch10PacketComponent::ParseElements().
    ElemPtrVec std_elems_vec_;
    ElemPtrVec secondary_checksum_elems_vec_;

    Ch10Time ch10_time_;

    //
    // Vars for checksum calculation.
    //

    // Count of checksum units, depending on the checksum data type.
    uint32_t checksum_unit_count_;

    // Pointers and vars for summing the values in the checksum.
    const uint16_t* checksum_data_ptr16_;
    uint16_t checksum_value16_;
    const uint32_t* checksum_data_ptr32_;
    uint32_t checksum_value32_;
    const uint8_t* checksum_data_ptr8_;
    uint8_t checksum_value8_;

   public:
    // Sync value in Ch10 header as defined by Ch10 spec.
    const uint16_t sync_;

    // Standard header count of bytes used in checksum.
    const uint8_t header_checksum_byte_count_;

    // Publically available parsed data.
    const Ch10PacketElement<Ch10PacketHeaderFmt>& std_hdr_elem;
    const Ch10PacketElement<Ch10PacketSecondaryHeaderChecksum>& secondary_checksum_elem;

    const uint64_t std_hdr_size_;
    const uint64_t secondary_hdr_size_;
    Ch10PacketHeaderComponent(Ch10Context* const ch10ctx) : Ch10PacketComponent(ch10ctx),
                                                            std_hdr_elem_(),
                                                            secondary_checksum_elem_(),
                                                            ch10_time_(),
                                                            std_hdr_elem(std_hdr_elem_),
                                                            secondary_checksum_elem(secondary_checksum_elem_),
                                                            std_elems_vec_{dynamic_cast<Ch10PacketElementBase*>(&std_hdr_elem_)},
                                                            secondary_checksum_elems_vec_{dynamic_cast<Ch10PacketElementBase*>(&secondary_checksum_elem_)},
                                                            sync_(0xEB25),
                                                            std_hdr_size_(std_hdr_elem_.size),
                                                            secondary_hdr_size_(12),
                                                            header_checksum_byte_count_(std_hdr_elem_.size - 2),
                                                            checksum_unit_count_(0),
                                                            checksum_data_ptr16_(nullptr),
                                                            checksum_value16_(0),
                                                            checksum_data_ptr32_(nullptr),
                                                            checksum_value32_(0),
                                                            checksum_data_ptr8_(nullptr),
                                                            checksum_value8_(0) {}
    Ch10Status Parse(const uint8_t*& data) override;

    /*
    Parse a position in memory with the assumption that the binary data
    pointed by the data variable is secondary header matter. Calculate the
    secondary header and return the time in nanoseconds via the time_ns
    variable. Set the output time in variable time_ns to zero if there
    is an error or the secondary header is not present.

    Args:
        data    --> pointer to a position in the binary data which points
                    appropriately formatted secondary header block
        time_ns --> Output time stamp in nanosecond units

    Return:
        Ch10Status indicator
    */
    Ch10Status ParseSecondaryHeader(const uint8_t*& data, uint64_t& time_ns);

    /*
    Verify the secondary header checksum.

    Args:
        pkt_data        --> Pointer to the beginning of the secondary header
        checksum_value  --> Value of the secondary header checksum as obtained
                            from the ch10 data
    Return:
        Either Ch10Status::CHECKSUM_TRUE or Ch10Status::CHECKSUM_FALSE
    */
    Ch10Status VerifySecondaryHeaderChecksum(const uint8_t* pkt_data,
                                             const uint16_t& checksum_value);

    Ch10Status VerifyHeaderChecksum(const uint8_t* pkt_data, const uint32_t& checksum_value);
    Ch10Status VerifyDataChecksum(const uint8_t* body_data, const uint32_t& checksum_existence,
                                  const uint32_t& pkt_size, const uint32_t& secondary_hdr);

    /*
    Parse a block of raw binary data beginning at the 'data' pointer
    according to the Ch10PacketHeaderFmt::intrapkt_ts_source and 
    Ch10PacketHeaderFmt::time_format. 

    Args:
        data	--> pointer to a position in the binary data which is currently
					expected to contain IPTS data
        time_ns --> 
    */
    void ParseTimeStampNS(const uint8_t*& data, uint64_t& time_ns);
};

#endif