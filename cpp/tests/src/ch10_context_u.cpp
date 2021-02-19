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
	Ch10PacketHeaderFmt hdr_fmt;
	uint64_t abs_pos = 344199919;
	hdr_fmt.pkt_size = 4320;
	hdr_fmt.data_size = 3399;
	hdr_fmt.rtc1 = 321053;
	hdr_fmt.rtc2 = 502976;
	hdr_fmt.chanID = 9;
	uint64_t rtc = ((uint64_t(hdr_fmt.rtc2) << 32) + uint64_t(hdr_fmt.rtc1)) * 100;
	hdr_fmt.intrapkt_ts_source = 0;
	hdr_fmt.time_format = 1;

	ctx.UpdateContext(abs_pos, &hdr_fmt);
	EXPECT_EQ(abs_pos, ctx.absolute_position);
	EXPECT_EQ(hdr_fmt.pkt_size, ctx.pkt_size);
	EXPECT_EQ(hdr_fmt.data_size, ctx.data_size);
	EXPECT_EQ(rtc, ctx.rtc);
	EXPECT_EQ(hdr_fmt.intrapkt_ts_source, ctx.intrapkt_ts_src);
	EXPECT_EQ(hdr_fmt.time_format, ctx.time_format);
	EXPECT_TRUE(ctx.chanid_remoteaddr1_map.count(hdr_fmt.chanID) == 1);
}

TEST(Ch10ContextTest, UpdateWithTDPDataTDPIsNone)
{
	Ch10Context ctx(0);
	Ch10PacketHeaderFmt hdr_fmt;
	uint64_t tdp_abs_time = 4772113676;
	uint64_t abs_pos = 344199919;
	hdr_fmt.pkt_size = 4320;
	hdr_fmt.data_size = 3399;
	hdr_fmt.rtc1 = 321053;
	hdr_fmt.rtc2 = 502976;
	uint64_t rtc = ((uint64_t(hdr_fmt.rtc2) << 32) + uint64_t(hdr_fmt.rtc1)) * 100;
	hdr_fmt.intrapkt_ts_source = 0;
	hdr_fmt.time_format = 1;
	uint8_t tdp_doy = 1;

	// Update Context to assign rtc value internally
	ctx.UpdateContext(abs_pos, &hdr_fmt);
	
	// Update with TDP, but set tdp_valid bool to false = invalid.
	bool tdp_valid = false;
	ctx.UpdateWithTDPData(tdp_abs_time, tdp_doy, tdp_valid);

	EXPECT_EQ(ctx.tdp_valid, tdp_valid);

	// Confirm that certain variables have NOT been updated to 
	// reflect whatever is passed into the function.
	EXPECT_FALSE(ctx.tdp_abs_time == tdp_abs_time);
	EXPECT_FALSE(ctx.tdp_rtc == rtc);
}

TEST(Ch10ContextTest, UpdateWithTDPDataVarsUpdated)
{
	Ch10Context ctx(0);
	uint64_t tdp_abs_time = 344199919;
	Ch10PacketHeaderFmt hdr_fmt;
	uint64_t abs_pos = 344199919;
	hdr_fmt.pkt_size = 4320;
	hdr_fmt.data_size = 3399;
	hdr_fmt.rtc1 = 321053;
	hdr_fmt.rtc2 = 502976;
	uint64_t rtc = ((uint64_t(hdr_fmt.rtc2) << 32) + uint64_t(hdr_fmt.rtc1)) * 100;
	hdr_fmt.intrapkt_ts_source = 0;
	hdr_fmt.time_format = 1;
	uint8_t tdp_doy = 1;

	// Update Context to assign rtc value internally
	ctx.UpdateContext(abs_pos, &hdr_fmt);

	// Update with TDP valid
	bool tdp_valid = true;
	ctx.UpdateWithTDPData(tdp_abs_time, tdp_doy, tdp_valid);

	EXPECT_EQ(ctx.tdp_valid, tdp_valid);

	// Confirm that certain variables have been updated to 
	// reflect whatever is passed into the function.
	EXPECT_EQ(ctx.tdp_abs_time, tdp_abs_time);
	EXPECT_EQ(ctx.tdp_rtc, rtc);
	EXPECT_EQ(ctx.tdp_doy, tdp_doy);
	EXPECT_EQ(ctx.found_tdp, true);
}

