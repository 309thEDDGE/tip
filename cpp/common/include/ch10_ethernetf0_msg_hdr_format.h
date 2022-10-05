#ifndef CH10_ETHERNETF0_MSG_HDR_FORMAT_H_
#define CH10_ETHERNETF0_MSG_HDR_FORMAT_H_

#include <cstdint>

class EthernetF0CSDW
{
   public:
    uint16_t frame_count;
    uint16_t : 9;  // reserved

    // Indicate the bit of ethernet MAC frame to which the IPH time is applicable
    uint16_t time_tag_bits : 3;
    uint16_t format : 4;  // Type of ethernet packet
};

class EthernetF0FrameIDWord
{
   public:
    uint32_t data_length : 14;  // length of frame in bytes
    uint32_t LE : 1;            // data length error: 0 = valid length; 1 = too long/short
    uint32_t DCE : 1;           // data CRC error: 0 = no error
    uint32_t net_id : 8;        // physical network identification

    // negotiated bit rate:
    // 0 = auto
    // 1 = 10 Mbps
    // 2 = 100 Mbps
    // 3 = 1 Gbps
    // 4 = 10 Gbps
    // ... reserved
    uint32_t speed : 4;

    // extent of captured mac frame:
    // 0 = full MAC frame starting with the 6-byte destination MAC address
    //     and ending with the 4-byte frame check sequence
    // 1 = Payload (data) only: starting at the 14th byte offset from the
    //     beginning of the destination MAC address and ending before the
    //     4-byte frame check sequence of the MAC frame.
    // 2, 3 reserved
    uint32_t content : 2;

    // indicate any error that occurred during transmission, 0 = no error
    uint32_t frame_error : 1;

    // the frame CRC error bit, is used to indicate that a MAC frame CRC
    // error occurred during frame transmission.
    // 0 = no error
    uint32_t FCE : 1;
};

#endif