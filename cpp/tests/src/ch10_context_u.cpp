#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ch10_context.h"

TEST(Ch10ContextTest, ContinueWithPacketTypeTDPSearchMustReturnTrue)
{
	Ch10Context ctx(0);
	ctx.SetSearchingForTDP(true);

	ASSERT_TRUE(ctx.ContinueWithPacketType(
		static_cast<uint8_t>(Ch10PacketType::COMPUTER_GENERATED_DATA_F1)));
	ASSERT_TRUE(ctx.ContinueWithPacketType(
		static_cast<uint8_t>(Ch10PacketType::TIME_DATA_F1)));

}

TEST(Ch10ContextTest, ContinueWithPacketTypeTDPSearchMustReturnFalse)
{
	Ch10Context ctx(0);
	ctx.SetSearchingForTDP(true);

	ASSERT_FALSE(ctx.ContinueWithPacketType(
		static_cast<uint8_t>(Ch10PacketType::MILSTD1553_F1)));
	ASSERT_FALSE(ctx.ContinueWithPacketType(
		static_cast<uint8_t>(Ch10PacketType::VIDEO_DATA_F0)));
	// . . . and all other types other than tmats and tdp1, once defined.

}

TEST(Ch10ContextTest, ContinueWithPacketTypeNoTDPSearchMustReturnFalse)
{
	Ch10Context ctx(0);
	ctx.SetSearchingForTDP(false);

	ASSERT_FALSE(ctx.ContinueWithPacketType(
		static_cast<uint8_t>(Ch10PacketType::TIME_DATA_F1)));
}

TEST(Ch10ContextTest, UpdateAbsolutePosition)
{
	Ch10Context ctx(0);
	uint64_t update_val = 4131999919;
	ctx.UpdateAbsolutePosition(update_val);
	ASSERT_EQ(ctx.absolute_position, update_val);
}

