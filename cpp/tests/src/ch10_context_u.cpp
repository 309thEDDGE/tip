#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ch10_context.h"

TEST(Ch10ContextTest, ContinueWithPacketTypeTDPSearchYes)
{
	Ch10Context ctx(0);
	ctx.SetSearchingForTDP(true);

	ASSERT_EQ(ctx.ContinueWithPacketType(
		static_cast<uint8_t>(Ch10PacketType::COMPUTER_GENERATED_DATA_F1)),
		Ch10Status::PKT_TYPE_YES);
	ASSERT_EQ(ctx.ContinueWithPacketType(
		static_cast<uint8_t>(Ch10PacketType::TIME_DATA_F1)),
		Ch10Status::PKT_TYPE_YES);

	// Second call with time data packet must also return true.
	ASSERT_EQ(ctx.ContinueWithPacketType(
		static_cast<uint8_t>(Ch10PacketType::TIME_DATA_F1)),
		Ch10Status::PKT_TYPE_YES);
}

TEST(Ch10ContextTest, ContinueWithPacketTypeTDPSearchNo)
{
	Ch10Context ctx(0);
	ctx.SetSearchingForTDP(true);

	ASSERT_EQ(ctx.ContinueWithPacketType(
		static_cast<uint8_t>(Ch10PacketType::MILSTD1553_F1)),
		Ch10Status::PKT_TYPE_NO);
	ASSERT_EQ(ctx.ContinueWithPacketType(
		static_cast<uint8_t>(Ch10PacketType::VIDEO_DATA_F0)),
		Ch10Status::PKT_TYPE_NO);
	// . . . and all other types other than tmats and tdp1, once defined.

	// Now find the time data.
	ASSERT_EQ(ctx.ContinueWithPacketType(
		static_cast<uint8_t>(Ch10PacketType::TIME_DATA_F1)),
		Ch10Status::PKT_TYPE_YES);

	// All other packets now must return true.
	ASSERT_EQ(ctx.ContinueWithPacketType(
		static_cast<uint8_t>(Ch10PacketType::MILSTD1553_F1)),
		Ch10Status::PKT_TYPE_YES);
	ASSERT_EQ(ctx.ContinueWithPacketType(
		static_cast<uint8_t>(Ch10PacketType::VIDEO_DATA_F0)),
		Ch10Status::PKT_TYPE_YES);
	// . . . and all other types other than tmats and tdp1, once defined.
}

TEST(Ch10ContextTest, ContinueWithPacketTypeNoTDPSearch)
{
	Ch10Context ctx(0);
	ctx.SetSearchingForTDP(false);

	ASSERT_EQ(ctx.ContinueWithPacketType(
		static_cast<uint8_t>(Ch10PacketType::TIME_DATA_F1)),
		Ch10Status::PKT_TYPE_EXIT);

	// All other packets now must return true.
	ASSERT_EQ(ctx.ContinueWithPacketType(
		static_cast<uint8_t>(Ch10PacketType::MILSTD1553_F1)),
		Ch10Status::PKT_TYPE_YES);
	ASSERT_EQ(ctx.ContinueWithPacketType(
		static_cast<uint8_t>(Ch10PacketType::VIDEO_DATA_F0)),
		Ch10Status::PKT_TYPE_YES);
	// . . . and all other types other than tmats and tdp1, once defined.
}

TEST(Ch10ContextTest, UpdateAbsolutePosition)
{
	Ch10Context ctx(0);
	uint64_t update_val = 4131999919;
	ctx.UpdateAbsolutePosition(update_val);
	ASSERT_EQ(ctx.absolute_position, update_val);
}

