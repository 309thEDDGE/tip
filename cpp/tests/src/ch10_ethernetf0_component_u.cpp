#include <algorithm>
#include <cassert>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "ch10_ethernetf0_component.h"
#include "ch10_time.h"
#include "ch10_context.h"
#include "network_packet_parser.h"

// DO NOT REORDER THIS INCLUDE
// changing the positioning here breaks Windows build
#include "spdlog/spdlog.h"

using ::testing::_;
using ::testing::DoAll;
using ::testing::Ref;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::SetArgPointee;
using ::testing::SetArgReferee;

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

class MockNetworkPacketParser : public NetworkPacketParser
{
   public:
    MockNetworkPacketParser() : NetworkPacketParser() {}
    MOCK_METHOD4(Parse, bool(const uint8_t* buffer, const uint32_t& length,
                             EthernetData* eth_data, const uint32_t& channel_id));
};

class Ch10EthernetF0ComponentTest : public ::testing::Test
{
   protected:
    Ch10EthernetF0Component eth_;
    const uint8_t* data_ptr_;
    Ch10Status status_;
    Ch10Context ctx_;
    Ch10Time ch10_time_;
    EthernetF0CSDW csdw_;
    EthernetF0FrameIDWord frameid_word_;
    MockCh10TimeEthF0 mock_ch10_time_;
    Ch10Time* ch10_time_ptr_;
    uint64_t ipts_time_;
    MockCh10ContextEthF0 mock_ch10_context_;
    Ch10Context* ch10_context_ptr_;
    ::testing::NiceMock<MockNetworkPacketParser> mock_npp_;
    NetworkPacketParser* npp_ptr_;

    Ch10EthernetF0ComponentTest() : data_ptr_(nullptr),
                                    status_(Ch10Status::NONE),
                                    ctx_(0),
                                    eth_(&ctx_),
                                    mock_ch10_time_(),
                                    ch10_time_ptr_(&mock_ch10_time_),
                                    ipts_time_(0),
                                    mock_ch10_context_(),
                                    ch10_context_ptr_(&mock_ch10_context_),
                                    ch10_time_(),
                                    mock_npp_(),
                                    npp_ptr_(&mock_npp_)
    {
    }

    static void SetUpTestCase()
    {
        auto logger = spdlog::get("ethf0_logger");
        assert(logger);
    }
};

TEST_F(Ch10EthernetF0ComponentTest, ParseExcessiveFrameCount)
{
    csdw_.frame_count = eth_.max_frame_count_ + 100;  // > max_frame_count_
    data_ptr_ = (const uint8_t*)&csdw_;
    status_ = eth_.Parse(data_ptr_);
    EXPECT_EQ(status_, Ch10Status::ETHERNETF0_FRAME_COUNT);
}

TEST_F(Ch10EthernetF0ComponentTest, ParseFramesParseIPTS)
{
    csdw_.frame_count = 10;
    uint64_t mock_abs_time = 1234567890;
    int intrapkt_hdr_size = 8 + 4;  // IPTS + Frame ID word
    uint32_t data_length = 1284;
    std::vector<uint8_t> dummy_buffer(intrapkt_hdr_size + data_length, 0);
    data_ptr_ = dummy_buffer.data();
    EthernetF0FrameIDWord* frame_id_word = (EthernetF0FrameIDWord*)(data_ptr_ + 8);
    frame_id_word->data_length = data_length;

    // Each time, set the 0-th argument back to 8 bytes from the beginning
    // of the dummy buffer to avoid straying into memory which I don't
    // control.
    EXPECT_CALL(mock_ch10_time_, ParseIPTS(_,
                                           _, ctx_.intrapkt_ts_src, ctx_.time_format))
        .Times(csdw_.frame_count)
        .WillRepeatedly(DoAll(
            SetArgReferee<0>(dummy_buffer.data() + 8),
            SetArgReferee<1>(ipts_time_),
            Return(Ch10Status::OK)));

    ON_CALL(mock_npp_, Parse(_, _, _, _)).WillByDefault(Return(true));

    /*EXPECT_CALL(mock_ch10_context_, CalculateIPTSAbsTime(ipts_time_)).
        Times(csdw_.frame_count).WillRepeatedly(ReturnRef(mock_abs_time));*/

    status_ = eth_.ParseFrames(&csdw_, &ctx_, npp_ptr_, ch10_time_ptr_, data_ptr_);
    EXPECT_EQ(status_, Ch10Status::OK);
}

