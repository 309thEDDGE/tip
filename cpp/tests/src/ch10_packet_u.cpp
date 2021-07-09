#include "gtest/gtest.h"
#include "gmock/gmock.h"

// Important to include the following two
// headers prior to binbuff.h, which includes
// spdlog. There is a redefinition error with
// some of the types defined in Arrow.
#include "ch10_context.h"
#include "ch10_packet.h"
#include "binbuff.h"

using ::testing::Return;

// Mock BinBuff
class MockBinBuffCh10Packet : public BinBuff
{
   public:
    MockBinBuffCh10Packet() : BinBuff() {}
    MOCK_CONST_METHOD1(BytesAvailable, bool(const uint64_t& count));
    MOCK_METHOD1(AdvanceReadPos, uint8_t(const uint64_t& count));
};

class Ch10PacketTest : public ::testing::Test
{
   protected:
    MockBinBuffCh10Packet mock_bb_;
    BinBuff* bb_ptr_;
    Ch10Context ch10_ctx_;
    Ch10Status status_;
    std::vector<std::string> tmats_vec_;

    Ch10PacketTest() : status_(Ch10Status::NONE), mock_bb_(), ch10_ctx_(0), bb_ptr_(&mock_bb_)
    {
    }

    ~Ch10PacketTest() {}
};

TEST_F(Ch10PacketTest, AdvanceBufferBinBuffGoodAdvance)
{
    uint64_t advance_by = 41132100;
    EXPECT_CALL(mock_bb_, AdvanceReadPos(advance_by))
        .WillOnce(Return(0));

    Ch10Packet p(bb_ptr_, &ch10_ctx_, tmats_vec_);
    status_ = p.AdvanceBuffer(advance_by);
    EXPECT_EQ(status_, Ch10Status::OK);
}

TEST_F(Ch10PacketTest, AdvanceBufferABinBuffBadAdvance)
{
    uint64_t advance_by = 32100;
    EXPECT_CALL(mock_bb_, AdvanceReadPos(advance_by))
        .WillOnce(Return(1));

    Ch10Packet p(bb_ptr_, &ch10_ctx_, tmats_vec_);
    status_ = p.AdvanceBuffer(advance_by);
    EXPECT_EQ(status_, Ch10Status::BUFFER_LIMITED);
}

TEST_F(Ch10PacketTest, ManageHeaderParseStatusOK)
{
    uint64_t pkt_size = 1839;

    Ch10Packet p(bb_ptr_, &ch10_ctx_, tmats_vec_);
    status_ = Ch10Status::OK;
    status_ = p.ManageHeaderParseStatus(status_, pkt_size);
    EXPECT_EQ(status_, Ch10Status::OK);
}

TEST_F(Ch10PacketTest, ManageHeaderParseStatusBadSyncNotBufferLimited)
{
    uint64_t pkt_size = 1839;

    // BinBuff::AdvanceReadPos will be called by Ch10Packet::AdvanceBufferAndAbsPosition.
    // This function will also make a call to Ch10Context.

    // The buffer will return 0 to indicate that it can advance by the
    // requested amount.
    EXPECT_CALL(mock_bb_, AdvanceReadPos(1))
        .WillOnce(Return(0));

    Ch10Packet p(bb_ptr_, &ch10_ctx_, tmats_vec_);
    status_ = Ch10Status::BAD_SYNC;
    status_ = p.ManageHeaderParseStatus(status_, pkt_size);
    EXPECT_EQ(status_, Ch10Status::BAD_SYNC);
}

TEST_F(Ch10PacketTest, ManageHeaderParseStatusBadSyncBufferLimited)
{
    uint64_t pkt_size = 1839;

    // BinBuff::AdvanceReadPos will be called by Ch10Packet::AdvanceBufferAndAbsPosition.
    // This function will also make a call to Ch10Context.

    // The buffer will return 1 to indicate that it can't advance by
    // requested amount.
    EXPECT_CALL(mock_bb_, AdvanceReadPos(1))
        .WillOnce(Return(1));

    Ch10Packet p(bb_ptr_, &ch10_ctx_, tmats_vec_);
    status_ = Ch10Status::BAD_SYNC;
    status_ = p.ManageHeaderParseStatus(status_, pkt_size);
    EXPECT_EQ(status_, Ch10Status::BUFFER_LIMITED);
}

