
#ifndef CH10_PACKET_H_
#define CH10_PACKET_H_

#include "binbuff.h"
#include "ch10_status.h"
#include "ch10_context.h"
#include "ch10_packet_header_component.h"
#include "ch10_tmats_component.h"
#include "ch10_tdp_component.h"
#include "ch10_1553f1_component.h"
#include "ch10_videof0_component.h"

// body
// footer

class Ch10Packet
{
private:

    const uint8_t* data_ptr_;

    Ch10PacketHeaderComponent header_;

    // Hold pointers to BinBuff and Ch10Context to avoid 
    // the need to pass them into the Parse function each time
    // a packet is parsed. Focus on performance.
    // Both objects will need to be updated 
    // as packets are parsed.
    BinBuff *const bb_;
    Ch10Context *const ctx_;

    // Member variable to hold the BinBuff object function responses.
    uint8_t bb_response_;

    // Hold Ch10Status, avoid creating new variable each time.
    Ch10Status status_;

    // Hold pkt_size temporarily to allow passing by reference.
    uint64_t temp_pkt_size_;

    // Track packet type if a packet is being parsed.
    Ch10PacketType pkt_type_;

    // Various packet parser instances.
    Ch10TMATSComponent tmats_;
    std::vector<std::string>& tmats_vec_;
    Ch10TDPComponent tdp_component_;
    Ch101553F1Component milstd1553f1_component_;
    Ch10VideoF0Component videof0_component_;

public:
    const Ch10PacketType& current_pkt_type;
    Ch10Packet(BinBuff* binbuff, Ch10Context* context, std::vector<std::string>& tmats_vec) 
        : tmats_vec_(tmats_vec),
        bb_(binbuff), ctx_(context), data_ptr_(nullptr), bb_response_(0), 
        status_(Ch10Status::OK), temp_pkt_size_(0),
        pkt_type_(Ch10PacketType::NONE), current_pkt_type(pkt_type_), header_(context), 
        tmats_(context), tdp_component_(context), milstd1553f1_component_(context),
        videof0_component_(context) {}

    /*
    Parse the ch10 header at the current location of the buffer.

    Advances the buffer to the next potentially viable position.
    */
    Ch10Status ParseHeader();

    /*
    Maintain the logic for Ch10PacketHeaderComponent::Parse status. Header
    parsing outcome has great impact on the next parsing steps. Encapsulate
    the logic for the various status possibilities in this function.

    Args:

        status --> Ch10Status returned from Ch10PacketHeaderComponent::Parse()

        pkt_size --> Size of ch10 packet as given by pkt_size field in the 
            Ch10PacketHeaderFmt.

    Return:

        Ch10Status --> Two classes of returns, good and bad. Good is indicated
            by Ch10Status::OK, and bad is any other status. A good indication
            implies that parsing of the header should continue and bad indicates
            that parsing of the header should stop and the status returned
            to the calling function.
    */
    Ch10Status ManageHeaderParseStatus(const Ch10Status& status, const uint64_t& pkt_size);


    /*
    Maintain the logic for Ch10PacketHeaderComponent::ParseSecondaryHeader 
    status. Header
    parsing outcome has great impact on the next parsing steps. Encapsulate
    the logic for the various status possibilities in this function.

    Args:

        status --> Ch10Status returned from Ch10PacketHeaderComponent::Parse()

        pkt_size --> Size of ch10 packet as given by pkt_size field in the
            Ch10PacketHeaderFmt.

    Return:

        Ch10Status --> Two classes of returns, good and bad. Good is indicated
            by Ch10Status::OK, and bad is any other status. A good indication
            implies that parsing of the header should continue and bad indicates
            that parsing of the header should stop and the status returned
            to the calling function.
    */
    Ch10Status ManageSecondaryHeaderParseStatus(const Ch10Status& status, 
        const uint64_t& pkt_size);

    /*
    Move position of buffer and absolute position index by amount indicated.
    Check for error response from BinBuff object.

    Args:

        byte_count --> Count of bytes to advance buffer and absolute position.
                       Input data type is uint32_t because the most common use 
                       of this function will be to advance by the packet size
                       to move from the beginning of the current packet to the 
                       beginning of the next packet. This value is parsed out
                       the ch10 packet header as a type uint32_t. This will
                       avoid casting during the vast majority of calls.
    */
    Ch10Status AdvanceBuffer(const uint64_t& byte_count);

    /*
    The primary purpose of this function is to call the correct parser
    for the packet type indicated in the header.
    */
    void ParseBody();

    /*
    Retrieve the earliest timestamp frpm this packet
    */
    uint64_t GetMinVideoTime();
};

#endif