TEST_F(Ch10EthernetF0ComponentTest, ParseFramesParseIPTSError)
{
    csdw_.frame_count = 10;

    EXPECT_CALL(mock_ch10_time_, ParseIPTS(_,
                                           _, ctx_.intrapkt_ts_src, ctx_.time_format))
        .Times(1)
        .WillOnce(Return(Ch10Status::TIME_FORMAT_INCONCLUSIVE));

    status_ = eth_.ParseFrames(&csdw_, &ctx_, npp_ptr_, ch10_time_ptr_, data_ptr_);
    EXPECT_EQ(status_, Ch10Status::TIME_FORMAT_INCONCLUSIVE);
}

TEST_F(Ch10EthernetF0ComponentTest, ParseFramesCalcIPTSAbsTime)
{
    csdw_.frame_count = 4;
    uint64_t mock_abs_time = 1234567890;
    int intrapkt_hdr_size = 8 + 4;  // IPTS + Frame ID word
    std::vector<uint8_t> dummy_buffer(intrapkt_hdr_size, 0);
    data_ptr_ = dummy_buffer.data();
    EthernetF0FrameIDWord* frame_id_word = (EthernetF0FrameIDWord*)(data_ptr_ + 8);
    frame_id_word->data_length = 1233;

    EXPECT_CALL(mock_ch10_time_, ParseIPTS(_,
                                           _, ctx_.intrapkt_ts_src, ctx_.time_format))
        .Times(csdw_.frame_count)
        .WillRepeatedly(DoAll(
            SetArgReferee<0>(dummy_buffer.data() + 8),
            SetArgReferee<1>(ipts_time_),
            Return(Ch10Status::OK)));

    EXPECT_CALL(mock_ch10_context_, CalculateIPTSAbsTime(ipts_time_)).Times(csdw_.frame_count).WillRepeatedly(ReturnRef(mock_abs_time));

    ON_CALL(mock_npp_, Parse(_, _, _, _)).WillByDefault(Return(true));

    status_ = eth_.ParseFrames(&csdw_, ch10_context_ptr_, npp_ptr_, ch10_time_ptr_,
                               data_ptr_);
    EXPECT_EQ(status_, Ch10Status::OK);
}

