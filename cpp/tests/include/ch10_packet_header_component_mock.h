#ifndef CH10_PACKET_HEADER_COMPONENT_MOCK_H_
#define CH10_PACKET_HEADER_COMPONENT_MOCK_H_

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ch10_packet_header_component.h"

class MockCh10PacketHeaderComponent: public Ch10PacketHeaderComponent
{
   public:
    MockCh10PacketHeaderComponent(Ch10Context* const ch10ctx) : 
        Ch10PacketHeaderComponent(ch10ctx) {}
    MOCK_METHOD1(Parse, Ch10Status(const uint8_t*& data));
    MOCK_METHOD4(VerifyDataChecksum, Ch10Status(const uint8_t* body_data, 
        const uint32_t& checksum_existence, const uint32_t& pkt_size, const uint32_t& secondary_hdr));
    MOCK_METHOD2(ParseSecondaryHeader, Ch10Status(const uint8_t*& data, uint64_t& time_ns));
    MOCK_CONST_METHOD0(GetHeader, const Ch10PacketHeaderFmt* const());
};

#endif  // CH10_PACKET_HEADER_COMPONENT_MOCK_H_