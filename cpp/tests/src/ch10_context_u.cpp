#include <cassert>

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
    bool status = ctx.SetPacketTypeConfig(pkt_type_conf, ctx.pkt_type_config_map);
    EXPECT_TRUE(status);

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
    bool status = ctx.SetPacketTypeConfig(pkt_type_conf, ctx.pkt_type_config_map);
    EXPECT_TRUE(status);
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
    bool status = ctx.SetPacketTypeConfig(pkt_type_conf, ctx.pkt_type_config_map);
    EXPECT_TRUE(status);
    EXPECT_TRUE(ctx.pkt_type_config_map.at(Ch10PacketType::MILSTD1553_F1));

    // Video was explicitly turned off.
    EXPECT_FALSE(ctx.pkt_type_config_map.at(Ch10PacketType::VIDEO_DATA_F0));
}

TEST(Ch10ContextTest, SetPacketTypeConfigDisallowTypeNotInDefault)
{
    Ch10Context ctx(0);

    std::map<Ch10PacketType, bool> pkt_type_conf;

    // Turn on 1553 and disable video.
    pkt_type_conf[Ch10PacketType::MILSTD1553_F1] = true;
    pkt_type_conf[Ch10PacketType::VIDEO_DATA_F0] = false;

    std::unordered_map<Ch10PacketType, bool> default_pkt_type_conf = {
        {Ch10PacketType::MILSTD1553_F1, true}};
    default_pkt_type_conf.erase(Ch10PacketType::VIDEO_DATA_F0);

    // Set the configuration.
    bool status = ctx.SetPacketTypeConfig(pkt_type_conf, default_pkt_type_conf);
    EXPECT_FALSE(status);
}

TEST(Ch10ContextTest, UpdateContextSetVars)
{
    Ch10Context ctx(0);
    Ch10PacketHeaderFmt hdr_fmt{};
    uint64_t abs_pos = 344199919;
    hdr_fmt.pkt_size = 4320;
    hdr_fmt.data_size = 3399;
    hdr_fmt.rtc1 = 321053;
    hdr_fmt.rtc2 = 2976;
    hdr_fmt.chanID = 9;
    hdr_fmt.data_type = static_cast<uint8_t>(Ch10PacketType::MILSTD1553_F1);
    uint64_t rtc = ((uint64_t(hdr_fmt.rtc2) << 32) + uint64_t(hdr_fmt.rtc1)) * 100;
    hdr_fmt.intrapkt_ts_source = 0;
    hdr_fmt.secondary_hdr = 0;
    hdr_fmt.time_format = 0;

    Ch10Status status = ctx.UpdateContext(abs_pos, &hdr_fmt, rtc);
    EXPECT_EQ(status, Ch10Status::OK);
    EXPECT_EQ(abs_pos, ctx.absolute_position);
    EXPECT_EQ(hdr_fmt.pkt_size, ctx.pkt_size);
    EXPECT_EQ(hdr_fmt.data_size, ctx.data_size);
    EXPECT_EQ(rtc, ctx.rtc);
    EXPECT_EQ(hdr_fmt.intrapkt_ts_source, ctx.intrapkt_ts_src);
    EXPECT_EQ(hdr_fmt.time_format, ctx.time_format);
    EXPECT_EQ(hdr_fmt.secondary_hdr, ctx.secondary_hdr);
    EXPECT_TRUE(ctx.chanid_remoteaddr1_map.count(hdr_fmt.chanID) == 1);
}

TEST(Ch10ContextTest, UpdateContextInconclusiveTimeFormat)
{
    Ch10Context ctx(0);
    Ch10PacketHeaderFmt hdr_fmt{};
    uint64_t abs_pos = 344199919;
    uint64_t rtc = 0;
    hdr_fmt.intrapkt_ts_source = 0;
    hdr_fmt.secondary_hdr = 1;
    hdr_fmt.time_format = 1;

    Ch10Status status = ctx.UpdateContext(abs_pos, &hdr_fmt, rtc);
    EXPECT_EQ(status, Ch10Status::TIME_FORMAT_INCONCLUSIVE);

    hdr_fmt.intrapkt_ts_source = 1;
    hdr_fmt.secondary_hdr = 0;
    status = ctx.UpdateContext(abs_pos, &hdr_fmt, rtc);
    EXPECT_EQ(status, Ch10Status::TIME_FORMAT_INCONCLUSIVE);
}