TEST_F(Ch10EthernetF0ComponentTest, ParseFramesBufferAdvancement)
{
    csdw_.frame_count = 3;
    uint32_t channelid = 10;
    uint64_t mock_abs_time = 1234567890;
    int intrapkt_hdr_size = 8 + 4;  // IPTS + Frame ID word
    uint32_t data_length = 138;
    size_t single_frame_size = intrapkt_hdr_size + data_length;
    size_t total_buff_size = single_frame_size * csdw_.frame_count;
    std::vector<uint8_t> dummy_buffer(total_buff_size, 0);
    std::vector<uint8_t> copy_buffer(intrapkt_hdr_size, 0);
    data_ptr_ = dummy_buffer.data();
    EthernetF0FrameIDWord* frame_id_word = (EthernetF0FrameIDWord*)(copy_buffer.data() + 8);
    frame_id_word->data_length = data_length;

    // Copy the first data_length bytes into the other n sections
    // of data_length.
    for (size_t i = 0; i < csdw_.frame_count; i++)
    {
        std::copy(copy_buffer.data(), copy_buffer.data() + intrapkt_hdr_size,
                  dummy_buffer.data() + single_frame_size * i);
    }

    // Use this variable since it expects a concrete value
    // and we don't want to increment data_ptr_ since it hasn't
    // actually been passed to ParseFrames yet.
    const uint8_t* temp_data_ptr = data_ptr_;
    EXPECT_CALL(mock_ch10_time_, ParseIPTS(temp_data_ptr, _, ctx_.intrapkt_ts_src,
                                           ctx_.time_format))
        .Times(1)
        .WillOnce(DoAll(
            SetArgReferee<0>(data_ptr_ + 8),
            SetArgReferee<1>(ipts_time_),
            Return(Ch10Status::OK)));

    EXPECT_CALL(mock_npp_, Parse(temp_data_ptr + intrapkt_hdr_size, data_length, _, _)).Times(1).WillOnce(Return(true));

    temp_data_ptr += single_frame_size;
    EXPECT_CALL(mock_ch10_time_, ParseIPTS(temp_data_ptr,
                                           _, ctx_.intrapkt_ts_src,
                                           ctx_.time_format))
        .Times(1)
        .WillOnce(DoAll(
            SetArgReferee<0>(data_ptr_ + single_frame_size + 8),
            SetArgReferee<1>(ipts_time_),
            Return(Ch10Status::OK)));

    EXPECT_CALL(mock_npp_, Parse(temp_data_ptr + intrapkt_hdr_size, data_length, _, _)).Times(1).WillOnce(Return(true));

    temp_data_ptr += single_frame_size;
    EXPECT_CALL(mock_ch10_time_, ParseIPTS(temp_data_ptr, _, ctx_.intrapkt_ts_src,
                                           ctx_.time_format))
        .Times(1)
        .WillOnce(DoAll(
            SetArgReferee<0>(data_ptr_ + single_frame_size * 2 + 8),
            SetArgReferee<1>(ipts_time_),
            Return(Ch10Status::OK)));

    EXPECT_CALL(mock_npp_, Parse(temp_data_ptr + intrapkt_hdr_size, data_length, _, _)).Times(1).WillOnce(Return(true));

    EXPECT_CALL(mock_ch10_context_, CalculateIPTSAbsTime(ipts_time_)).Times(csdw_.frame_count).WillRepeatedly(ReturnRef(mock_abs_time));

    status_ = eth_.ParseFrames(&csdw_, ch10_context_ptr_, npp_ptr_, ch10_time_ptr_,
                               data_ptr_);
    EXPECT_EQ(status_, Ch10Status::OK);
    EXPECT_EQ(dummy_buffer.data() + total_buff_size,
              data_ptr_);
}

TEST_F(Ch10EthernetF0ComponentTest, ParseFramesFrameLengthExceedsMax)
{
    csdw_.frame_count = 10;
    uint64_t mock_abs_time = 1234567890;
    int intrapkt_hdr_size = 8 + 4;  // IPTS + Frame ID word
    std::vector<uint8_t> dummy_buffer(intrapkt_hdr_size, 0);
    data_ptr_ = dummy_buffer.data();
    EthernetF0FrameIDWord* frame_id_word = (EthernetF0FrameIDWord*)(data_ptr_ + 8);
    frame_id_word->data_length = 60000;  // > mac_frame_max_length_

    EXPECT_CALL(mock_ch10_time_, ParseIPTS(data_ptr_,
                                           _, ctx_.intrapkt_ts_src, ctx_.time_format))
        .Times(1)
        .WillOnce(DoAll(
            SetArgReferee<0>(dummy_buffer.data() + 8),
            SetArgReferee<1>(ipts_time_),
            Return(Ch10Status::OK)));

    EXPECT_CALL(mock_ch10_context_, CalculateIPTSAbsTime(ipts_time_)).Times(1).WillOnce(ReturnRef(mock_abs_time));

    ON_CALL(mock_npp_, Parse(_, _, _, _)).WillByDefault(Return(true));

    status_ = eth_.ParseFrames(&csdw_, ch10_context_ptr_, npp_ptr_, ch10_time_ptr_,
                               data_ptr_);
    EXPECT_EQ(status_, Ch10Status::ETHERNETF0_FRAME_LENGTH);
}

