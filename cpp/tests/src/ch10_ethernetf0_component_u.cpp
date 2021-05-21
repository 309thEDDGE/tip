#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ch10_ethernetf0_component.h"
#include "ch10_time.h"
#include "ch10_context.h"

using ::testing::_;
using ::testing::SetArgReferee;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::SetArgPointee;

class MockCh10TimeEthF0 : public Ch10Time
{
public:
    MockCh10TimeEthF0() : Ch10Time() {}
    MOCK_METHOD4(ParseIPTS, Ch10Status(const uint8_t*& data, uint64_t& time_ns,
        uint8_t intrapkt_ts_src, uint8_t time_fmt));
};

class MockCh10ContextEthF0 : public Ch10Context
{
public:
    MockCh10ContextEthF0() : Ch10Context() {}
    MOCK_METHOD1(CalculateIPTSAbsTime, uint64_t&(const uint64_t& ipts_time));
};

class Ch10EthernetF0ComponentTest : public ::testing::Test
{
protected:
    Ch10EthernetF0Component eth_;
    const uint8_t* data_ptr_;
    Ch10Status status_;
    Ch10Context ctx_;
    EthernetF0CSDW csdw_;
    EthernetF0FrameIDWord frameid_word_;
    MockCh10TimeEthF0 mock_ch10_time_;
    Ch10Time* ch10_time_ptr_;
    uint64_t ipts_time_;
    MockCh10ContextEthF0 mock_ch10_context_;
    Ch10Context* ch10_context_ptr_;

    Ch10EthernetF0ComponentTest() : data_ptr_(nullptr),
        status_(Ch10Status::NONE), ctx_(0), eth_(&ctx_), mock_ch10_time_(), 
        ch10_time_ptr_(&mock_ch10_time_), ipts_time_(0), mock_ch10_context_(),
        ch10_context_ptr_(&mock_ch10_context_)
    {
    }
};

TEST_F(Ch10EthernetF0ComponentTest, ParseExcessiveFrameCount)
{
    csdw_.frame_count = eth_.max_frame_count_ + 100; // > max_frame_count_
    data_ptr_ = (const uint8_t*)&csdw_;
    status_ = eth_.Parse(data_ptr_);
    EXPECT_EQ(status_, Ch10Status::ETHERNETF0_FRAME_COUNT);
}

TEST_F(Ch10EthernetF0ComponentTest, ParseFramesParseIPTS)
{
    csdw_.frame_count = 10;

    EXPECT_CALL(mock_ch10_time_, ParseIPTS(_, 
        _, ctx_.intrapkt_ts_src, ctx_.time_format))
        .Times(csdw_.frame_count).WillRepeatedly(Return(Ch10Status::OK));

    status_ = eth_.ParseFrames(&csdw_, &ctx_, ch10_time_ptr_, data_ptr_);
    EXPECT_EQ(status_, Ch10Status::OK);
}

TEST_F(Ch10EthernetF0ComponentTest, ParseFramesParseIPTSError)
{
    csdw_.frame_count = 10;

    EXPECT_CALL(mock_ch10_time_, ParseIPTS(_,
        _, ctx_.intrapkt_ts_src, ctx_.time_format))
        .Times(1).WillOnce(Return(Ch10Status::TIME_FORMAT_INCONCLUSIVE));

    status_ = eth_.ParseFrames(&csdw_, &ctx_, ch10_time_ptr_, data_ptr_);
    EXPECT_EQ(status_, Ch10Status::TIME_FORMAT_INCONCLUSIVE);
}

TEST_F(Ch10EthernetF0ComponentTest, ParseFramesCalcIPTSAbsTime)
{
    csdw_.frame_count = 4;
    uint64_t mock_abs_time = 1234567890;

    EXPECT_CALL(mock_ch10_time_, ParseIPTS(_,
        _, ctx_.intrapkt_ts_src, ctx_.time_format))
        .Times(csdw_.frame_count).WillRepeatedly(::testing::DoAll(
            SetArgReferee<1>(ipts_time_), Return(Ch10Status::OK)));

    EXPECT_CALL(mock_ch10_context_, CalculateIPTSAbsTime(ipts_time_)).
        Times(csdw_.frame_count).WillRepeatedly(ReturnRef(mock_abs_time));

    status_ = eth_.ParseFrames(&csdw_, ch10_context_ptr_, ch10_time_ptr_, 
        data_ptr_);
    EXPECT_EQ(status_, Ch10Status::OK);
}

TEST_F(Ch10EthernetF0ComponentTest, ParseFramesBufferAdvancement)
{
    csdw_.frame_count = 4;
    uint64_t mock_abs_time = 1234567890;

    // use SetArgePointee
    EXPECT_CALL(mock_ch10_time_, ParseIPTS(_,
        _, ctx_.intrapkt_ts_src, ctx_.time_format))
        .Times(csdw_.frame_count).WillRepeatedly(::testing::DoAll(
            SetArgReferee<1>(ipts_time_), Return(Ch10Status::OK)));

    EXPECT_CALL(mock_ch10_context_, CalculateIPTSAbsTime(ipts_time_)).
        Times(csdw_.frame_count).WillRepeatedly(ReturnRef(mock_abs_time));

    status_ = eth_.ParseFrames(&csdw_, ch10_context_ptr_, ch10_time_ptr_,
        data_ptr_);
    EXPECT_EQ(status_, Ch10Status::OK);
}