TEST(Ch10ContextTest, UpdateWithTDPDataTDPIsNone)
{
    Ch10Context ctx(0);
    Ch10PacketHeaderFmt hdr_fmt{};
    uint64_t tdp_abs_time = 4772113676;
    uint64_t abs_pos = 344199919;
    hdr_fmt.pkt_size = 4320;
    hdr_fmt.data_size = 3399;
    hdr_fmt.rtc1 = 321053;
    hdr_fmt.rtc2 = 2976;
    uint64_t rtc = ((uint64_t(hdr_fmt.rtc2) << 32) + uint64_t(hdr_fmt.rtc1)) * 100;
    hdr_fmt.intrapkt_ts_source = 0;
    hdr_fmt.secondary_hdr = 0;
    hdr_fmt.time_format = 1;
    uint8_t tdp_doy = 1;

    // Update Context to assign rtc value internally
    Ch10Status status = ctx.UpdateContext(abs_pos, &hdr_fmt, rtc);
    EXPECT_EQ(status, Ch10Status::OK);

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
    Ch10PacketHeaderFmt hdr_fmt{};
    uint64_t abs_pos = 344199919;
    hdr_fmt.pkt_size = 4320;
    hdr_fmt.data_size = 3399;
    hdr_fmt.rtc1 = 321053;
    hdr_fmt.rtc2 = 2976;
    uint64_t rtc = ((uint64_t(hdr_fmt.rtc2) << 32) + uint64_t(hdr_fmt.rtc1)) * 100;
    hdr_fmt.intrapkt_ts_source = 0;
    hdr_fmt.secondary_hdr = 0;
    hdr_fmt.time_format = 0;
    uint8_t tdp_doy = 1;

    // Update Context to assign rtc value internally
    Ch10Status status = ctx.UpdateContext(abs_pos, &hdr_fmt, rtc);
    EXPECT_EQ(status, Ch10Status::OK);

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
    Ch10PacketHeaderFmt hdr_fmt{};

    hdr_fmt.rtc1 = 321053;
    hdr_fmt.rtc2 = 2976;
    uint64_t rtc = ((uint64_t(hdr_fmt.rtc2) << 32) + uint64_t(hdr_fmt.rtc1)) * 100;
    uint64_t abs_pos = 4823829394;
    Ch10Status status = ctx.UpdateContext(abs_pos, &hdr_fmt, rtc);
    EXPECT_EQ(status, Ch10Status::OK);
    assert(status == Ch10Status::OK);

    // Update context with TDP-specific data.
    uint64_t tdp_abs_time = 344199919;
    uint8_t tdp_doy = 0;
    uint8_t tdp_valid = true;
    ctx.UpdateWithTDPData(tdp_abs_time, tdp_doy, tdp_valid);

    uint64_t current_rtc1 = 321200;
    uint64_t current_rtc2 = 502999;
    uint64_t current_rtc = ((uint64_t(current_rtc2) << 32) + uint64_t(current_rtc1)) * 100;
    uint64_t expected_abs_time = tdp_abs_time + (current_rtc - rtc);
    uint64_t calculated_abs_time = ctx.CalculateAbsTimeFromRTCFormat(current_rtc);
    ASSERT_EQ(calculated_abs_time, expected_abs_time);
}

TEST(Ch10ContextTest, GetPacketAbsoluteTimeFromHeaderRTC)
{
    Ch10Context ctx(0);
    Ch10PacketHeaderFmt hdr_fmt{};
    uint64_t tdp_abs_time = 1000000;
    uint64_t tdp_rtc1 = 1;
    uint64_t tdp_rtc2 = 2;
    uint64_t abs_pos = 0;
    hdr_fmt.pkt_size = 4320;
    hdr_fmt.data_size = 3399;
    hdr_fmt.rtc1 = tdp_rtc1;
    hdr_fmt.rtc2 = tdp_rtc2;
    uint64_t rtc_to_ns = 100;
    uint64_t rtc = ((uint64_t(hdr_fmt.rtc2) << 32) + uint64_t(hdr_fmt.rtc1)) * rtc_to_ns;
    hdr_fmt.intrapkt_ts_source = 0;
    hdr_fmt.time_format = 1;
    uint8_t tdp_doy = 1;

    // Update Context to assign rtc value internally
    bool tdp_valid = true;
    Ch10Status status = ctx.UpdateContext(abs_pos, &hdr_fmt, rtc);
    EXPECT_EQ(status, Ch10Status::OK);
    ctx.UpdateWithTDPData(tdp_abs_time, tdp_doy, tdp_valid);

    // Update context as if with a following non-TDP packet
    hdr_fmt.rtc1 += 20;
    rtc = ((uint64_t(hdr_fmt.rtc2) << 32) + uint64_t(hdr_fmt.rtc1)) * rtc_to_ns;
    status = ctx.UpdateContext(abs_pos, &hdr_fmt, rtc);

    uint64_t expected_time = tdp_abs_time + 20 * rtc_to_ns;
    uint64_t packet_time = ctx.GetPacketAbsoluteTimeFromHeaderRTC();
    ASSERT_EQ(expected_time, packet_time);
}

