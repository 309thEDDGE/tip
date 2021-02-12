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

TEST(Ch10ContextTest, CreatePacketTypeConfigReferenceNecessaryValsPresent)
{
	Ch10Context ctx(0);

	std::map<Ch10PacketType, uint64_t> testmap;
	ctx.CreatePacketTypeConfigReference(testmap);
	uint64_t two = 2;
	uint64_t index = 0;

	EXPECT_EQ(testmap.at(Ch10PacketType::COMPUTER_GENERATED_DATA_F1), std::pow(two, index));
	EXPECT_EQ(testmap[Ch10PacketType::TIME_DATA_F1], std::pow(two, index+1));
	EXPECT_EQ(testmap[Ch10PacketType::MILSTD1553_F1], std::pow(two, index+2));
	EXPECT_EQ(testmap[Ch10PacketType::VIDEO_DATA_F0], std::pow(two, index+3));
}

TEST(Ch10ContextTest, SetPacketTypeConfigDefaultsNonconfigurable)
{
	Ch10Context ctx(0);

	std::map<Ch10PacketType, bool> pkt_type_config_map;

	// Set the two non-configurable packet types to false.
	pkt_type_config_map[Ch10PacketType::COMPUTER_GENERATED_DATA_F1] = false;
	pkt_type_config_map[Ch10PacketType::TIME_DATA_F1] = false;

	// Set the configuration.
	ctx.SetPacketTypeConfig(pkt_type_config_map);

	// Check that the bits representative of the default packet types 
	// are high (= true).
	uint64_t ref_value = ctx.pkt_type_config_reference_map.at(Ch10PacketType::COMPUTER_GENERATED_DATA_F1);
	EXPECT_EQ(ref_value & ctx.pkt_type_config, ref_value);
	ref_value = ctx.pkt_type_config_reference_map.at(Ch10PacketType::TIME_DATA_F1);
	EXPECT_EQ(ref_value & ctx.pkt_type_config, ref_value);
}

TEST(Ch10ContextTest, SetPacketTypeConfigUnconfiguredAreTrue)
{
	Ch10Context ctx(0);

	std::map<Ch10PacketType, bool> pkt_type_config_map;

	// Turn on 1553 and neglect to specify video. 
	pkt_type_config_map[Ch10PacketType::MILSTD1553_F1] = true;

	// Set the configuration.
	ctx.SetPacketTypeConfig(pkt_type_config_map);

	uint64_t ref_value = ctx.pkt_type_config_reference_map.at(Ch10PacketType::MILSTD1553_F1);
	EXPECT_EQ(ref_value & ctx.pkt_type_config, ref_value);

	// Video data should be turned on be default since it was not specified.
	ref_value = ctx.pkt_type_config_reference_map.at(Ch10PacketType::VIDEO_DATA_F0);
	EXPECT_EQ(ref_value & ctx.pkt_type_config, ref_value);
}

TEST(Ch10ContextTest, SetPacketTypeConfigConfirmDisabledTypes)
{
	Ch10Context ctx(0);

	std::map<Ch10PacketType, bool> pkt_type_config_map;

	// Turn on 1553 and disable video.
	pkt_type_config_map[Ch10PacketType::MILSTD1553_F1] = true;
	pkt_type_config_map[Ch10PacketType::VIDEO_DATA_F0] = false;

	// Set the configuration.
	ctx.SetPacketTypeConfig(pkt_type_config_map);

	uint64_t ref_value = ctx.pkt_type_config_reference_map.at(Ch10PacketType::MILSTD1553_F1);
	EXPECT_EQ(ref_value & ctx.pkt_type_config, ref_value);

	// Video bit-wise AND ought to evaluate to 0 since it was explicitly turned off.
	ref_value = ctx.pkt_type_config_reference_map.at(Ch10PacketType::VIDEO_DATA_F0);
	EXPECT_EQ(ref_value & ctx.pkt_type_config, 0);
}


