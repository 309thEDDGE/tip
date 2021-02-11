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
    const uint8_t* data_ptr_;
    const uint8_t* orig_data_ptr_;
    Ch10Status status_;

    Ch10PacketTest() : loc_(0), data_ptr_(nullptr), orig_data_ptr_(nullptr),
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
	Ch10Status status = p.AdvanceBufferAndAbsPosition(advance_by);
	EXPECT_EQ(ch10_ctx_.absolute_position, loc_ + advance_by);
	EXPECT_EQ(status, Ch10Status::OK);
}

TEST_F(Ch10PacketTest, AdvanceBufferAndAbsPositionBinBuffBadAdvance)
{
	loc_ = 1456;
	ch10_ctx_.UpdateAbsolutePosition(loc_);
	uint64_t advance_by = 32100;
	EXPECT_CALL(mock_bb_, AdvanceReadPos(advance_by))
		.WillOnce(Return(1));

	Ch10Packet p(bb_ptr_, &ch10_ctx_);
	Ch10Status status = p.AdvanceBufferAndAbsPosition(advance_by);
	EXPECT_EQ(ch10_ctx_.absolute_position, loc_);
	EXPECT_EQ(status, Ch10Status::BUFFER_LIMITED);
}

// test parse header