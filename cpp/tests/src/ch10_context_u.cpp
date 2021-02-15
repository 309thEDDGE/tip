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

TEST(Ch10ContextTest, CreateDefaultPacketTypeConfigNecessaryValsPresent)
{
	Ch10Context ctx(0);

	std::unordered_map<Ch10PacketType, bool> testmap;
	ctx.CreateDefaultPacketTypeConfig(testmap);

	EXPECT_EQ(testmap.at(Ch10PacketType::COMPUTER_GENERATED_DATA_F1), true);
	EXPECT_EQ(testmap[Ch10PacketType::TIME_DATA_F1], true);
	EXPECT_EQ(testmap[Ch10PacketType::MILSTD1553_F1], true);
	EXPECT_EQ(testmap[Ch10PacketType::VIDEO_DATA_F0], true);
}

TEST(Ch10ContextTest, SetPacketTypeConfigDefaultsNonconfigurable)
{
	Ch10Context ctx(0);

	std::map<Ch10PacketType, bool> pkt_type_conf;

	// Set the two non-configurable packet types to false.
	pkt_type_conf[Ch10PacketType::COMPUTER_GENERATED_DATA_F1] = false;
	pkt_type_conf[Ch10PacketType::TIME_DATA_F1] = false;

	// Set the configuration.
	ctx.SetPacketTypeConfig(pkt_type_conf);

	// Check that the config state of the default packet types 
	// are true.
	EXPECT_TRUE(ctx.pkt_type_config_map.at(Ch10PacketType::COMPUTER_GENERATED_DATA_F1));
	EXPECT_TRUE(ctx.pkt_type_config_map.at(Ch10PacketType::TIME_DATA_F1));
}

TEST(Ch10ContextTest, SetPacketTypeConfigUnconfiguredAreTrue)
{
	Ch10Context ctx(0);

	std::map<Ch10PacketType, bool> pkt_type_conf;

	// Turn on 1553 and neglect to specify video. 
	pkt_type_conf[Ch10PacketType::MILSTD1553_F1] = true;

	// Set the configuration.
	ctx.SetPacketTypeConfig(pkt_type_conf);

	EXPECT_TRUE(ctx.pkt_type_config_map.at(Ch10PacketType::MILSTD1553_F1));

	// Video data should be turned on be default since it was not specified.
	EXPECT_TRUE(ctx.pkt_type_config_map.at(Ch10PacketType::VIDEO_DATA_F0));
}

TEST(Ch10ContextTest, SetPacketTypeConfigConfirmDisabledTypes)
{
	Ch10Context ctx(0);

	std::map<Ch10PacketType, bool> pkt_type_conf;

	// Turn on 1553 and disable video.
	pkt_type_conf[Ch10PacketType::MILSTD1553_F1] = true;
	pkt_type_conf[Ch10PacketType::VIDEO_DATA_F0] = false;

	// Set the configuration.
	ctx.SetPacketTypeConfig(pkt_type_conf);

	EXPECT_TRUE(ctx.pkt_type_config_map.at(Ch10PacketType::MILSTD1553_F1));

	// Video was explicitly turned off.
	EXPECT_FALSE(ctx.pkt_type_config_map.at(Ch10PacketType::VIDEO_DATA_F0));
}

TEST(Ch10ContextTest, UpdateContextSetVars)
{
	Ch10Context ctx(0);
	uint64_t abs_pos = 344199919;
	uint32_t pkt_size = 4320;
	uint32_t data_size = 3399;
	uint32_t rtc1 = 321053;
	uint32_t rtc2 = 502976;
	uint64_t rtc = ((uint64_t(rtc2) << 32) + uint64_t(rtc1)) * 100;

	ctx.UpdateContext(abs_pos, pkt_size, data_size, rtc1, rtc2);
	EXPECT_EQ(abs_pos, ctx.absolute_position);
	EXPECT_EQ(pkt_size, ctx.pkt_size);
	EXPECT_EQ(data_size, ctx.data_size);
	EXPECT_EQ(rtc, ctx.rtc);
}