TEST(Ch10ContextTest, CalculateAbsTimeFromRTCFormat)
{
	Ch10Context ctx(0);

	// Update context with "current" header data.
	Ch10PacketHeaderFmt hdr_fmt;
	hdr_fmt.rtc1 = 321053;
	hdr_fmt.rtc2 = 502976;
	uint64_t rtc = ((uint64_t(hdr_fmt.rtc2) << 32) + uint64_t(hdr_fmt.rtc1)) * 100;
	uint64_t abs_pos = 4823829394;
	ctx.UpdateContext(abs_pos, &hdr_fmt);

	// Update context with TDP-specific data.
	uint64_t tdp_abs_time = 344199919;
	uint8_t tdp_doy = 0;
	uint8_t tdp_valid = true;
	ctx.UpdateWithTDPData(tdp_abs_time, tdp_doy, tdp_valid);

	uint32_t current_rtc1 = 321200;
	uint32_t current_rtc2 = 502999;
	uint64_t current_rtc = ((uint64_t(current_rtc2) << 32) + uint64_t(current_rtc1)) * 100;
	uint64_t expected_abs_time = tdp_abs_time + (current_rtc - rtc);
	uint64_t calculated_abs_time = ctx.CalculateAbsTimeFromRTCFormat(current_rtc1,
		current_rtc2);
	ASSERT_EQ(calculated_abs_time, expected_abs_time);
}

TEST(Ch10ContextTest, UpdateChannelIDToLRUAddressMapsRTtoRT)
{
	Ch10Context ctx(0);

	// Update context with "current" header data. Only care about 
	// chanID, which will create an entry in the map with an empty 
	// set.
	Ch10PacketHeaderFmt hdr_fmt;
	hdr_fmt.chanID = 4;
	uint64_t abs_pos = 4823829394;
	ctx.UpdateContext(abs_pos, &hdr_fmt);

	// Update the maps. If RTtoRT is 1, then both addr maps 1 and 2 
	// will be updated.
	uint16_t RTtoRT = 1;
	uint16_t remoteaddr1 = 10;
	uint16_t remoteaddr2 = 3;
	ctx.UpdateChannelIDToLRUAddressMaps(hdr_fmt.chanID, RTtoRT, remoteaddr1,
		remoteaddr2);

	remoteaddr1 = 12;
	remoteaddr2 = 3;
	ctx.UpdateChannelIDToLRUAddressMaps(hdr_fmt.chanID, RTtoRT, remoteaddr1,
		remoteaddr2);

	// Check if the key and values are correct.
	EXPECT_TRUE(ctx.chanid_remoteaddr1_map.count(hdr_fmt.chanID) == 1);
	EXPECT_TRUE(ctx.chanid_remoteaddr2_map.count(hdr_fmt.chanID) == 1);

	EXPECT_THAT(ctx.chanid_remoteaddr1_map.at(hdr_fmt.chanID), 
		::testing::UnorderedElementsAre(10, 12 ));
	EXPECT_THAT(ctx.chanid_remoteaddr2_map.at(hdr_fmt.chanID), 
		::testing::UnorderedElementsAre(3));
}

TEST(Ch10ContextTest, UpdateChannelIDToLRUAddressMapsNotRTtoRT)
{
	Ch10Context ctx(0);

	// Update context with "current" header data. Only care about 
	// chanID, which will create an entry in the map with an empty 
	// set.
	Ch10PacketHeaderFmt hdr_fmt;
	hdr_fmt.chanID = 4;
	uint64_t abs_pos = 4823829394;
	ctx.UpdateContext(abs_pos, &hdr_fmt);

	// Update the maps. If RTtoRT is 1, then both addr maps 1 and 2 
	// will be updated.
	uint16_t RTtoRT = 0;
	uint16_t remoteaddr1 = 10;
	uint16_t remoteaddr2 = 3;
	ctx.UpdateChannelIDToLRUAddressMaps(hdr_fmt.chanID, RTtoRT, remoteaddr1,
		remoteaddr2);

	remoteaddr1 = 12;
	remoteaddr2 = 3;
	ctx.UpdateChannelIDToLRUAddressMaps(hdr_fmt.chanID, RTtoRT, remoteaddr1,
		remoteaddr2);

	// Check if the key and values are correct.
	EXPECT_TRUE(ctx.chanid_remoteaddr1_map.count(hdr_fmt.chanID) == 1);
	EXPECT_TRUE(ctx.chanid_remoteaddr2_map.count(hdr_fmt.chanID) == 1);

	EXPECT_THAT(ctx.chanid_remoteaddr1_map.at(hdr_fmt.chanID),
		::testing::UnorderedElementsAre(10, 12));
	EXPECT_EQ(ctx.chanid_remoteaddr2_map.at(hdr_fmt.chanID).size(), 0);
}
