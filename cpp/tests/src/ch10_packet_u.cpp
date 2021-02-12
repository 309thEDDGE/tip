#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "binbuff.h"
#include "ch10_context.h"
#include "ch10_packet.h"

using ::testing::Return;

// Mock BinBuff
class MockBinBuff : public BinBuff
{
public:
	MockBinBuff() : BinBuff() {}
	MOCK_CONST_METHOD1(BytesAvailable, bool(const uint64_t& count));
	MOCK_METHOD1(AdvanceReadPos, uint8_t(const uint64_t& count));
};


class Ch10PacketTest : public ::testing::Test
{
protected:
    MockBinBuff mock_bb_;
	BinBuff* bb_ptr_;
    Ch10Context ch10_ctx_;
    uint64_t loc_;
    Ch10Status status_;

    Ch10PacketTest() : loc_(0),
        status_(Ch10Status::NONE), mock_bb_(), ch10_ctx_(loc_), bb_ptr_(&mock_bb_)
    {
    }
};

TEST_F(Ch10PacketTest, AdvanceBufferAndAbsPositionBinBuffGoodAdvance)
{
	loc_ = 1456;
	ch10_ctx_.UpdateAbsolutePosition(loc_);
	uint64_t advance_by = 41132100;
	EXPECT_CALL(mock_bb_, AdvanceReadPos(advance_by))
		.WillOnce(Return(0));

	Ch10Packet p(bb_ptr_, &ch10_ctx_);
	status_ = p.AdvanceBufferAndAbsPosition(advance_by);
	EXPECT_EQ(ch10_ctx_.absolute_position, loc_ + advance_by);
	EXPECT_EQ(status_, Ch10Status::OK);
}

TEST_F(Ch10PacketTest, AdvanceBufferAndAbsPositionBinBuffBadAdvance)
{
	loc_ = 1456;
	ch10_ctx_.UpdateAbsolutePosition(loc_);
	uint64_t advance_by = 32100;
	EXPECT_CALL(mock_bb_, AdvanceReadPos(advance_by))
		.WillOnce(Return(1));

	Ch10Packet p(bb_ptr_, &ch10_ctx_);
	status_ = p.AdvanceBufferAndAbsPosition(advance_by);
	EXPECT_EQ(ch10_ctx_.absolute_position, loc_);
	EXPECT_EQ(status_, Ch10Status::BUFFER_LIMITED);
}

TEST_F(Ch10PacketTest, ManageHeaderParseStatusOK)
{
	loc_ = 1456;
	ch10_ctx_.UpdateAbsolutePosition(loc_);
	uint64_t pkt_size = 1839;

	Ch10Packet p(bb_ptr_, &ch10_ctx_);
	status_ = Ch10Status::OK;
	status_ = p.ManageHeaderParseStatus(status_, pkt_size);
	EXPECT_EQ(status_, Ch10Status::OK);
}

TEST_F(Ch10PacketTest, ManageHeaderParseStatusBadSyncNotBufferLimited)
{
	loc_ = 1456;
	ch10_ctx_.UpdateAbsolutePosition(loc_);
	uint64_t pkt_size = 1839;

	// BinBuff::AdvanceReadPos will be called by Ch10Packet::AdvanceBufferAndAbsPosition.
	// This function will also make a call to Ch10Context.

	// The buffer will return 0 to indicate that it can advance by the 
	// requested amount.
	EXPECT_CALL(mock_bb_, AdvanceReadPos(1))
		.WillOnce(Return(0));

	Ch10Packet p(bb_ptr_, &ch10_ctx_);
	status_ = Ch10Status::BAD_SYNC;
	status_ = p.ManageHeaderParseStatus(status_, pkt_size);
	EXPECT_EQ(status_, Ch10Status::BAD_SYNC);

	// Check to make sure the Ch10Context instance absolute position
	// was updated correctly. Note that AdvanceBufferAndAbsPosition is
	// already tested.
	EXPECT_EQ(ch10_ctx_.absolute_position, loc_ + 1);
}