TEST_F(Ch10PacketTest, ManageHeaderParseStatusNotOKDefaultNotBufferLimited)
{
    uint64_t pkt_size = 1839;

    // BinBuff::AdvanceReadPos will be called by Ch10Packet::AdvanceBufferAndAbsPosition.
    // This function will also make a call to Ch10Context.

    // The buffer will return 0 to indicate that it can advance by the
    // requested amount.
    EXPECT_CALL(mock_bb_, AdvanceReadPos(pkt_size))
        .WillOnce(Return(0));

    Ch10Packet p(bb_ptr_, &ch10_ctx_, tmats_vec_);
    status_ = Ch10Status::PKT_TYPE_EXIT;
    status_ = p.ManageHeaderParseStatus(status_, pkt_size);
    EXPECT_EQ(status_, Ch10Status::PKT_TYPE_NO);
}

TEST_F(Ch10PacketTest, ManageHeaderParseStatusNotOKDefaultBufferLimited)
{
    uint64_t pkt_size = 1839;

    // BinBuff::AdvanceReadPos will be called by Ch10Packet::AdvanceBufferAndAbsPosition.
    // This function will also make a call to Ch10Context.

    // The buffer will return 1 to indicate that it can't advance by
    // requested amount.
    EXPECT_CALL(mock_bb_, AdvanceReadPos(pkt_size))
        .WillOnce(Return(1));

    Ch10Packet p(bb_ptr_, &ch10_ctx_, tmats_vec_);
    status_ = Ch10Status::PKT_TYPE_EXIT;
    status_ = p.ManageHeaderParseStatus(status_, pkt_size);
    EXPECT_EQ(status_, Ch10Status::BUFFER_LIMITED);
}

TEST_F(Ch10PacketTest, ManageSecondaryHeaderParseStatusOK)
{
    uint64_t pkt_size = 1839;

    Ch10Packet p(bb_ptr_, &ch10_ctx_, tmats_vec_);
    status_ = Ch10Status::OK;
    status_ = p.ManageSecondaryHeaderParseStatus(status_, pkt_size);
    EXPECT_EQ(status_, Ch10Status::OK);
}

TEST_F(Ch10PacketTest, ManageSecondaryHeaderParseStatusInvalidNotBufferLimited)
{
    uint64_t pkt_size = 1839;

    EXPECT_CALL(mock_bb_, AdvanceReadPos(pkt_size))
        .WillOnce(Return(0));

    Ch10Packet p(bb_ptr_, &ch10_ctx_, tmats_vec_);
    status_ = Ch10Status::INVALID_SECONDARY_HDR_FMT;
    status_ = p.ManageSecondaryHeaderParseStatus(status_, pkt_size);
    EXPECT_EQ(status_, Ch10Status::PKT_TYPE_NO);
}

TEST_F(Ch10PacketTest, ManageSecondaryHeaderParseStatusInvalidBufferLimited)
{
    uint64_t pkt_size = 1839;

    EXPECT_CALL(mock_bb_, AdvanceReadPos(pkt_size))
        .WillOnce(Return(1));

    Ch10Packet p(bb_ptr_, &ch10_ctx_, tmats_vec_);
    status_ = Ch10Status::INVALID_SECONDARY_HDR_FMT;
    status_ = p.ManageSecondaryHeaderParseStatus(status_, pkt_size);
    EXPECT_EQ(status_, Ch10Status::BUFFER_LIMITED);
}

TEST_F(Ch10PacketTest, InitializeFileWriters)
{
}

// test parse header