TEST(Ch10ContextTest, CalculateIPTSAbsTimeRTCSource)
{
    Ch10Context ctx(0);
    Ch10PacketHeaderFmt hdr_fmt{};
    uint64_t tdp_abs_time = 1000000;
    uint64_t tdp_rtc1 = 1;
    uint64_t tdp_rtc2 = 2;
    uint64_t abs_pos = 0;
    hdr_fmt.pkt_size = 4320;
    hdr_fmt.data_size = 3399;
    hdr_fmt.rtc1 = tdp_rtc1;
    hdr_fmt.rtc2 = tdp_rtc2;
    uint64_t rtc_to_ns = 100;
    uint64_t rtc = ((uint64_t(hdr_fmt.rtc2) << 32) + uint64_t(hdr_fmt.rtc1)) * rtc_to_ns;
    hdr_fmt.intrapkt_ts_source = 0;
    uint8_t tdp_doy = 1;

    // Update Context to assign rtc value internally
    bool tdp_valid = true;
    Ch10Status status = ctx.UpdateContext(abs_pos, &hdr_fmt, rtc);
    EXPECT_EQ(status, Ch10Status::OK);
    ctx.UpdateWithTDPData(tdp_abs_time, tdp_doy, tdp_valid);

    // Update context as if with a following non-TDP packet
    hdr_fmt.rtc1 += 20;
    rtc = ((uint64_t(hdr_fmt.rtc2) << 32) + uint64_t(hdr_fmt.rtc1)) * rtc_to_ns;
    status = ctx.UpdateContext(abs_pos, &hdr_fmt, rtc);

    uint64_t expected_time = tdp_abs_time + 20 * rtc_to_ns;
    uint64_t packet_time = ctx.CalculateIPTSAbsTime(rtc);
    ASSERT_EQ(expected_time, packet_time);
}

// Needs tests for CalculateIPTSAbsTime for secondary header time
// sources. Exactly how to calculate abs time from these sources has
// not been determined, mostly due to lack of knowledge
// regarding relative or absolute nature of these sources.

TEST(Ch10ContextTest, UpdateChannelIDToLRUAddressMapsRTtoRT)
{
    Ch10Context ctx(0);

    // Update context with "current" header data. Only care about
    // chanID, which will create an entry in the map with an empty
    // set.
    Ch10PacketHeaderFmt hdr_fmt{};
    hdr_fmt.chanID = 4;
    hdr_fmt.intrapkt_ts_source = 0;
    hdr_fmt.secondary_hdr = 0;
    uint64_t abs_pos = 4823829394;
    uint64_t rtc = 0;
    Ch10Status status = ctx.UpdateContext(abs_pos, &hdr_fmt, rtc);
    EXPECT_EQ(status, Ch10Status::OK);

    // Update the maps. If RTtoRT (RR) is 1, then both addr maps 1 and 2
    // will be updated.
    MilStd1553F1DataHeaderCommWordFmt data_hdr;
    const MilStd1553F1DataHeaderCommWordOnlyFmt* cw =
        (const MilStd1553F1DataHeaderCommWordOnlyFmt*)&data_hdr;
    data_hdr.RR = 1;
    data_hdr.remote_addr1 = 10;
    data_hdr.remote_addr2 = 3;
    uint32_t commword_val1 = (uint32_t(cw->comm_word2) << 16) + cw->comm_word1;
    ctx.UpdateChannelIDToLRUAddressMaps(hdr_fmt.chanID, &data_hdr);

    data_hdr.remote_addr1 = 12;
    data_hdr.remote_addr2 = 3;
    uint32_t commword_val2 = (uint32_t(cw->comm_word2) << 16) + cw->comm_word1;
    ctx.UpdateChannelIDToLRUAddressMaps(hdr_fmt.chanID, &data_hdr);

    // Check if the key and values are correct.
    EXPECT_TRUE(ctx.chanid_remoteaddr1_map.count(hdr_fmt.chanID) == 1);
    EXPECT_TRUE(ctx.chanid_remoteaddr2_map.count(hdr_fmt.chanID) == 1);
    EXPECT_TRUE(ctx.chanid_commwords_map.count(hdr_fmt.chanID) == 1);

    EXPECT_THAT(ctx.chanid_remoteaddr1_map.at(hdr_fmt.chanID),
                ::testing::UnorderedElementsAre(10, 12));
    EXPECT_THAT(ctx.chanid_remoteaddr2_map.at(hdr_fmt.chanID),
                ::testing::UnorderedElementsAre(3));
    EXPECT_THAT(ctx.chanid_commwords_map.at(hdr_fmt.chanID),
                ::testing::UnorderedElementsAre(commword_val1, commword_val2));
}