TEST_F(Ch10EthernetF0ComponentTest, ParseFramesFrameLengthZero)
{
    csdw_.frame_count = 10;
    uint64_t mock_abs_time = 1234567890;
    int intrapkt_hdr_size = 8 + 4;  // IPTS + Frame ID word
    std::vector<uint8_t> dummy_buffer(intrapkt_hdr_size, 0);
    data_ptr_ = dummy_buffer.data();
    EthernetF0FrameIDWord* frame_id_word = (EthernetF0FrameIDWord*)(data_ptr_ + 8);
    frame_id_word->data_length = 0;

    EXPECT_CALL(mock_ch10_time_, ParseIPTS(data_ptr_,
                                           _, ctx_.intrapkt_ts_src, ctx_.time_format))
        .Times(1)
        .WillOnce(DoAll(
            SetArgReferee<0>(dummy_buffer.data() + 8),
            SetArgReferee<1>(ipts_time_),
            Return(Ch10Status::OK)));

    EXPECT_CALL(mock_ch10_context_, CalculateIPTSAbsTime(ipts_time_)).Times(1).WillOnce(ReturnRef(mock_abs_time));

    ON_CALL(mock_npp_, Parse(_, _, _, _)).WillByDefault(Return(true));

    status_ = eth_.ParseFrames(&csdw_, ch10_context_ptr_, npp_ptr_, ch10_time_ptr_,
                               data_ptr_);
    EXPECT_EQ(status_, Ch10Status::ETHERNETF0_FRAME_LENGTH);
}

// Test occurrence of error with NetworkPacketParse::Parse().
// This test is only valid if a NPP parse error should immediately
// return the error status. If the frame is skipped and the loop
// over the frames in the ch10 ethernet packet is allowed to
// continue then this test doesn't have a means of knowing
// an error occurred.
//TEST_F(Ch10EthernetF0ComponentTest, ParseFramesNPPParseError)
//{
//    csdw_.frame_count = 4;
//    uint64_t mock_abs_time = 1234567890;
//    int intrapkt_hdr_size = 8 + 4; // IPTS + Frame ID word
//    int32_t data_length = 10; // too short for real eth packet
//    size_t single_frame_size = intrapkt_hdr_size + data_length;
//    std::vector<uint8_t> dummy_buffer(single_frame_size, 0);
//    data_ptr_ = dummy_buffer.data();
//    EthernetF0FrameIDWord* frame_id_word = (EthernetF0FrameIDWord*)(dummy_buffer.data() + 8);
//    frame_id_word->data_length = data_length;
//
//    EXPECT_CALL(mock_ch10_time_, ParseIPTS(_,
//        _, ctx_.intrapkt_ts_src, ctx_.time_format))
//        .Times(1).WillOnce(DoAll(
//            SetArgReferee<0>(dummy_buffer.data() + 8),
//            SetArgReferee<1>(ipts_time_),
//            Return(Ch10Status::OK)));
//
//    EXPECT_CALL(mock_ch10_context_, CalculateIPTSAbsTime(ipts_time_)).
//        Times(1).WillOnce(ReturnRef(mock_abs_time));
//
//    // Pass in real, un-mocked NetworkPacketParser object which ought
//    // to fail at parsing/interpreting the short dummy data.
//    NetworkPacketParser npp;
//    status_ = eth_.ParseFrames(&csdw_, ch10_context_ptr_, &npp, ch10_time_ptr_,
//        data_ptr_);
//    EXPECT_EQ(status_, Ch10Status::ETHERNETF0_FRAME_PARSE_ERROR);
//}
