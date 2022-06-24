#ifndef BINBUFF_MOCK_H_
#define BINBUFF_MOCK_H_

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "binbuff.h"

class MockBinBuff: public BinBuff
{
   public:
    MockBinBuff() : BinBuff() {}
    MOCK_CONST_METHOD1(BytesAvailable, bool(const uint64_t& count));
    MOCK_METHOD1(AdvanceReadPos, uint8_t(const uint64_t& count));
    MOCK_METHOD4(Initialize, uint64_t(std::ifstream& file,
                                      const uint64_t& file_size, const uint64_t& read_pos,
                                      const uint64_t& read_count));
    MOCK_CONST_METHOD0(Data, const uint8_t*());
};

#endif  // BINBUFF_MOCK_H_