TEST(Ch10ContextTest, UpdateChannelIDToLRUAddressMapsNotRTtoRT)
{
    Ch10Context ctx(0);

    // Update context with "current" header data. Only care about
    // chanID, which will create an entry in the map with an empty
    // set.
    Ch10PacketHeaderFmt hdr_fmt{};
    hdr_fmt.chanID = 4;
    hdr_fmt.intrapkt_ts_source = 0;
    hdr_fmt.secondary_hdr = 0;
    hdr_fmt.data_type = static_cast<uint8_t>(Ch10PacketType::MILSTD1553_F1);
    uint64_t abs_pos = 4823829394;
    uint64_t rtc = 0;
    Ch10Status status = ctx.UpdateContext(abs_pos, &hdr_fmt, rtc);

    // Update the maps. If RTtoRT is 1, then both addr maps 1 and 2
    // will be updated.
    MilStd1553F1DataHeaderCommWordFmt data_hdr;
    const MilStd1553F1DataHeaderCommWordOnlyFmt* cw =
        (const MilStd1553F1DataHeaderCommWordOnlyFmt*)&data_hdr;
    data_hdr.RR = 0;
    data_hdr.remote_addr1 = 10;
    data_hdr.remote_addr2 = 3;
    data_hdr.tx1 = 1;
    uint32_t commword_val1 = uint32_t(cw->comm_word1) << 16;
    ctx.UpdateChannelIDToLRUAddressMaps(hdr_fmt.chanID, &data_hdr);

    data_hdr.remote_addr1 = 12;
    data_hdr.remote_addr2 = 3;
    data_hdr.tx1 = 0;
    uint32_t commword_val2 = cw->comm_word1;
    ctx.UpdateChannelIDToLRUAddressMaps(hdr_fmt.chanID, &data_hdr);

    // Check if the key and values are correct.
    EXPECT_TRUE(ctx.chanid_remoteaddr1_map.count(hdr_fmt.chanID) == 1);
    EXPECT_TRUE(ctx.chanid_remoteaddr2_map.count(hdr_fmt.chanID) == 1);

    EXPECT_THAT(ctx.chanid_remoteaddr1_map.at(hdr_fmt.chanID),
                ::testing::UnorderedElementsAre(10, 12));
    EXPECT_EQ(ctx.chanid_remoteaddr2_map.at(hdr_fmt.chanID).size(), 0);
    EXPECT_THAT(ctx.chanid_commwords_map.at(hdr_fmt.chanID),
                ::testing::UnorderedElementsAre(commword_val1, commword_val2));
}

TEST(Ch10ContextTest, CheckConfigurationPathsNotRequired)
{
    Ch10Context ctx(0);

    // Note: TSP not defined and set to true. Normally this is ensured by other
    // functions. However, TMATS and TDP (normally set to true) do not require
    // paths and the lack of paths for either should not indicate an erroneous
    // configuration.
    std::unordered_map<Ch10PacketType, bool> pkt_type_enabled = {
        {Ch10PacketType::COMPUTER_GENERATED_DATA_F1, true}};

    std::map<Ch10PacketType, ManagedPath> enabled_paths;

    // No paths defined.
    std::map<Ch10PacketType, ManagedPath> pkt_type_paths;
    bool config_ok = ctx.CheckConfiguration(pkt_type_enabled, pkt_type_paths,
                                            enabled_paths);
    EXPECT_TRUE(config_ok);
    EXPECT_EQ(enabled_paths.size(), 0);

    // Try both non-required types set to true.
    enabled_paths.clear();
    pkt_type_enabled[Ch10PacketType::TIME_DATA_F1] = true;
    config_ok = ctx.CheckConfiguration(pkt_type_enabled, pkt_type_paths,
                                       enabled_paths);
    EXPECT_TRUE(config_ok);
    EXPECT_EQ(enabled_paths.size(), 0);

    // Add video path, even though it's not enabled.
    enabled_paths.clear();
    pkt_type_paths[Ch10PacketType::VIDEO_DATA_F0] = ManagedPath();
    config_ok = ctx.CheckConfiguration(pkt_type_enabled, pkt_type_paths,
                                       enabled_paths);
    EXPECT_TRUE(config_ok);
    EXPECT_EQ(enabled_paths.size(), 0);

    // Add video and 1553, but not enabled.
    enabled_paths.clear();
    pkt_type_enabled[Ch10PacketType::VIDEO_DATA_F0] = false;
    pkt_type_enabled[Ch10PacketType::MILSTD1553_F1] = false;
    config_ok = ctx.CheckConfiguration(pkt_type_enabled, pkt_type_paths,
                                       enabled_paths);
    EXPECT_TRUE(config_ok);
    EXPECT_EQ(enabled_paths.size(), 0);
}

