#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ch10_1553f1_component.h"

class Ch101553F1ComponentTest : public ::testing::Test
{
protected:
    Ch101553F1Component comp_;
    uint64_t loc_;
    const uint8_t* data_ptr_;
    Ch10Status status_;
    Ch10Context ctx_;
    MilStd1553F1DataHeaderFmt fmt_;

    Ch101553F1ComponentTest() : loc_(0), data_ptr_(nullptr),
        status_(Ch10Status::NONE), ctx_(0), comp_(&ctx_)
    {
    }
};

TEST_F(Ch101553F1ComponentTest, ParsePayloadTooManyBytesRTtoRT)
{
    // Set message byte count to > 72, the max possible.
    fmt_.length = 80;
    fmt_.RR = 1;

    status_ = comp_.ParsePayload(data_ptr_, &fmt_);
    EXPECT_EQ(status_, Ch10Status::MILSTD1553_MSG_LENGTH);
}

TEST_F(Ch101553F1ComponentTest, ParsePayloadTooManyBytesNonRTtoRT)
{
    // Set message byte count to > 72, the max possible.
    fmt_.length = 80;
    fmt_.RR = 0;
    fmt_.tx1 = 0;

    status_ = comp_.ParsePayload(data_ptr_, &fmt_);
    EXPECT_EQ(status_, Ch10Status::MILSTD1553_MSG_LENGTH);

    // Transmit bit status should not impact this check.
    fmt_.tx1 = 1;
    status_ = comp_.ParsePayload(data_ptr_, &fmt_);
    EXPECT_EQ(status_, Ch10Status::MILSTD1553_MSG_LENGTH);
}