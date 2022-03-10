#ifndef CH10_CONTEXT_MOCK_H_
#define CH10_CONTEXT_MOCK_H_

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ch10_context.h"

class MockCh10Context: public Ch10Context
{
   public:
    MockCh10Context() : Ch10Context() {}
    MOCK_METHOD3(UpdateContext, Ch10Status(const uint64_t& abs_pos, 
        const Ch10PacketHeaderFmt* const hdr_fmt_ptr_, const uint64_t& rtc_time));
    MOCK_METHOD1(UpdateWithSecondaryHeaderTime, void(const uint64_t& time_ns));
    MOCK_METHOD1(ContinueWithPacketType, Ch10Status(uint8_t data_type));
    MOCK_METHOD1(AdvanceAbsPos, void(uint64_t advance_bytes));
    MOCK_METHOD1(IsPacketTypeEnabled, bool(const Ch10PacketType& pkt_type));
    MOCK_METHOD1(RecordMinVideoTimeStamp, void(const uint64_t& ts));
    MOCK_METHOD1(RegisterUnhandledPacketType, bool(const Ch10PacketType& pkt_type));
};


#endif  // CH10_CONTEXT_MOCK_H_