TEST(Ch10ContextTest, CheckConfigurationPathsRequired)
{
    Ch10Context ctx(0);

    // Default minimal config for this map.
    std::unordered_map<Ch10PacketType, bool> pkt_type_enabled = {
        {Ch10PacketType::COMPUTER_GENERATED_DATA_F1, true},
        {Ch10PacketType::TIME_DATA_F1, true}};
    std::map<Ch10PacketType, ManagedPath> enabled_paths;

    // Add 1533 and video, with only 1553 enabled ==> 1553 path required.
    pkt_type_enabled[Ch10PacketType::VIDEO_DATA_F0] = false;
    pkt_type_enabled[Ch10PacketType::MILSTD1553_F1] = true;
    std::map<Ch10PacketType, ManagedPath> pkt_type_paths;
    bool config_ok = ctx.CheckConfiguration(pkt_type_enabled, pkt_type_paths,
                                            enabled_paths);
    EXPECT_FALSE(config_ok);
    EXPECT_EQ(enabled_paths.size(), 0);

    // Set video to enabled.
    enabled_paths.clear();
    pkt_type_enabled[Ch10PacketType::VIDEO_DATA_F0] = true;
    config_ok = ctx.CheckConfiguration(pkt_type_enabled, pkt_type_paths,
                                       enabled_paths);
    EXPECT_FALSE(config_ok);
    EXPECT_EQ(enabled_paths.size(), 0);

    // Insert video path. 1553 path still not present. Configuration fails.
    enabled_paths.clear();
    pkt_type_paths[Ch10PacketType::VIDEO_DATA_F0] = ManagedPath();
    config_ok = ctx.CheckConfiguration(pkt_type_enabled, pkt_type_paths,
                                       enabled_paths);
    EXPECT_FALSE(config_ok);
    EXPECT_EQ(enabled_paths.size(), 0);

    // Insert 1553 path.
    enabled_paths.clear();
    pkt_type_paths[Ch10PacketType::MILSTD1553_F1] = ManagedPath();
    config_ok = ctx.CheckConfiguration(pkt_type_enabled, pkt_type_paths,
                                       enabled_paths);
    EXPECT_TRUE(config_ok);
    EXPECT_EQ(enabled_paths.count(Ch10PacketType::VIDEO_DATA_F0), 1);
    EXPECT_EQ(enabled_paths.count(Ch10PacketType::MILSTD1553_F1), 1);

    // Disable video, path still present shouldn't matter. Config should
    // be ok.
    enabled_paths.clear();
    pkt_type_enabled[Ch10PacketType::VIDEO_DATA_F0] = false;
    config_ok = ctx.CheckConfiguration(pkt_type_enabled, pkt_type_paths,
                                       enabled_paths);
    EXPECT_TRUE(config_ok);
    EXPECT_EQ(enabled_paths.count(Ch10PacketType::VIDEO_DATA_F0), 0);
    EXPECT_EQ(enabled_paths.count(Ch10PacketType::MILSTD1553_F1), 1);
}

TEST(Ch10ContextTest, InitializeAndCloseFileWriters)
{
    Ch10Context ctx(0);
    std::map<Ch10PacketType, ManagedPath> enabled_paths{
        {Ch10PacketType::MILSTD1553_F1, ManagedPath{"..", "parsed1553_test.parquet"}},
        {Ch10PacketType::VIDEO_DATA_F0, ManagedPath{"..", "parsedvideo_test.parquet"}},
        {Ch10PacketType::ETHERNET_DATA_F0, ManagedPath{"..", "parsedeth_test.parquet"}},
        {Ch10PacketType::ARINC429_F0, ManagedPath{"..", "parsed429_test.parquet"}},
    };
    ASSERT_TRUE(ctx.InitializeFileWriters(enabled_paths));

    ctx.CloseFileWriters();
}