TEST_F(Ch10PacketTest, ManageHeaderParseStatusBadSyncBufferLimited)
{
	loc_ = 1456;
	ch10_ctx_.UpdateAbsolutePosition(loc_);
	uint64_t pkt_size = 1839;

	// BinBuff::AdvanceReadPos will be called by Ch10Packet::AdvanceBufferAndAbsPosition.
	// This function will also make a call to Ch10Context.

	// The buffer will return 1 to indicate that it can't advance by
	// requested amount.
	EXPECT_CALL(mock_bb_, AdvanceReadPos(1))
		.WillOnce(Return(1));

	Ch10Packet p(bb_ptr_, &ch10_ctx_);
	status_ = Ch10Status::BAD_SYNC;
	status_ = p.ManageHeaderParseStatus(status_, pkt_size);
	EXPECT_EQ(status_, Ch10Status::BUFFER_LIMITED);
}

TEST_F(Ch10PacketTest, ManageHeaderParseStatusNotOKDefaultNotBufferLimited)
{
	loc_ = 1456;
	ch10_ctx_.UpdateAbsolutePosition(loc_);
	uint64_t pkt_size = 1839;

	// BinBuff::AdvanceReadPos will be called by Ch10Packet::AdvanceBufferAndAbsPosition.
	// This function will also make a call to Ch10Context.

	// The buffer will return 0 to indicate that it can advance by the 
	// requested amount.
	EXPECT_CALL(mock_bb_, AdvanceReadPos(pkt_size))
		.WillOnce(Return(0));

	Ch10Packet p(bb_ptr_, &ch10_ctx_);
	status_ = Ch10Status::PKT_TYPE_EXIT;
	status_ = p.ManageHeaderParseStatus(status_, pkt_size);
	EXPECT_EQ(status_, Ch10Status::PKT_TYPE_NO);
}

TEST_F(Ch10PacketTest, ManageHeaderParseStatusNotOKDefaultBufferLimited)
{
	loc_ = 1456;
	ch10_ctx_.UpdateAbsolutePosition(loc_);
	uint64_t pkt_size = 1839;

	// BinBuff::AdvanceReadPos will be called by Ch10Packet::AdvanceBufferAndAbsPosition.
	// This function will also make a call to Ch10Context.

	// The buffer will return 1 to indicate that it can't advance by
	// requested amount.
	EXPECT_CALL(mock_bb_, AdvanceReadPos(pkt_size))
		.WillOnce(Return(1));

	Ch10Packet p(bb_ptr_, &ch10_ctx_);
	status_ = Ch10Status::PKT_TYPE_EXIT;
	status_ = p.ManageHeaderParseStatus(status_, pkt_size);
	EXPECT_EQ(status_, Ch10Status::BUFFER_LIMITED);
}

TEST_F(Ch10PacketTest, ManageSecondaryHeaderParseStatusOK)
{
	loc_ = 1456;
	ch10_ctx_.UpdateAbsolutePosition(loc_);
	uint64_t pkt_size = 1839;

	Ch10Packet p(bb_ptr_, &ch10_ctx_);
	status_ = Ch10Status::OK;
	status_ = p.ManageSecondaryHeaderParseStatus(status_, pkt_size);
	EXPECT_EQ(status_, Ch10Status::OK);
}

TEST_F(Ch10PacketTest, ManageSecondaryHeaderParseStatusInvalidNotBufferLimited)
{
	loc_ = 1456;
	ch10_ctx_.UpdateAbsolutePosition(loc_);
	uint64_t pkt_size = 1839;

	EXPECT_CALL(mock_bb_, AdvanceReadPos(pkt_size))
		.WillOnce(Return(0));

	Ch10Packet p(bb_ptr_, &ch10_ctx_);
	status_ = Ch10Status::INVALID_SECONDARY_HDR_FMT;
	status_ = p.ManageSecondaryHeaderParseStatus(status_, pkt_size);
	EXPECT_EQ(status_, Ch10Status::PKT_TYPE_NO);
}

TEST_F(Ch10PacketTest, ManageSecondaryHeaderParseStatusInvalidBufferLimited)
{
	loc_ = 1456;
	ch10_ctx_.UpdateAbsolutePosition(loc_);
	uint64_t pkt_size = 1839;

	EXPECT_CALL(mock_bb_, AdvanceReadPos(pkt_size))
		.WillOnce(Return(1));

	Ch10Packet p(bb_ptr_, &ch10_ctx_);
	status_ = Ch10Status::INVALID_SECONDARY_HDR_FMT;
	status_ = p.ManageSecondaryHeaderParseStatus(status_, pkt_size);
	EXPECT_EQ(status_, Ch10Status::BUFFER_LIMITED);
}




// test parse header