TEST(Ch10ContextTest, InitializeFileWriters1553Fail)
{
    Ch10Context ctx(0);

    // Empty path ought to fail at pq_ctx_->OpenForWrite()
    std::map<Ch10PacketType, ManagedPath> enabled_paths{
        {Ch10PacketType::MILSTD1553_F1, ManagedPath("")},
    };
    ASSERT_FALSE(ctx.InitializeFileWriters(enabled_paths));
}

TEST(Ch10ContextTest, InitializeFileWritersDefault)
{
    Ch10Context ctx(0);

    std::map<Ch10PacketType, ManagedPath> enabled_paths{
        {Ch10PacketType::NONE, ManagedPath("")},
    };
    ASSERT_TRUE(ctx.InitializeFileWriters(enabled_paths));
}

TEST(Ch10ContextTest, RecordMinVideoTimeStampChannelIDNotPresent)
{
    Ch10Context ctx(0);
    Ch10PacketHeaderFmt hdr_fmt{};
    hdr_fmt.chanID = 9;
    uint64_t abs_pos = 0;
    uint64_t rtc = 0;

    // For this test, we're only concerned with setting a specific channel ID.
    // None of the other parameters in the Ch10PacketHeaderFmt are used.
    Ch10Status status = ctx.UpdateContext(abs_pos, &hdr_fmt, rtc);
    EXPECT_EQ(status, Ch10Status::OK);

    uint64_t ts = 3939292991;

    // The Ch10Context instance is new, therefore the map should be initially empty.
    EXPECT_EQ(ctx.chanid_minvideotimestamp_map.size(), 0);
    ctx.RecordMinVideoTimeStamp(ts);
    EXPECT_EQ(ctx.chanid_minvideotimestamp_map.count(hdr_fmt.chanID), 1);
}

TEST(Ch10ContextTest, RecordMinVideoTimeStampLessThan)
{
    Ch10Context ctx(0);
    Ch10PacketHeaderFmt hdr_fmt{};
    hdr_fmt.chanID = 9;
    uint64_t abs_pos = 0;
    uint64_t rtc = 0;

    // For this test, we're only concerned with setting a specific channel ID.
    // None of the other parameters in the Ch10PacketHeaderFmt are used.
    Ch10Status status = ctx.UpdateContext(abs_pos, &hdr_fmt, rtc);
    EXPECT_EQ(status, Ch10Status::OK);

    uint64_t ts = 3939292991;
    ctx.RecordMinVideoTimeStamp(ts);
    ts = 3939291991;
    ctx.RecordMinVideoTimeStamp(ts);
    EXPECT_EQ(ctx.chanid_minvideotimestamp_map.count(hdr_fmt.chanID), 1);
    EXPECT_EQ(ctx.chanid_minvideotimestamp_map.at(hdr_fmt.chanID), ts);
}

TEST(Ch10ContextTest, RecordMinVideoTimeStampGreaterThan)
{
    Ch10Context ctx(0);
    Ch10PacketHeaderFmt hdr_fmt{};
    hdr_fmt.chanID = 9;
    uint64_t abs_pos = 0;
    uint64_t rtc = 0;

    // For this test, we're only concerned with setting a specific channel ID.
    // None of the other parameters in the Ch10PacketHeaderFmt are used.
    Ch10Status status = ctx.UpdateContext(abs_pos, &hdr_fmt, rtc);

    uint64_t ts1 = 3939292991;
    ctx.RecordMinVideoTimeStamp(ts1);
    uint64_t ts2 = 3939294991;
    ctx.RecordMinVideoTimeStamp(ts2);
    EXPECT_EQ(ctx.chanid_minvideotimestamp_map.count(hdr_fmt.chanID), 1);
    EXPECT_EQ(ctx.chanid_minvideotimestamp_map.at(hdr_fmt.chanID), ts1);
}

TEST(Ch10ContextTest, Calculate429WordAbsTimeRTCSource)
{
    Ch10Context ctx(0);
    Ch10PacketHeaderFmt hdr_fmt{};
    uint64_t tdp_abs_time = 1000000;
    uint64_t tdp_rtc1 = 1;
    uint64_t tdp_rtc2 = 2;
    uint64_t abs_pos = 0;
    hdr_fmt.pkt_size = 4320;
    hdr_fmt.data_size = 3399;
    hdr_fmt.rtc1 = tdp_rtc1;
    hdr_fmt.rtc2 = tdp_rtc2;
    uint64_t rtc_to_ns = 100;
    uint64_t rtc = ((uint64_t(hdr_fmt.rtc2) << 32) + uint64_t(hdr_fmt.rtc1)) * rtc_to_ns;
    hdr_fmt.intrapkt_ts_source = 0;
    uint8_t tdp_doy = 1;

    uint64_t total_gap_time = 121;
    uint64_t gap_time_to_ns = 100;

    // Update Context to assign rtc value internally
    bool tdp_valid = true;
    Ch10Status status = ctx.UpdateContext(abs_pos, &hdr_fmt, rtc);
    EXPECT_EQ(status, Ch10Status::OK);
    ctx.UpdateWithTDPData(tdp_abs_time, tdp_doy, tdp_valid);

    // Update context as if with a following non-TDP packet
    hdr_fmt.rtc1 += 20;
    rtc = ((uint64_t(hdr_fmt.rtc2) << 32) + uint64_t(hdr_fmt.rtc1)) * rtc_to_ns;
    status = ctx.UpdateContext(abs_pos, &hdr_fmt, rtc);

    uint64_t expected_time = tdp_abs_time + 20 * rtc_to_ns + total_gap_time * gap_time_to_ns;
    uint64_t packet_time = ctx.Calculate429WordAbsTime(total_gap_time);
    ASSERT_EQ(expected_time, packet_time);
}

TEST(Ch10ContextTest, Calculate429AbsTimeSecondaryHeaderSource)
{
    Ch10Context ctx(0);
    Ch10PacketHeaderFmt hdr_fmt{};
    uint64_t tdp_abs_time = 1000000;
    uint64_t tdp_rtc1 = 1;
    uint64_t tdp_rtc2 = 2;
    uint64_t abs_pos = 0;
    hdr_fmt.pkt_size = 4320;
    hdr_fmt.data_size = 3399;
    hdr_fmt.rtc1 = tdp_rtc1;
    hdr_fmt.rtc2 = tdp_rtc2;
    uint64_t rtc_to_ns = 100;
    uint64_t rtc = ((uint64_t(hdr_fmt.rtc2) << 32) + uint64_t(hdr_fmt.rtc1)) * rtc_to_ns;
    uint8_t tdp_doy = 1;

    // Use Secondary Header
    hdr_fmt.intrapkt_ts_source = 1;
    hdr_fmt.secondary_hdr = 1;

    // Update Context to assign rtc value internally
    bool tdp_valid = true;
    Ch10Status status = ctx.UpdateContext(abs_pos, &hdr_fmt, rtc);
    EXPECT_EQ(status, Ch10Status::OK);
    ctx.UpdateWithTDPData(tdp_abs_time, tdp_doy, tdp_valid);

    // Update context as if with a following non-TDP packet
    hdr_fmt.rtc1 += 20;
    rtc = ((uint64_t(hdr_fmt.rtc2) << 32) + uint64_t(hdr_fmt.rtc1)) * rtc_to_ns;
    status = ctx.UpdateContext(abs_pos, &hdr_fmt, rtc);

    // Define seconday header and time gap info
    uint64_t total_gap_time = 121;
    uint64_t gap_time_to_ns = 100;
    uint64_t packet_abs_time = 1007000;

    // Matching the context above (where updated with non-TDP packet) the packet's
    // absolute time would be 1002000.
    // If the secondary header abs time is used (meaning packet_abs_time = 1007000),
    // the Word's Abs Time will be 1007000 + total_gap_time * gap_time_to_ns = 1019100.
    // Test will fail if RTC derived time is used making Word's Abs Time = 1014100.

    // Update Context with Secondary Header Time
    ctx.UpdateWithSecondaryHeaderTime(packet_abs_time);

    uint64_t expected_time = packet_abs_time + total_gap_time * gap_time_to_ns;
    uint64_t packet_time = ctx.Calculate429WordAbsTime(total_gap_time);
    ASSERT_EQ(expected_time, packet_time);
}

TEST(Ch10ContextTest, UpdateARINC429Maps)
{
    Ch10Context ctx(0);

    // Update context with "current" header data. Only care about
    // chanID, which will create an entry in the map with an empty
    // set.
    Ch10PacketHeaderFmt hdr_fmt{};
    hdr_fmt.chanID = 4;
    hdr_fmt.intrapkt_ts_source = 0;
    hdr_fmt.secondary_hdr = 0;
    hdr_fmt.data_type = static_cast<uint8_t>(Ch10PacketType::ARINC429_F0);
    uint64_t abs_pos = 4823829394;
    uint64_t rtc = 0;
    Ch10Status status = ctx.UpdateContext(abs_pos, &hdr_fmt, rtc);

    // Update the maps.
    ARINC429F0MsgFmt data_hdr;
    data_hdr.label = 202;
    data_hdr.bus = 1;
    ctx.UpdateARINC429Maps(hdr_fmt.chanID, &data_hdr);

    data_hdr.label = 194;
    data_hdr.bus = 2;
    ctx.UpdateARINC429Maps(hdr_fmt.chanID, &data_hdr);

    data_hdr.label = 194;
    data_hdr.bus = 3;
    ctx.UpdateARINC429Maps(hdr_fmt.chanID, &data_hdr);

    // Check if the key and values are correct.
    EXPECT_TRUE(ctx.chanid_labels_map.count(hdr_fmt.chanID) == 1);
    EXPECT_TRUE(ctx.chanid_busnumbers_map.count(hdr_fmt.chanID) == 1);

    EXPECT_EQ(ctx.chanid_busnumbers_map.at(hdr_fmt.chanID).size(), 3);
    EXPECT_THAT(ctx.chanid_busnumbers_map.at(hdr_fmt.chanID),
                ::testing::UnorderedElementsAre(1, 2, 3));

    EXPECT_EQ(ctx.chanid_labels_map.at(hdr_fmt.chanID).size(), 2);
    EXPECT_THAT(ctx.chanid_labels_map.at(hdr_fmt.chanID),
                ::testing::UnorderedElementsAre(202, 194));
}

TEST(Ch10ContextTest, IsPacketTypeEnabledNotInMap)
{
    Ch10Context ctx(0);

    // None not in default map
    EXPECT_FALSE(ctx.IsPacketTypeEnabled(Ch10PacketType::NONE));
}

TEST(Ch10ContextTest, IsPacketTypeEnabledTrue)
{
    Ch10Context ctx(0);
    std::map<Ch10PacketType, bool> pkt_type_conf;
    pkt_type_conf[Ch10PacketType::MILSTD1553_F1] = true;
    std::unordered_map<Ch10PacketType, bool> default_pkt_type_conf = {
        {Ch10PacketType::MILSTD1553_F1, true}};

    // Set the configuration.
    bool status = ctx.SetPacketTypeConfig(pkt_type_conf, default_pkt_type_conf);
    ASSERT_TRUE(status);

    EXPECT_TRUE(ctx.IsPacketTypeEnabled(Ch10PacketType::MILSTD1553_F1));
    EXPECT_TRUE(ctx.parsed_packet_types.count(Ch10PacketType::MILSTD1553_F1) == 1);
}

TEST(Ch10ContextTest, IsPacketTypeEnabledFalse)
{
    Ch10Context ctx(0);
    std::map<Ch10PacketType, bool> pkt_type_conf;
    pkt_type_conf[Ch10PacketType::MILSTD1553_F1] = false;
    std::unordered_map<Ch10PacketType, bool> default_pkt_type_conf = {
        {Ch10PacketType::MILSTD1553_F1, true}};

    // Set the configuration.
    bool status = ctx.SetPacketTypeConfig(pkt_type_conf, default_pkt_type_conf);
    ASSERT_TRUE(status);

    EXPECT_FALSE(ctx.IsPacketTypeEnabled(Ch10PacketType::MILSTD1553_F1));
    EXPECT_TRUE(ctx.parsed_packet_types.count(Ch10PacketType::MILSTD1553_F1) == 0);
}

TEST(Ch10ContextTest, RegisterUnhandledPacketType)
{
    Ch10Context ctx(0);
    Ch10PacketType anaf1 = Ch10PacketType::ANALOG_F1;
    Ch10PacketType pcmf0 = Ch10PacketType::PCM_F0;
    Ch10PacketType pcmf1 = Ch10PacketType::PCM_F1;

    EXPECT_TRUE(ctx.RegisterUnhandledPacketType(anaf1));
    EXPECT_TRUE(ctx.RegisterUnhandledPacketType(pcmf0));
    EXPECT_FALSE(ctx.RegisterUnhandledPacketType(anaf1));
    EXPECT_TRUE(ctx.RegisterUnhandledPacketType(pcmf1));
    EXPECT_FALSE(ctx.RegisterUnhandledPacketType(pcmf0));
    EXPECT_FALSE(ctx.RegisterUnhandledPacketType(pcmf1));
}