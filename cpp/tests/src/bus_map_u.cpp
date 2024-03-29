#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "bus_map.h"
#include "file_reader.h"
#include "icd_data.h"

class BusMapTest : public ::testing::Test
{
   protected:
    std::unordered_map<uint64_t, std::set<std::string>> icd_message_key_to_busnames_map;
    std::unordered_map<uint64_t, std::set<uint64_t>> icd_message_key_to_channelids_map;
    std::map<uint64_t, std::string> tmats_chanid_to_source_map;
    BusMap b;
    std::map<uint64_t, std::string> tmats_1553_chanid_compare_map;
    IterableTools iterable_tools_;
    FileReader fr;
    uint64_t mask = UINT64_MAX;
    uint64_t vote_threshold = 0;
    bool vote_method_checks_tmats = false;
    std::set<std::string> bus_name_exclusions;

    BusMapTest()
    {
    }
    void SetUp() override
    {
    }

    template <typename Map>
    bool map_compare(Map const& lhs, Map const& rhs)
    {
        return lhs.size() == rhs.size() && std::equal(lhs.begin(), lhs.end(),
                                                      rhs.begin());
    }
};

// User decides to run translation
TEST_F(BusMapTest, UserAdjustmentsUserSpecifiesContinue)
{
    std::vector<std::string> adj_vec = {"1"};
    bool continue_translation =
        b.UserAdjustments(&adj_vec);
    EXPECT_TRUE(continue_translation);
}

// User decides to quit translation
TEST_F(BusMapTest, UserAdjustmentsUserSpecifiesQuit)
{
    std::vector<std::string> adj_vec = {"q"};
    bool continue_translation =
        b.UserAdjustments(&adj_vec);
    EXPECT_FALSE(continue_translation);
}

// Junk then continue
TEST_F(BusMapTest, UserAdjustmentsJunkInputThenContinue)
{
    std::vector<std::string> adj_vec = {"4", "0", "a", "1"};
    bool continue_translation =
        b.UserAdjustments(&adj_vec);
    EXPECT_TRUE(continue_translation);
}

// Junk then quit
TEST_F(BusMapTest, UserAdjustmentsJunkInputThenQuit)
{
    std::vector<std::string> adj_vec = {"4", "0", "a", "q"};
    bool continue_translation =
        b.UserAdjustments(&adj_vec);
    EXPECT_FALSE(continue_translation);
}

// Junk -> Adjust then quit
TEST_F(BusMapTest, UserAdjustmentsJunkInputThenAdjustThenQuit)
{
    std::vector<std::string> adj_vec = {"4", "0", "2", "q"};
    bool continue_translation =
        b.UserAdjustments(&adj_vec);
    EXPECT_FALSE(continue_translation);
}

// Adjust -> make a change to channel ID -> then quit
TEST_F(BusMapTest, UserAdjustmentsAdjustThenValidChannelIDThenQuit)
{
    icd_message_key_to_busnames_map[10 & mask] = std::set<std::string>({"BUS1"});
    icd_message_key_to_busnames_map[11 & mask] = std::set<std::string>({"BUS2"});

    b.InitializeMaps(&icd_message_key_to_busnames_map,
                     std::set<uint64_t>({1, 5}),
                     mask,
                     vote_threshold,
                     vote_method_checks_tmats,
                     bus_name_exclusions,
                     tmats_chanid_to_source_map);

    std::vector<uint64_t> transmit_cmds = std::vector<uint64_t>({0});
    std::vector<uint64_t> recieve_cmds = std::vector<uint64_t>({10});
    std::vector<uint64_t> channel_ids = std::vector<uint64_t>({1});  // channel id 1 match and channel id 5 missing

    EXPECT_TRUE(b.SubmitMessages(transmit_cmds, recieve_cmds, channel_ids));

    std::vector<std::string> adj_vec = {"2", "1", "q"};

    std::map<uint64_t, std::string> res;
    bool continue_translation = b.Finalize(res, 0, true, &adj_vec);
    EXPECT_FALSE(continue_translation);
    EXPECT_TRUE(iterable_tools_.GetKeys(res).size() == 0);
}

// Adjust -> invalid channel id -> then quit
TEST_F(BusMapTest, UserAdjustmentsAdjustThenInvalidChannelIDThenQuit)
{
    icd_message_key_to_busnames_map[10 & mask] = std::set<std::string>({"BUS1"});
    icd_message_key_to_busnames_map[11 & mask] = std::set<std::string>({"BUS2"});

    b.InitializeMaps(&icd_message_key_to_busnames_map,
                     std::set<uint64_t>({1, 5}),
                     mask,
                     vote_threshold,
                     vote_method_checks_tmats,
                     bus_name_exclusions,
                     tmats_chanid_to_source_map);

    std::vector<uint64_t> transmit_cmds = std::vector<uint64_t>({0});
    std::vector<uint64_t> recieve_cmds = std::vector<uint64_t>({10});
    std::vector<uint64_t> channel_ids = std::vector<uint64_t>({1});  // channel id 1 match and channel id 5 missing

    EXPECT_TRUE(b.SubmitMessages(transmit_cmds, recieve_cmds, channel_ids));

    std::vector<std::string> adj_vec = {"2", "invalid", "q"};
    std::map<uint64_t, std::string> res;
    bool continue_translation = b.Finalize(res, 0, true, &adj_vec);
    EXPECT_FALSE(continue_translation);
    EXPECT_TRUE(iterable_tools_.GetKeys(res).size() == 0);
}

// Adjust -> valid channel id -> invalid bus names -> then quit
TEST_F(BusMapTest, UserAdjustmentsAdjustThenValidChannelIDThenInvalidBusNameThenQuit)
{
    icd_message_key_to_busnames_map[10 & mask] = std::set<std::string>({"BUS1"});
    icd_message_key_to_busnames_map[11 & mask] = std::set<std::string>({"BUS2"});

    b.InitializeMaps(&icd_message_key_to_busnames_map,
                     std::set<uint64_t>({1, 5}),
                     mask,
                     vote_threshold,
                     vote_method_checks_tmats,
                     bus_name_exclusions,
                     tmats_chanid_to_source_map);

    std::vector<uint64_t> transmit_cmds = std::vector<uint64_t>({0});
    std::vector<uint64_t> recieve_cmds = std::vector<uint64_t>({10});
    std::vector<uint64_t> channel_ids = std::vector<uint64_t>({1});  // channel id 1 match and channel id 5 missing

    EXPECT_TRUE(b.SubmitMessages(transmit_cmds, recieve_cmds, channel_ids));

    std::vector<std::string> adj_vec = {"2", "1", "invalid", "q"};
    std::map<uint64_t, std::string> res;
    bool continue_translation = b.Finalize(res, 0, true, &adj_vec);
    EXPECT_FALSE(continue_translation);
    EXPECT_TRUE(iterable_tools_.GetKeys(res).size() == 0);
}

TEST_F(BusMapTest, UserAdjustmentsAdjustThenInvalidChannelIDThenValidChannelIDThenInvalidBusNameThenValidBusName)
{
    icd_message_key_to_busnames_map[10 & mask] = std::set<std::string>({"BUS1"});
    icd_message_key_to_busnames_map[11 & mask] = std::set<std::string>({"BUS2"});
    icd_message_key_to_busnames_map[12 & mask] = std::set<std::string>({"BUS3"});

    // bus name exclusions should not limit the user from
    // manual selection
    bus_name_exclusions.insert("BUS3");

    tmats_chanid_to_source_map[1] = "BUS1";
    // TMATs bus "BUS3" does not exist,
    // but this should not limit the user from
    // choosing a bus for Channel ID 2
    vote_method_checks_tmats = true;

    b.InitializeMaps(&icd_message_key_to_busnames_map,
                     std::set<uint64_t>({1, 2, 5}),
                     mask,
                     vote_threshold,
                     vote_method_checks_tmats,
                     bus_name_exclusions,
                     tmats_chanid_to_source_map);

    std::vector<uint64_t> transmit_cmds = std::vector<uint64_t>({0});
    std::vector<uint64_t> recieve_cmds = std::vector<uint64_t>({10});
    std::vector<uint64_t> channel_ids = std::vector<uint64_t>({1});

    EXPECT_TRUE(b.SubmitMessages(transmit_cmds, recieve_cmds, channel_ids));

    std::vector<std::string> adj_vec = {"2", "invalid", "2", "invalid", "BUS3", "q"};
    std::map<uint64_t, std::string> res;
    bool continue_translation = b.Finalize(res, 0, true, &adj_vec);
    EXPECT_FALSE(continue_translation);
    EXPECT_TRUE(iterable_tools_.GetKeys(res).size() == 0);

    std::map<uint64_t, std::pair<std::string, std::string>> final_bus_map =
        b.GetFinalBusMap_withSource();

    ASSERT_TRUE(iterable_tools_.GetKeys(final_bus_map).size() == 2);
    EXPECT_EQ(final_bus_map[1].first, "BUS1");
    EXPECT_EQ(final_bus_map[1].second, "Vote Method & TMATS");

    EXPECT_EQ(final_bus_map[2].first, "BUS3");  // Previous mapping was chid 2 -> BUS2
    EXPECT_EQ(final_bus_map[2].second, "USER");
    EXPECT_FALSE(continue_translation);
}

TEST_F(BusMapTest, UserAdjustmentsOverrideExistingWithNewNameAndSource)
{
    icd_message_key_to_busnames_map[10 & mask] = std::set<std::string>({"BUS1"});
    icd_message_key_to_busnames_map[11 & mask] = std::set<std::string>({"BUS2"});
    icd_message_key_to_busnames_map[12 & mask] = std::set<std::string>({"BUS3"});

    b.InitializeMaps(&icd_message_key_to_busnames_map,
                     std::set<uint64_t>({1, 2, 5}),
                     mask,
                     vote_threshold,
                     vote_method_checks_tmats,
                     bus_name_exclusions,
                     tmats_chanid_to_source_map);

    std::vector<uint64_t> transmit_cmds = std::vector<uint64_t>({0});
    std::vector<uint64_t> recieve_cmds = std::vector<uint64_t>({10});
    std::vector<uint64_t> channel_ids = std::vector<uint64_t>({1});

    EXPECT_TRUE(b.SubmitMessages(transmit_cmds, recieve_cmds, channel_ids));

    std::vector<std::string> adj_vec = {"2", "1", "BUS2", "q"};
    std::map<uint64_t, std::string> res;
    bool continue_translation = b.Finalize(res, 0, true, &adj_vec);
    EXPECT_TRUE(iterable_tools_.GetKeys(res).size() == 0);

    std::map<uint64_t, std::pair<std::string, std::string>> final_bus_map =
        b.GetFinalBusMap_withSource();

    ASSERT_TRUE(iterable_tools_.GetKeys(final_bus_map).size() == 1);
    EXPECT_EQ(final_bus_map[1].first, "BUS2");
    EXPECT_EQ(final_bus_map[1].second, "USER");
    EXPECT_FALSE(continue_translation);
}

TEST_F(BusMapTest, UserAdjustmentsAddFromNonMappedToFinalMap)
{
    icd_message_key_to_busnames_map[10 & mask] = std::set<std::string>({"BUS1"});
    icd_message_key_to_busnames_map[11 & mask] = std::set<std::string>({"BUS2"});
    icd_message_key_to_busnames_map[12 & mask] = std::set<std::string>({"BUS3"});

    b.InitializeMaps(&icd_message_key_to_busnames_map,
                     std::set<uint64_t>({1, 2, 3}),
                     mask,
                     vote_threshold,
                     vote_method_checks_tmats,
                     bus_name_exclusions,
                     tmats_chanid_to_source_map);

    std::vector<uint64_t> transmit_cmds = std::vector<uint64_t>({0, 0});
    std::vector<uint64_t> recieve_cmds = std::vector<uint64_t>({10, 11});
    std::vector<uint64_t> channel_ids = std::vector<uint64_t>({1, 2});

    EXPECT_TRUE(b.SubmitMessages(transmit_cmds, recieve_cmds, channel_ids));

    std::vector<std::string> adj_vec = {"2", "3", "BUS3", "q"};
    std::map<uint64_t, std::string> res;
    bool continue_translation = b.Finalize(res, 0, true, &adj_vec);
    EXPECT_TRUE(iterable_tools_.GetKeys(res).size() == 0);

    std::map<uint64_t, std::pair<std::string, std::string>> final_bus_map =
        b.GetFinalBusMap_withSource();
    ASSERT_TRUE(iterable_tools_.GetKeys(final_bus_map).size() == 3);
    EXPECT_FALSE(continue_translation);
    EXPECT_EQ(final_bus_map[1].first, "BUS1");
    EXPECT_EQ(final_bus_map[1].second, "Vote Method");
    EXPECT_EQ(final_bus_map[2].first, "BUS2");
    EXPECT_EQ(final_bus_map[2].second, "Vote Method");
    EXPECT_EQ(final_bus_map[3].first, "BUS3");
    EXPECT_EQ(final_bus_map[3].second, "USER");
    EXPECT_FALSE(continue_translation);
}

TEST_F(BusMapTest, UserAdjustmentsMapMultipleAndContinueAdjustsFinalMap)
{
    icd_message_key_to_busnames_map[10 & mask] = std::set<std::string>({"BUS1"});
    icd_message_key_to_busnames_map[11 & mask] = std::set<std::string>({"BUS2"});
    icd_message_key_to_busnames_map[12 & mask] = std::set<std::string>({"BUS3"});

    b.InitializeMaps(&icd_message_key_to_busnames_map,
                     std::set<uint64_t>({1, 2, 3, 4}),
                     mask,
                     vote_threshold,
                     vote_method_checks_tmats,
                     bus_name_exclusions,
                     tmats_chanid_to_source_map);

    std::vector<uint64_t> transmit_cmds = std::vector<uint64_t>({0, 0});
    std::vector<uint64_t> recieve_cmds = std::vector<uint64_t>({10, 11});
    std::vector<uint64_t> channel_ids = std::vector<uint64_t>({1, 2});

    EXPECT_TRUE(b.SubmitMessages(transmit_cmds, recieve_cmds, channel_ids));

    std::vector<std::string> adj_vec = {"2", "invalid_chid", "3", "invalid_bus",
                                        "BUS3", "2", "invalid_chid", "4",
                                        "invalid_bus", "BUS1", "1"};
    std::map<uint64_t, std::string> res;
    bool continue_translation = b.Finalize(res, 0, true, &adj_vec);

    std::map<uint64_t, std::pair<std::string, std::string>> final_bus_map =
        b.GetFinalBusMap_withSource();
    EXPECT_TRUE(iterable_tools_.GetKeys(final_bus_map).size() == 4);
    EXPECT_EQ(final_bus_map[1].first, "BUS1");
    EXPECT_EQ(final_bus_map[1].second, "Vote Method");
    EXPECT_EQ(final_bus_map[2].first, "BUS2");
    EXPECT_EQ(final_bus_map[2].second, "Vote Method");
    EXPECT_EQ(final_bus_map[3].first, "BUS3");
    EXPECT_EQ(final_bus_map[3].second, "USER");
    EXPECT_EQ(final_bus_map[4].first, "BUS1");
    EXPECT_EQ(final_bus_map[4].second, "USER");
    ASSERT_TRUE(iterable_tools_.GetKeys(res).size() == 4);
    EXPECT_EQ(res[1], "BUS1");
    EXPECT_EQ(res[2], "BUS2");
    EXPECT_EQ(res[3], "BUS3");
    EXPECT_EQ(res[4], "BUS1");
    EXPECT_TRUE(continue_translation);
}

TEST_F(BusMapTest, UserAdjustmentsMapMultipleAndContinueAdjustsFinalMapTMATS)
{
    icd_message_key_to_busnames_map[10 & mask] = std::set<std::string>({"BUS1"});
    icd_message_key_to_busnames_map[11 & mask] = std::set<std::string>({"BUS2"});
    icd_message_key_to_busnames_map[12 & mask] = std::set<std::string>({"BUS3"});

    tmats_chanid_to_source_map[1] = "BUSA";
    tmats_chanid_to_source_map[2] = "BUS2";
    tmats_chanid_to_source_map[3] = "BUSC";

    b.InitializeMaps(&icd_message_key_to_busnames_map,
                     std::set<uint64_t>({1, 2, 3, 4}),
                     mask,
                     vote_threshold,
                     vote_method_checks_tmats,
                     bus_name_exclusions,
                     tmats_chanid_to_source_map);

    std::vector<std::string> adj_vec = {"2", "invalid_chid", "3", "invalid_bus",
                                        "BUS3", "2", "invalid_chid", "1",
                                        "invalid_bus", "BUS1", "1"};
    std::map<uint64_t, std::string> res;
    bool continue_translation = b.Finalize(res, true, true, &adj_vec);

    std::map<uint64_t, std::pair<std::string, std::string>> final_bus_map =
        b.GetFinalBusMap_withSource();
    EXPECT_TRUE(iterable_tools_.GetKeys(final_bus_map).size() == 3);
    EXPECT_EQ(final_bus_map[1].first, "BUS1");
    EXPECT_EQ(final_bus_map[1].second, "USER");
    EXPECT_EQ(final_bus_map[2].first, "BUS2");
    EXPECT_EQ(final_bus_map[2].second, "TMATS");
    EXPECT_EQ(final_bus_map[3].first, "BUS3");
    EXPECT_EQ(final_bus_map[3].second, "USER");
    ASSERT_TRUE(iterable_tools_.GetKeys(res).size() == 3);
    EXPECT_EQ(res[1], "BUS1");
    EXPECT_EQ(res[2], "BUS2");
    EXPECT_EQ(res[3], "BUS3");
    EXPECT_TRUE(continue_translation);
}

TEST_F(BusMapTest, UserAdjustmentsEnsureNoMapInputDoesnotAllowChangesToFinalMap)
{
    b.InitializeMaps(&icd_message_key_to_busnames_map,
                     std::set<uint64_t>(),
                     mask,
                     vote_threshold,
                     vote_method_checks_tmats,
                     bus_name_exclusions,
                     tmats_chanid_to_source_map);

    std::vector<std::string> adj_vec = {"2", "invalid", "q"};
    std::map<uint64_t, std::string> res;
    bool continue_translation = b.Finalize(res, 0, true, &adj_vec);
    EXPECT_TRUE(iterable_tools_.GetKeys(res).size() == 0);

    std::map<uint64_t, std::pair<std::string, std::string>> final_bus_map =
        b.GetFinalBusMap_withSource();

    EXPECT_TRUE(iterable_tools_.GetKeys(final_bus_map).size() == 0);
    EXPECT_FALSE(continue_translation);
}

TEST_F(BusMapTest, InitializeMapsInitialMapsAssigned)
{
    // Empty Map
    b.InitializeMaps(&icd_message_key_to_busnames_map,
                     std::set<uint64_t>(),
                     mask,
                     vote_threshold,
                     vote_method_checks_tmats,
                     bus_name_exclusions,
                     tmats_chanid_to_source_map);

    EXPECT_EQ(b.GetICD_MessageKeyToBusNamesMap().size(), 0);

    // Map with entries
    icd_message_key_to_busnames_map[1] = std::set<std::string>({"BusA", "BusB", "BusC"});
    icd_message_key_to_busnames_map[2] = std::set<std::string>({"BusA"});

    b.InitializeMaps(&icd_message_key_to_busnames_map,
                     std::set<uint64_t>({5, 6, 7, 8}),
                     mask,
                     vote_threshold,
                     vote_method_checks_tmats,
                     bus_name_exclusions,
                     tmats_chanid_to_source_map);
    EXPECT_EQ(b.GetICD_MessageKeyToBusNamesMap(), icd_message_key_to_busnames_map);
    EXPECT_THAT(b.GetChannelIDs(), ::testing::ElementsAre(5, 6, 7, 8));
}

TEST_F(BusMapTest, InitializeMapsWithMask)
{
    // Map with entries
    icd_message_key_to_busnames_map[10] = std::set<std::string>({"BusA", "BusB", "BusC"});
    icd_message_key_to_busnames_map[11] = std::set<std::string>({"BusD"});
    // 10 and 11 keys should be masked to be the same key (10)

    uint64_t mask_input = 0b11111111110;
    b.InitializeMaps(&icd_message_key_to_busnames_map,
                     std::set<uint64_t>({5, 6, 7, 8}),
                     mask_input,
                     vote_threshold,
                     vote_method_checks_tmats,
                     bus_name_exclusions,
                     tmats_chanid_to_source_map);

    std::unordered_map<uint64_t, std::set<std::string>> compare_map;

    compare_map[10] = std::set<std::string>({"BusA", "BusB", "BusC", "BusD"});

    EXPECT_EQ(b.GetICD_MessageKeyToBusNamesMap(), compare_map);
}

TEST_F(BusMapTest, InitializeMapsMessageKeyToChannelIDCreationAndUniqueBuses)
{
    // Map with entries
    icd_message_key_to_busnames_map[1 & mask] = std::set<std::string>({"BusA", "BusB", "BusC"});
    icd_message_key_to_busnames_map[2 & mask] = std::set<std::string>({"BusA"});
    icd_message_key_to_busnames_map[3 & mask] = std::set<std::string>({"BusD", "BusB"});

    b.InitializeMaps(&icd_message_key_to_busnames_map,
                     std::set<uint64_t>(),
                     mask,
                     vote_threshold,
                     vote_method_checks_tmats,
                     bus_name_exclusions,
                     tmats_chanid_to_source_map);

    icd_message_key_to_channelids_map[1] = std::set<uint64_t>();
    icd_message_key_to_channelids_map[2] = std::set<uint64_t>();
    icd_message_key_to_channelids_map[3] = std::set<uint64_t>();

    EXPECT_EQ(b.GetICD_MessageKeyToChannelIDSMap(), icd_message_key_to_channelids_map);
    EXPECT_THAT(b.GetUniqueBuses(), ::testing::ElementsAre("BusA", "BusB", "BusC", "BusD"));
}

// If tmats source map is empty, bus map is considered to be given no tmats data
TEST_F(BusMapTest, InitializeMapsTMATSCheck)
{
    icd_message_key_to_busnames_map[10] = std::set<std::string>({"BUS1", "BUS2", "BUS3", "BUS4"});

    // No tmats provided
    b.InitializeMaps(&icd_message_key_to_busnames_map,
                     std::set<uint64_t>());
    EXPECT_FALSE(b.TmatsPresent());

    // empty tmats provided
    b.InitializeMaps(&icd_message_key_to_busnames_map,
                     std::set<uint64_t>({1, 2, 3, 4}),
                     mask,
                     vote_threshold,
                     vote_method_checks_tmats,
                     bus_name_exclusions,
                     tmats_chanid_to_source_map);
    EXPECT_FALSE(b.TmatsPresent());

    tmats_chanid_to_source_map[1] = "BUS1";
    tmats_chanid_to_source_map[2] = "BUS2";
    tmats_chanid_to_source_map[3] = "BUS3";
    tmats_chanid_to_source_map[4] = "BUS4";

    // tmats provided
    b.InitializeMaps(&icd_message_key_to_busnames_map,
                     std::set<uint64_t>({1, 2, 3, 4}),
                     mask,
                     vote_threshold,
                     vote_method_checks_tmats,
                     bus_name_exclusions,
                     tmats_chanid_to_source_map);
    EXPECT_TRUE(b.TmatsPresent());

    EXPECT_EQ(b.GetTMATSchannelidToSourceMap(), tmats_chanid_to_source_map);
}

TEST_F(BusMapTest, InitializeMapsTMATSReplacements)
{
    // No tmats provided
    b.InitializeMaps(&icd_message_key_to_busnames_map,
                     std::set<uint64_t>());
    EXPECT_FALSE(b.TmatsPresent());

    // empty tmats provided
    b.InitializeMaps(&icd_message_key_to_busnames_map,
                     std::set<uint64_t>(),
                     mask,
                     vote_threshold,
                     vote_method_checks_tmats,
                     bus_name_exclusions,
                     tmats_chanid_to_source_map);
    EXPECT_FALSE(b.TmatsPresent());

    tmats_chanid_to_source_map[1] = "BUS1";
    tmats_chanid_to_source_map[2] = "BUS2";
    tmats_chanid_to_source_map[3] = "BUS3";
    tmats_chanid_to_source_map[4] = "BUS4";

    std::map<std::string, std::string> tmats_replacements;

    tmats_replacements["BUS3"] = "BUS3Replacement";
    tmats_replacements["BUS1"] = "BUS1Replacement";
    tmats_replacements["BUSB"] = "NonExistantReplacement";

    std::map<uint64_t, std::string> compare_map;
    compare_map[1] = "BUS1Replacement";
    compare_map[2] = "BUS2";
    compare_map[3] = "BUS3Replacement";
    compare_map[4] = "BUS4";

    // tmats provided with tmats replacements
    b.InitializeMaps(&icd_message_key_to_busnames_map,
                     std::set<uint64_t>({1, 2, 3, 4}),
                     mask,
                     vote_threshold,
                     vote_method_checks_tmats,
                     bus_name_exclusions,
                     tmats_chanid_to_source_map,
                     tmats_replacements);

    EXPECT_TRUE(b.TmatsPresent());
    EXPECT_EQ(b.GetTMATSchannelidToSourceMap(), compare_map);
}

TEST_F(BusMapTest, SubmitMessages)
{
    // Map with entries
    // keys are transmit command word bit shifted left 16 bits
    // followed by the recieve command word
    icd_message_key_to_busnames_map[10 << 16 | 10] = std::set<std::string>();
    icd_message_key_to_busnames_map[11 << 16 | 11] = std::set<std::string>();
    icd_message_key_to_busnames_map[12 << 16 | 12] = std::set<std::string>();
    icd_message_key_to_busnames_map[13 << 16 | 13] = std::set<std::string>();
    icd_message_key_to_busnames_map[14 << 16 | 14] = std::set<std::string>();
    icd_message_key_to_busnames_map[15 << 16 | 15] = std::set<std::string>();
    icd_message_key_to_busnames_map[16 << 16 | 16] = std::set<std::string>();
    icd_message_key_to_busnames_map[17 << 16 | 17] = std::set<std::string>();
    icd_message_key_to_busnames_map[18 << 16 | 18] = std::set<std::string>();
    icd_message_key_to_busnames_map[19 << 16 | 19] = std::set<std::string>();
    icd_message_key_to_busnames_map[20 << 16 | 20] = std::set<std::string>();

    b.InitializeMaps(&icd_message_key_to_busnames_map,
                     std::set<uint64_t>(),
                     mask);

    std::vector<uint64_t> transmit_cmds = std::vector<uint64_t>({10, 10, 11, 12, 13, 14, 15});
    std::vector<uint64_t> recieve_cmds = std::vector<uint64_t>({10, 10, 11, 12, 13, 20, 15});
    std::vector<uint64_t> channel_ids = std::vector<uint64_t>({0, 1, 1, 2, 3, 4, 5});

    EXPECT_TRUE(b.SubmitMessages(transmit_cmds, recieve_cmds, channel_ids));

    // last two elements should not be included in submission
    // because a value of 6 was passed into as the submission size
    transmit_cmds = std::vector<uint64_t>({16, 17, 18, 19, 20, 20, 100, 200});
    recieve_cmds = std::vector<uint64_t>({16, 17, 10, 19, 20, 20, 100, 200});
    channel_ids = std::vector<uint64_t>({5, 7, 8, 9, 10, 10, 100, 200});

    // include submission size
    EXPECT_TRUE(b.SubmitMessages(transmit_cmds, recieve_cmds, channel_ids, 6));

    std::unordered_map<uint64_t, std::set<uint64_t>> compare_map;
    compare_map[10 << 16 | 10] = std::set<uint64_t>({0, 1});  // multiple matches with different channel IDs
    compare_map[11 << 16 | 11] = std::set<uint64_t>({1});
    compare_map[12 << 16 | 12] = std::set<uint64_t>({2});
    compare_map[13 << 16 | 13] = std::set<uint64_t>({3});
    compare_map[14 << 16 | 14] = std::set<uint64_t>();  // missing from input
    compare_map[15 << 16 | 15] = std::set<uint64_t>({5});
    compare_map[16 << 16 | 16] = std::set<uint64_t>({5});  // duplicate channel ids with different keys
    compare_map[17 << 16 | 17] = std::set<uint64_t>({7});
    compare_map[18 << 16 | 18] = std::set<uint64_t>();  // missing from input
    compare_map[19 << 16 | 19] = std::set<uint64_t>({9});
    compare_map[20 << 16 | 20] = std::set<uint64_t>({10});  // duplicate channel ids with same keys

    EXPECT_EQ(b.GetICD_MessageKeyToChannelIDSMap(), compare_map);
}

TEST_F(BusMapTest, SubmitMessage)
{
    // Map with entries
    // keys are transmit command word bit shifted left 16 bits
    // followed by the recieve command word
    icd_message_key_to_busnames_map[10 << 16 | 10] = std::set<std::string>();
    icd_message_key_to_busnames_map[11 << 16 | 11] = std::set<std::string>();
    icd_message_key_to_busnames_map[12 << 16 | 12] = std::set<std::string>();
    icd_message_key_to_busnames_map[13 << 16 | 13] = std::set<std::string>();
    icd_message_key_to_busnames_map[14 << 16 | 14] = std::set<std::string>();
    icd_message_key_to_busnames_map[15 << 16 | 15] = std::set<std::string>();

    b.InitializeMaps(&icd_message_key_to_busnames_map,
                     std::set<uint64_t>(),
                     mask);

    std::vector<uint64_t> transmit_cmds = std::vector<uint64_t>({10, 10, 11, 12, 13, 14, 15});
    std::vector<uint64_t> recieve_cmds = std::vector<uint64_t>({10, 10, 11, 12, 13, 20, 15});
    std::vector<uint64_t> channel_ids = std::vector<uint64_t>({0, 1, 1, 2, 3, 4, 5});

    b.SubmitMessage(10, 10, 0);
    b.SubmitMessage(10, 10, 1);
    b.SubmitMessage(11, 11, 1);
    b.SubmitMessage(12, 12, 2);
    b.SubmitMessage(13, 13, 3);
    b.SubmitMessage(14, 20, 4);
    b.SubmitMessage(15, 15, 5);

    std::unordered_map<uint64_t, std::set<uint64_t>> compare_map;
    compare_map[10 << 16 | 10] = std::set<uint64_t>({0, 1});  // multiple matches with different channel IDs
    compare_map[11 << 16 | 11] = std::set<uint64_t>({1});
    compare_map[12 << 16 | 12] = std::set<uint64_t>({2});
    compare_map[13 << 16 | 13] = std::set<uint64_t>({3});
    compare_map[14 << 16 | 14] = std::set<uint64_t>();  // missing from input
    compare_map[15 << 16 | 15] = std::set<uint64_t>({5});

    EXPECT_EQ(b.GetICD_MessageKeyToChannelIDSMap(), compare_map);
}

TEST_F(BusMapTest, SubmitMessagesWithMask)
{
    // Map with entries
    icd_message_key_to_busnames_map[10 << 16] = std::set<std::string>({"BusA", "BusB", "BusC"});
    icd_message_key_to_busnames_map[(10 << 16) + 1] = std::set<std::string>({"BusD"});

    uint64_t mask_input = UINT64_MAX - 1;
    b.InitializeMaps(&icd_message_key_to_busnames_map,
                     std::set<uint64_t>({0, 1}),
                     mask_input,
                     vote_threshold,
                     vote_method_checks_tmats,
                     bus_name_exclusions,
                     tmats_chanid_to_source_map);

    // Both entries should map with the same key given the mask =  UINT64_MAX - 1
    std::vector<uint64_t> transmit_cmds = std::vector<uint64_t>({10, 10});
    std::vector<uint64_t> recieve_cmds = std::vector<uint64_t>({0, 1});
    std::vector<uint64_t> channel_ids = std::vector<uint64_t>({0, 1});

    EXPECT_TRUE(b.SubmitMessages(transmit_cmds, recieve_cmds, channel_ids));

    std::unordered_map<uint64_t, std::set<uint64_t>> compare_map;
    compare_map[10 << 16] = std::set<uint64_t>({0, 1});

    EXPECT_EQ(b.GetICD_MessageKeyToChannelIDSMap(), compare_map);
}

TEST_F(BusMapTest, SubmitMessagesNonEqualSizedVectors)
{
    b.InitializeMaps(&icd_message_key_to_busnames_map,
                     std::set<uint64_t>(),
                     mask);

    std::vector<uint64_t> transmit_cmds = std::vector<uint64_t>({10, 10});
    std::vector<uint64_t> recieve_cmds = std::vector<uint64_t>({10, 10, 11});
    std::vector<uint64_t> channel_ids = std::vector<uint64_t>({0, 1, 1, 2});

    EXPECT_FALSE(b.SubmitMessages(transmit_cmds, recieve_cmds, channel_ids));
}

TEST_F(BusMapTest, VoteMappingNoMatches)
{
    icd_message_key_to_busnames_map[10] = std::set<std::string>({"BusA", "BusB"});
    icd_message_key_to_busnames_map[11 & mask] = std::set<std::string>({"BusB", "BusC"});
    icd_message_key_to_busnames_map[12 & mask] = std::set<std::string>({"BusC", "BusD"});

    b.InitializeMaps(&icd_message_key_to_busnames_map,
                     std::set<uint64_t>(),
                     mask);

    icd_message_key_to_channelids_map[10] = std::set<uint64_t>();
    icd_message_key_to_channelids_map[11] = std::set<uint64_t>();
    icd_message_key_to_channelids_map[12] = std::set<uint64_t>();

    std::map<uint64_t, std::string> compare_map;

    EXPECT_EQ(b.TestVoteMapping(icd_message_key_to_channelids_map), compare_map);
}

TEST_F(BusMapTest, VoteMappingMatchHighestVotes)
{
    icd_message_key_to_busnames_map[10] = std::set<std::string>({"BusA", "BusB"});
    icd_message_key_to_busnames_map[11] = std::set<std::string>({"BusB", "BusC"});
    icd_message_key_to_busnames_map[12] = std::set<std::string>({"BusC", "BusD"});
    icd_message_key_to_busnames_map[13] = std::set<std::string>({"BusD", "BusE"});
    icd_message_key_to_busnames_map[14] = std::set<std::string>({"BusD", "BusE"});
    icd_message_key_to_busnames_map[15] = std::set<std::string>({"BusE"});
    icd_message_key_to_busnames_map[16] = std::set<std::string>({"BusG"});

    b.InitializeMaps(&icd_message_key_to_busnames_map,
                     std::set<uint64_t>({1, 2, 4}),
                     mask);

    icd_message_key_to_channelids_map[10] = std::set<uint64_t>({1});
    icd_message_key_to_channelids_map[11] = std::set<uint64_t>({1});
    icd_message_key_to_channelids_map[12] = std::set<uint64_t>({2});
    icd_message_key_to_channelids_map[13] = std::set<uint64_t>({2, 4});
    icd_message_key_to_channelids_map[14] = std::set<uint64_t>({4});
    icd_message_key_to_channelids_map[15] = std::set<uint64_t>({4});
    icd_message_key_to_channelids_map[16] = std::set<uint64_t>({});

    std::map<uint64_t, std::string> compare_map;
    compare_map[1] = "BusB";
    compare_map[2] = "BusD";
    compare_map[4] = "BusE";

    EXPECT_EQ(b.TestVoteMapping(icd_message_key_to_channelids_map), compare_map);
}

TEST_F(BusMapTest, VoteMappingNoMatchesWhenVoteCountIsTheSame)
{
    icd_message_key_to_busnames_map[10] = std::set<std::string>({"BusA", "BusB"});
    // Bus A and B are a tie
    icd_message_key_to_busnames_map[11] = std::set<std::string>({"BusA", "BusB", "BusC"});
    // Channel ID 2 will tie Bus C
    icd_message_key_to_busnames_map[12] = std::set<std::string>({"BusC"});
    // Channel ID 3 should break the tie with BusC
    icd_message_key_to_busnames_map[13] = std::set<std::string>({"BusC"});

    b.InitializeMaps(&icd_message_key_to_busnames_map,
                     std::set<uint64_t>({1, 2, 4}));

    icd_message_key_to_channelids_map[10] = std::set<uint64_t>({1, 2, 3});
    icd_message_key_to_channelids_map[11] = std::set<uint64_t>({1, 2, 3});
    icd_message_key_to_channelids_map[12] = std::set<uint64_t>({2, 3});
    icd_message_key_to_channelids_map[13] = std::set<uint64_t>({3});

    std::map<uint64_t, std::string> compare_map;
    compare_map[3] = "BusC";

    EXPECT_EQ(b.TestVoteMapping(icd_message_key_to_channelids_map), compare_map);
}

TEST_F(BusMapTest, MaskTest)
{
    icd_message_key_to_busnames_map[10] = std::set<std::string>({"BUSA"});
    icd_message_key_to_busnames_map[11] = std::set<std::string>({"BUSB"});

    uint64_t mask_input = 0b1111111111110;

    b.InitializeMaps(&icd_message_key_to_busnames_map,
                     std::set<uint64_t>({1, 2, 4}),
                     mask_input);

    std::vector<uint64_t> transmit_cmds = std::vector<uint64_t>({0, 0});
    std::vector<uint64_t> recieve_cmds = std::vector<uint64_t>({10, 11});
    //the mask should make the 10 and 11 recieve cmds the same
    std::vector<uint64_t> channel_ids = std::vector<uint64_t>({1, 2});

    EXPECT_TRUE(b.SubmitMessages(transmit_cmds, recieve_cmds, channel_ids));

    icd_message_key_to_channelids_map[10] = std::set<uint64_t>({1, 2});

    EXPECT_EQ(b.GetICD_MessageKeyToChannelIDSMap(), icd_message_key_to_channelids_map);
}

TEST_F(BusMapTest, FinalizeTMATSMoreChannelIDsThanNecessaryAndOverridesVoteMap)
{
    icd_message_key_to_busnames_map[10] = std::set<std::string>({"BusA"});
    icd_message_key_to_busnames_map[11] = std::set<std::string>({"BusB"});
    icd_message_key_to_busnames_map[12] = std::set<std::string>({"BusC"});
    icd_message_key_to_busnames_map[13] = std::set<std::string>({"BusD"});
    icd_message_key_to_busnames_map[14] = std::set<std::string>({"BusE"});
    icd_message_key_to_busnames_map[15] = std::set<std::string>({"BusF"});

    tmats_chanid_to_source_map[1] = "BusF";
    tmats_chanid_to_source_map[2] = "BusE";
    tmats_chanid_to_source_map[3] = "BusD";
    tmats_chanid_to_source_map[4] = "BusC";

    b.InitializeMaps(&icd_message_key_to_busnames_map,
                     std::set<uint64_t>({1, 2, 3}),
                     mask,
                     vote_threshold,
                     vote_method_checks_tmats,
                     bus_name_exclusions,
                     tmats_chanid_to_source_map);

    ASSERT_TRUE(b.TmatsPresent());

    std::vector<uint64_t> transmit_cmds = std::vector<uint64_t>({0, 0, 0, 0, 0, 0});
    std::vector<uint64_t> recieve_cmds = std::vector<uint64_t>({10, 11, 12, 13, 14, 15});
    std::vector<uint64_t> channel_ids = std::vector<uint64_t>({0, 1, 2, 3, 4, 5});

    EXPECT_TRUE(b.SubmitMessages(transmit_cmds, recieve_cmds, channel_ids));

    b.InitializeMaps(&icd_message_key_to_busnames_map,
                     std::set<uint64_t>({1, 2, 3}),
                     mask,
                     vote_threshold,
                     vote_method_checks_tmats,
                     bus_name_exclusions,
                     tmats_chanid_to_source_map);

    ASSERT_TRUE(b.TmatsPresent());

    std::map<uint64_t, std::string> res;
    bool continue_translation = b.Finalize(res, 1, false);
    EXPECT_TRUE(iterable_tools_.GetKeys(res).size() == 3);
    EXPECT_TRUE(continue_translation);
    EXPECT_EQ(res[1], "BusF");
    EXPECT_EQ(res[2], "BusE");
    EXPECT_EQ(res[3], "BusD");

    // Also check the source
    std::map<uint64_t, std::pair<std::string, std::string>> final_bus_map_with_source =
        b.GetFinalBusMap_withSource();

    EXPECT_TRUE(iterable_tools_.GetKeys(final_bus_map_with_source).size() == 3);
    EXPECT_EQ(final_bus_map_with_source[1].first, "BusF");
    EXPECT_EQ(final_bus_map_with_source[1].second, "TMATS");

    EXPECT_EQ(final_bus_map_with_source[2].first, "BusE");
    EXPECT_EQ(final_bus_map_with_source[2].second, "TMATS");

    EXPECT_EQ(final_bus_map_with_source[3].first, "BusD");
    EXPECT_EQ(final_bus_map_with_source[3].second, "TMATS");
}

TEST_F(BusMapTest, FinalizeVoteMappingWithTMATSCheckSetToFalse)
{
    icd_message_key_to_busnames_map[10] = std::set<std::string>({"BUSA"});
    icd_message_key_to_busnames_map[11] = std::set<std::string>({"BUSB"});
    icd_message_key_to_busnames_map[12] = std::set<std::string>({"BUSC"});

    tmats_chanid_to_source_map[0] = "BUS1";
    tmats_chanid_to_source_map[1] = "BUS2";
    tmats_chanid_to_source_map[2] = "BUS3";

    /*
		Should not check tmats
	*/
    vote_method_checks_tmats = false;
    b.InitializeMaps(&icd_message_key_to_busnames_map,
                     std::set<uint64_t>({0, 1, 2, 3}),
                     mask,
                     vote_threshold,
                     vote_method_checks_tmats,
                     bus_name_exclusions,
                     tmats_chanid_to_source_map);

    ASSERT_TRUE(b.TmatsPresent());

    std::vector<uint64_t> transmit_cmds = std::vector<uint64_t>({0, 0, 0, 0, 0, 0});
    std::vector<uint64_t> recieve_cmds = std::vector<uint64_t>({10, 11, 12, 13, 14, 15});
    std::vector<uint64_t> channel_ids = std::vector<uint64_t>({0, 1, 2, 3, 4, 5});

    EXPECT_TRUE(b.SubmitMessages(transmit_cmds, recieve_cmds, channel_ids));

    std::map<uint64_t, std::string> res;
    bool continue_translation = b.Finalize(res, 0, false);
    EXPECT_TRUE(iterable_tools_.GetKeys(res).size() == 3);
    EXPECT_TRUE(continue_translation);
    EXPECT_EQ(res[0], "BUSA");
    EXPECT_EQ(res[1], "BUSB");
    EXPECT_EQ(res[2], "BUSC");

    // Also check the source
    std::map<uint64_t, std::pair<std::string, std::string>> final_bus_map_with_source =
        b.GetFinalBusMap_withSource();

    EXPECT_TRUE(iterable_tools_.GetKeys(final_bus_map_with_source).size() == 3);

    EXPECT_EQ(final_bus_map_with_source[0].first, "BUSA");
    EXPECT_EQ(final_bus_map_with_source[0].second, "Vote Method");

    EXPECT_EQ(final_bus_map_with_source[1].first, "BUSB");
    EXPECT_EQ(final_bus_map_with_source[1].second, "Vote Method");

    EXPECT_EQ(final_bus_map_with_source[2].first, "BUSC");
    EXPECT_EQ(final_bus_map_with_source[2].second, "Vote Method");
}

TEST_F(BusMapTest, FinalizeVoteMappingWithTMATSCheckSetToTrue)
{
    icd_message_key_to_busnames_map[10] = std::set<std::string>({"BUSA"});
    icd_message_key_to_busnames_map[11] = std::set<std::string>({"BUSB"});
    icd_message_key_to_busnames_map[12] = std::set<std::string>({"BUSC"});
    icd_message_key_to_busnames_map[13] = std::set<std::string>({"BUSD"});
    icd_message_key_to_busnames_map[14] = std::set<std::string>({"BUSE"});

    tmats_chanid_to_source_map[0] = "BUS1";         // BUSA should not match TMATS
    tmats_chanid_to_source_map[1] = "BUSB";         // BUSB should match TMATS
    tmats_chanid_to_source_map[2] = "THISBUSCMUX";  // Should match BUSC as a Substring
    // channel ID 3 should not map because it is missing from TMATS
    // channel ID 4 is a vote tie
    tmats_chanid_to_source_map[5] = "BUSA";  // Channel ID 5 has not votes and should not map
    tmats_chanid_to_source_map[6] = "BUSZ";  // Should match BUSA after tmats replacement
    tmats_chanid_to_source_map[7] = "BUS";   // Should match BUSE vote by Substring

    vote_method_checks_tmats = true;

    std::map<std::string, std::string> tmats_corrections;
    tmats_corrections["BUSZ"] = "BUSAMUX";
    b.InitializeMaps(&icd_message_key_to_busnames_map,
                     std::set<uint64_t>({0, 1, 2, 3, 4, 5, 6, 7}),
                     mask,
                     vote_threshold,
                     vote_method_checks_tmats,
                     bus_name_exclusions,
                     tmats_chanid_to_source_map,
                     tmats_corrections);

    ASSERT_TRUE(b.TmatsPresent());

    std::vector<uint64_t> transmit_cmds = std::vector<uint64_t>({0, 0, 0, 0, 0, 0, 0, 0, 0});
    std::vector<uint64_t> recieve_cmds = std::vector<uint64_t>({10, 11, 12, 13, 14, 13, 15, 10, 14});
    std::vector<uint64_t> channel_ids = std::vector<uint64_t>({0, 1, 2, 3, 4, 4, 5, 6, 7});

    EXPECT_TRUE(b.SubmitMessages(transmit_cmds, recieve_cmds, channel_ids));

    std::map<uint64_t, std::string> res;
    bool continue_translation = b.Finalize(res, 0, false);
    EXPECT_TRUE(iterable_tools_.GetKeys(res).size() == 4);
    EXPECT_TRUE(continue_translation);
    EXPECT_EQ(res[1], "BUSB");
    EXPECT_EQ(res[2], "BUSC");
    EXPECT_EQ(res[6], "BUSA");
    EXPECT_EQ(res[7], "BUSE");

    // Also check the source
    std::map<uint64_t, std::pair<std::string, std::string>> final_bus_map_with_source =
        b.GetFinalBusMap_withSource();

    EXPECT_TRUE(iterable_tools_.GetKeys(final_bus_map_with_source).size() == 4);

    EXPECT_EQ(final_bus_map_with_source[1].first, "BUSB");
    EXPECT_EQ(final_bus_map_with_source[1].second, "Vote Method & TMATS");

    EXPECT_EQ(final_bus_map_with_source[2].first, "BUSC");
    EXPECT_EQ(final_bus_map_with_source[2].second, "Vote Method & TMATS");

    EXPECT_EQ(final_bus_map_with_source[6].first, "BUSA");
    EXPECT_EQ(final_bus_map_with_source[6].second, "Vote Method & TMATS");

    EXPECT_EQ(final_bus_map_with_source[7].first, "BUSE");
    EXPECT_EQ(final_bus_map_with_source[7].second, "Vote Method & TMATS");

    // Check the excluded channel IDs map
    std::map<uint64_t, std::string> excludedChannelIDs = b.GetExcludedChannelIDs();
    EXPECT_TRUE(iterable_tools_.GetKeys(excludedChannelIDs).size() == 4);
    EXPECT_EQ(excludedChannelIDs[0], "TMATS Mismatch");
    EXPECT_EQ(excludedChannelIDs[3], "Missing From TMATS");
    EXPECT_EQ(excludedChannelIDs[4], "Tie Vote");
    EXPECT_EQ(excludedChannelIDs[5], "No Votes");
}

TEST_F(BusMapTest, FinalizeTMATSMustBeExactBusMatchFromDTSFile)
{
    icd_message_key_to_busnames_map[10] = std::set<std::string>({"BusA"});
    icd_message_key_to_busnames_map[11] = std::set<std::string>({"BusB"});
    icd_message_key_to_busnames_map[12] = std::set<std::string>({"BusC"});
    icd_message_key_to_busnames_map[13] = std::set<std::string>({"BusD"});
    icd_message_key_to_busnames_map[14] = std::set<std::string>({"BusE"});
    icd_message_key_to_busnames_map[15] = std::set<std::string>({"BusF"});

    tmats_chanid_to_source_map[1] = "BusA";   // Should match to BusB after busmap replacement
    tmats_chanid_to_source_map[2] = "BUS";    // Should not match
    tmats_chanid_to_source_map[3] = "BusD";   // Should be an exact match
    tmats_chanid_to_source_map[4] = "BusD";   // Duplicate exact match
    tmats_chanid_to_source_map[10] = "BusE";  // Should not match because the channel ID 10
                                              // was not sent to InitializeMaps

    std::map<std::string, std::string> tmats_busname_replacement;
    tmats_busname_replacement["BusA"] = "BusB";

    b.InitializeMaps(&icd_message_key_to_busnames_map,
                     std::set<uint64_t>({1, 2, 3, 4, 5, 6}),
                     mask,
                     vote_threshold,
                     vote_method_checks_tmats,
                     bus_name_exclusions,
                     tmats_chanid_to_source_map,
                     tmats_busname_replacement);

    ASSERT_TRUE(b.TmatsPresent());

    std::map<uint64_t, std::string> res;
    bool continue_translation = b.Finalize(res, 1, false);
    EXPECT_TRUE(iterable_tools_.GetKeys(res).size() == 3);
    EXPECT_TRUE(continue_translation);
    EXPECT_EQ(res[1], "BusB");
    EXPECT_EQ(res[3], "BusD");
    EXPECT_EQ(res[4], "BusD");

    // Also check the source
    std::map<uint64_t, std::pair<std::string, std::string>> final_bus_map_with_source =
        b.GetFinalBusMap_withSource();

    EXPECT_TRUE(iterable_tools_.GetKeys(final_bus_map_with_source).size() == 3);
    EXPECT_EQ(final_bus_map_with_source[1].first, "BusB");
    EXPECT_EQ(final_bus_map_with_source[1].second, "TMATS");

    EXPECT_EQ(final_bus_map_with_source[3].first, "BusD");
    EXPECT_EQ(final_bus_map_with_source[3].second, "TMATS");

    EXPECT_EQ(final_bus_map_with_source[4].first, "BusD");
    EXPECT_EQ(final_bus_map_with_source[4].second, "TMATS");

    // Check the excluded channel IDs map
    std::map<uint64_t, std::string> excludedChannelIDs = b.GetExcludedChannelIDs();
    EXPECT_TRUE(iterable_tools_.GetKeys(excludedChannelIDs).size() == 3);
    EXPECT_EQ(excludedChannelIDs[2], "TMATS Bus Name Not in DTS File");
    EXPECT_EQ(excludedChannelIDs[5], "Missing From TMATS");
    EXPECT_EQ(excludedChannelIDs[6], "Missing From TMATS");
}

TEST_F(BusMapTest, FinalizeTMATSFewerChannelIDsThanNecessaryAndOverridesVoteMap)
{
    icd_message_key_to_busnames_map[10] = std::set<std::string>({"BusA"});
    icd_message_key_to_busnames_map[11] = std::set<std::string>({"BusB"});
    icd_message_key_to_busnames_map[12] = std::set<std::string>({"BusC"});
    icd_message_key_to_busnames_map[13] = std::set<std::string>({"BusD"});
    icd_message_key_to_busnames_map[14] = std::set<std::string>({"BusE"});
    icd_message_key_to_busnames_map[15] = std::set<std::string>({"BusF"});

    tmats_chanid_to_source_map[1] = "BusD";
    tmats_chanid_to_source_map[2] = "BusF";

    b.InitializeMaps(&icd_message_key_to_busnames_map,
                     std::set<uint64_t>({1, 2, 3}),
                     mask,
                     vote_threshold,
                     vote_method_checks_tmats,
                     bus_name_exclusions,
                     tmats_chanid_to_source_map);

    ASSERT_TRUE(b.TmatsPresent());

    std::vector<uint64_t> transmit_cmds = std::vector<uint64_t>({0, 0, 0, 0, 0, 0});
    std::vector<uint64_t> recieve_cmds = std::vector<uint64_t>({10, 11, 12, 13, 14, 15});
    std::vector<uint64_t> channel_ids = std::vector<uint64_t>({0, 1, 2, 3, 4, 5});

    EXPECT_TRUE(b.SubmitMessages(transmit_cmds, recieve_cmds, channel_ids));

    std::map<uint64_t, std::string> res;
    bool continue_translation = b.Finalize(res, 1, false);
    EXPECT_TRUE(iterable_tools_.GetKeys(res).size() == 2);
    EXPECT_TRUE(continue_translation);
    EXPECT_EQ(res[1], "BusD");
    EXPECT_EQ(res[2], "BusF");

    // Also check the source
    std::map<uint64_t, std::pair<std::string, std::string>> final_bus_map_with_source =
        b.GetFinalBusMap_withSource();

    EXPECT_TRUE(iterable_tools_.GetKeys(final_bus_map_with_source).size() == 2);
    EXPECT_EQ(final_bus_map_with_source[1].first, "BusD");
    EXPECT_EQ(final_bus_map_with_source[1].second, "TMATS");

    EXPECT_EQ(final_bus_map_with_source[2].first, "BusF");
    EXPECT_EQ(final_bus_map_with_source[2].second, "TMATS");
}

TEST_F(BusMapTest, FinalizeVoteMappingMoreChannelIDsThanNecessaryAndOverridesTMATSMap)
{
    icd_message_key_to_busnames_map[10] = std::set<std::string>({"BUSA"});
    icd_message_key_to_busnames_map[11] = std::set<std::string>({"BUSB"});
    icd_message_key_to_busnames_map[12] = std::set<std::string>({"BUSC"});
    icd_message_key_to_busnames_map[13] = std::set<std::string>({"BUSD"});
    icd_message_key_to_busnames_map[14] = std::set<std::string>({"BUSE"});
    icd_message_key_to_busnames_map[15] = std::set<std::string>({"BUSF"});

    tmats_chanid_to_source_map[1] = "BUS1";
    tmats_chanid_to_source_map[2] = "BUS2";
    tmats_chanid_to_source_map[3] = "BUS3";
    tmats_chanid_to_source_map[4] = "BUS4";

    b.InitializeMaps(&icd_message_key_to_busnames_map,
                     std::set<uint64_t>({1, 2, 3}),
                     mask,
                     vote_threshold,
                     vote_method_checks_tmats,
                     bus_name_exclusions,
                     tmats_chanid_to_source_map);

    ASSERT_TRUE(b.TmatsPresent());

    std::vector<uint64_t> transmit_cmds = std::vector<uint64_t>({0, 0, 0, 0, 0, 0});
    std::vector<uint64_t> recieve_cmds = std::vector<uint64_t>({10, 11, 12, 13, 14, 15});
    std::vector<uint64_t> channel_ids = std::vector<uint64_t>({0, 1, 2, 3, 4, 5});

    EXPECT_TRUE(b.SubmitMessages(transmit_cmds, recieve_cmds, channel_ids));

    std::map<uint64_t, std::string> res;
    bool continue_translation = b.Finalize(res, 0, false);
    EXPECT_TRUE(iterable_tools_.GetKeys(res).size() == 3);
    EXPECT_TRUE(continue_translation);
    EXPECT_EQ(res[1], "BUSB");
    EXPECT_EQ(res[2], "BUSC");
    EXPECT_EQ(res[3], "BUSD");

    // Also check the source
    std::map<uint64_t, std::pair<std::string, std::string>> final_bus_map_with_source =
        b.GetFinalBusMap_withSource();

    EXPECT_TRUE(iterable_tools_.GetKeys(final_bus_map_with_source).size() == 3);
    EXPECT_EQ(final_bus_map_with_source[1].first, "BUSB");
    EXPECT_EQ(final_bus_map_with_source[1].second, "Vote Method");

    EXPECT_EQ(final_bus_map_with_source[2].first, "BUSC");
    EXPECT_EQ(final_bus_map_with_source[2].second, "Vote Method");

    EXPECT_EQ(final_bus_map_with_source[3].first, "BUSD");
    EXPECT_EQ(final_bus_map_with_source[3].second, "Vote Method");
}

TEST_F(BusMapTest, FinalizeVoteMappingExclusion)
{
    icd_message_key_to_busnames_map[10] = std::set<std::string>({"BUSB"});
    icd_message_key_to_busnames_map[11] = std::set<std::string>({"BUSB"});  // duplicate
    icd_message_key_to_busnames_map[12] = std::set<std::string>({"BUSC"});
    icd_message_key_to_busnames_map[13] = std::set<std::string>({"BUSD"});
    icd_message_key_to_busnames_map[14] = std::set<std::string>({"BUSX"});
    icd_message_key_to_busnames_map[15] = std::set<std::string>({"BUSF"});
    icd_message_key_to_busnames_map[16] = std::set<std::string>({"BusGMUX"});  // not in TMATs but is in exclusion set, should be removed
                                                                               // includes substring and case sensitive tests
    icd_message_key_to_busnames_map[17] = std::set<std::string>({"BUSD"});     // duplicate
    icd_message_key_to_busnames_map[18] = std::set<std::string>({"BUSZ"});     // channel ID 8 will vote for BUSZ, but it should still be removed
                                                                               // since channel ID 8 originated as BUSB in TMATS
                                                                               // and BUSB is in the removal list

    tmats_chanid_to_source_map[1] = "BUSBMUX";  // substring test
    tmats_chanid_to_source_map[2] = "BusC";     // Upper/Lower case test
    tmats_chanid_to_source_map[3] = "BUSD";     // Exact match
    tmats_chanid_to_source_map[4] = "BUSX";     // TMATs bus name replacement
    tmats_chanid_to_source_map[5] = "BUSF";     // Should be corrected and then removed
    tmats_chanid_to_source_map[7] = "BUSD";     // Duplicate
    tmats_chanid_to_source_map[8] = "BUSB";     // Vote result will vote for BUSZ but channel ID 8
                                                // should still be removed since BUSB is in the removal list

    b.InitializeMaps(&icd_message_key_to_busnames_map,
                     std::set<uint64_t>({0, 1, 2, 3, 4, 5, 6, 7, 8}),
                     mask,
                     vote_threshold,
                     vote_method_checks_tmats,
                     bus_name_exclusions,
                     tmats_chanid_to_source_map);

    ASSERT_TRUE(b.TmatsPresent());

    std::vector<uint64_t> transmit_cmds = std::vector<uint64_t>({0, 0, 0, 0, 0, 0, 0, 0, 0});
    std::vector<uint64_t> recieve_cmds = std::vector<uint64_t>({10, 11, 12, 13, 14, 15, 16, 17, 18});
    std::vector<uint64_t> channel_ids = std::vector<uint64_t>({0, 1, 2, 3, 4, 5, 6, 7, 8});

    EXPECT_TRUE(b.SubmitMessages(transmit_cmds, recieve_cmds, channel_ids));

    std::map<uint64_t, std::string> res;
    bool continue_translation = b.Finalize(res, 0, false);
    EXPECT_TRUE(iterable_tools_.GetKeys(res).size() == 9);
    EXPECT_TRUE(continue_translation);
    EXPECT_EQ(res[0], "BUSB");
    EXPECT_EQ(res[1], "BUSB");
    EXPECT_EQ(res[2], "BUSC");
    EXPECT_EQ(res[3], "BUSD");
    EXPECT_EQ(res[4], "BUSX");
    EXPECT_EQ(res[5], "BUSF");
    EXPECT_EQ(res[6], "BusGMUX");
    EXPECT_EQ(res[7], "BUSD");
    EXPECT_EQ(res[8], "BUSZ");

    // Also check the source
    std::map<uint64_t, std::pair<std::string, std::string>> final_bus_map_with_source =
        b.GetFinalBusMap_withSource();

    EXPECT_TRUE(iterable_tools_.GetKeys(final_bus_map_with_source).size() == 9);

    EXPECT_EQ(final_bus_map_with_source[0].first, "BUSB");
    EXPECT_EQ(final_bus_map_with_source[0].second, "Vote Method");
    EXPECT_EQ(final_bus_map_with_source[1].first, "BUSB");
    EXPECT_EQ(final_bus_map_with_source[1].second, "Vote Method");
    EXPECT_EQ(final_bus_map_with_source[2].first, "BUSC");
    EXPECT_EQ(final_bus_map_with_source[2].second, "Vote Method");
    EXPECT_EQ(final_bus_map_with_source[3].first, "BUSD");
    EXPECT_EQ(final_bus_map_with_source[3].second, "Vote Method");
    EXPECT_EQ(final_bus_map_with_source[4].first, "BUSX");
    EXPECT_EQ(final_bus_map_with_source[4].second, "Vote Method");
    EXPECT_EQ(final_bus_map_with_source[5].first, "BUSF");
    EXPECT_EQ(final_bus_map_with_source[5].second, "Vote Method");
    EXPECT_EQ(final_bus_map_with_source[6].first, "BusGMUX");
    EXPECT_EQ(final_bus_map_with_source[6].second, "Vote Method");
    EXPECT_EQ(final_bus_map_with_source[7].first, "BUSD");
    EXPECT_EQ(final_bus_map_with_source[7].second, "Vote Method");
    EXPECT_EQ(final_bus_map_with_source[8].first, "BUSZ");
    EXPECT_EQ(final_bus_map_with_source[8].second, "Vote Method");

    // After applying exclusion set
    BusMap b2;

    bus_name_exclusions = std::set<std::string>({"BUSB", "BUSC", "BUSD", "BUSE", "BUSg"});
    std::map<std::string, std::string> tmats_busname_replacements;
    tmats_busname_replacements["BUSX"] = "BUSE";

    b2.InitializeMaps(&icd_message_key_to_busnames_map,
                      std::set<uint64_t>({0, 1, 2, 3, 4, 5, 6, 7, 8}),
                      mask,
                      vote_threshold,
                      vote_method_checks_tmats,
                      bus_name_exclusions,
                      tmats_chanid_to_source_map,
                      tmats_busname_replacements);

    ASSERT_TRUE(b2.TmatsPresent());

    EXPECT_TRUE(b2.SubmitMessages(transmit_cmds, recieve_cmds, channel_ids));

    continue_translation = b2.Finalize(res, 0, false);
    EXPECT_TRUE(iterable_tools_.GetKeys(res).size() == 1);
    EXPECT_TRUE(continue_translation);
    EXPECT_EQ(res[5], "BUSF");

    // Also check the source
    final_bus_map_with_source =
        b2.GetFinalBusMap_withSource();

    EXPECT_TRUE(iterable_tools_.GetKeys(final_bus_map_with_source).size() == 1);

    EXPECT_EQ(final_bus_map_with_source[5].first, "BUSF");
    EXPECT_EQ(final_bus_map_with_source[5].second, "Vote Method");

    // Check the excluded channel IDs map
    std::map<uint64_t, std::string> excludedChannelIDs = b2.GetExcludedChannelIDs();
    EXPECT_TRUE(iterable_tools_.GetKeys(excludedChannelIDs).size() == 8);
    EXPECT_EQ(excludedChannelIDs[0], "Config Option: bus_name_exclusions");
    EXPECT_EQ(excludedChannelIDs[1], "Config Option: bus_name_exclusions");
    EXPECT_EQ(excludedChannelIDs[2], "Config Option: bus_name_exclusions");
    EXPECT_EQ(excludedChannelIDs[3], "Config Option: bus_name_exclusions");
    EXPECT_EQ(excludedChannelIDs[4], "Config Option: bus_name_exclusions");
    EXPECT_EQ(excludedChannelIDs[6], "Config Option: bus_name_exclusions");
    EXPECT_EQ(excludedChannelIDs[7], "Config Option: bus_name_exclusions");
    EXPECT_EQ(excludedChannelIDs[8], "Config Option: bus_name_exclusions");
}

TEST_F(BusMapTest, FinalizeExclusionTMATS)
{
    icd_message_key_to_busnames_map[11] = std::set<std::string>({"BUSB"});
    icd_message_key_to_busnames_map[12] = std::set<std::string>({"BusC"});
    icd_message_key_to_busnames_map[13] = std::set<std::string>({"BUSD"});
    icd_message_key_to_busnames_map[14] = std::set<std::string>({"BUSF"});
    icd_message_key_to_busnames_map[15] = std::set<std::string>({"BUSX"});
    icd_message_key_to_busnames_map[16] = std::set<std::string>({"BUSBMUX"});
    icd_message_key_to_busnames_map[17] = std::set<std::string>({"BUSE"});

    tmats_chanid_to_source_map[1] = "BUSBMUX";  // Substring test
    tmats_chanid_to_source_map[2] = "BusC";     // Upper/Lower case test
    tmats_chanid_to_source_map[3] = "BUSD";     // Exact match
    tmats_chanid_to_source_map[4] = "BUSX";     // TMATS bus name replacement
    tmats_chanid_to_source_map[5] = "BUSF";     // Should be corrected and then removed
    tmats_chanid_to_source_map[6] = "BUSD";     // Duplicate

    b.InitializeMaps(&icd_message_key_to_busnames_map,
                     std::set<uint64_t>({1, 2, 3, 4, 5, 6}),
                     mask,
                     vote_threshold,
                     vote_method_checks_tmats,
                     bus_name_exclusions,
                     tmats_chanid_to_source_map);

    ASSERT_TRUE(b.TmatsPresent());

    std::map<uint64_t, std::string> res;
    bool continue_translation = b.Finalize(res, true, false);
    EXPECT_TRUE(iterable_tools_.GetKeys(res).size() == 6);
    EXPECT_TRUE(continue_translation);
    EXPECT_EQ(res[1], "BUSBMUX");
    EXPECT_EQ(res[2], "BusC");
    EXPECT_EQ(res[3], "BUSD");
    EXPECT_EQ(res[4], "BUSX");
    EXPECT_EQ(res[5], "BUSF");
    EXPECT_EQ(res[6], "BUSD");

    // Also check the source
    std::map<uint64_t, std::pair<std::string, std::string>> final_bus_map_with_source =
        b.GetFinalBusMap_withSource();

    EXPECT_TRUE(iterable_tools_.GetKeys(final_bus_map_with_source).size() == 6);

    EXPECT_EQ(final_bus_map_with_source[1].first, "BUSBMUX");
    EXPECT_EQ(final_bus_map_with_source[1].second, "TMATS");
    EXPECT_EQ(final_bus_map_with_source[2].first, "BusC");
    EXPECT_EQ(final_bus_map_with_source[2].second, "TMATS");
    EXPECT_EQ(final_bus_map_with_source[3].first, "BUSD");
    EXPECT_EQ(final_bus_map_with_source[3].second, "TMATS");
    EXPECT_EQ(final_bus_map_with_source[4].first, "BUSX");
    EXPECT_EQ(final_bus_map_with_source[4].second, "TMATS");
    EXPECT_EQ(final_bus_map_with_source[5].first, "BUSF");
    EXPECT_EQ(final_bus_map_with_source[5].second, "TMATS");
    EXPECT_EQ(final_bus_map_with_source[6].first, "BUSD");
    EXPECT_EQ(final_bus_map_with_source[6].second, "TMATS");

    // After applying exclusion set
    BusMap b2;

    bus_name_exclusions = std::set<std::string>({"BUSB", "BUSC", "BUSD", "BUSE"});
    std::map<std::string, std::string> tmats_busname_replacements;
    tmats_busname_replacements["BUSX"] = "BUSE";

    b2.InitializeMaps(&icd_message_key_to_busnames_map,
                      std::set<uint64_t>({1, 2, 3, 4, 5, 6}),
                      mask,
                      vote_threshold,
                      vote_method_checks_tmats,
                      bus_name_exclusions,
                      tmats_chanid_to_source_map,
                      tmats_busname_replacements);

    ASSERT_TRUE(b2.TmatsPresent());

    continue_translation = b2.Finalize(res, true, false);
    EXPECT_TRUE(iterable_tools_.GetKeys(res).size() == 1);
    EXPECT_TRUE(continue_translation);
    EXPECT_EQ(res[5], "BUSF");

    // Also check the source
    final_bus_map_with_source =
        b2.GetFinalBusMap_withSource();

    EXPECT_TRUE(iterable_tools_.GetKeys(final_bus_map_with_source).size() == 1);

    EXPECT_EQ(final_bus_map_with_source[5].first, "BUSF");
    EXPECT_EQ(final_bus_map_with_source[5].second, "TMATS");

    // Check the excluded channel IDs map
    std::map<uint64_t, std::string> excludedChannelIDs = b2.GetExcludedChannelIDs();
    EXPECT_TRUE(iterable_tools_.GetKeys(excludedChannelIDs).size() == 5);
    EXPECT_EQ(excludedChannelIDs[1], "Config Option: bus_name_exclusions");
    EXPECT_EQ(excludedChannelIDs[2], "Config Option: bus_name_exclusions");
    EXPECT_EQ(excludedChannelIDs[3], "Config Option: bus_name_exclusions");
    EXPECT_EQ(excludedChannelIDs[4], "Config Option: bus_name_exclusions");
    EXPECT_EQ(excludedChannelIDs[6], "Config Option: bus_name_exclusions");
}

TEST_F(BusMapTest, FinalizeVoteMappingFewerChannelIDsThanNecessaryAndOverridesTMATSMap)
{
    icd_message_key_to_busnames_map[10] = std::set<std::string>({"BUSA"});
    icd_message_key_to_busnames_map[11] = std::set<std::string>({"BUSB"});
    icd_message_key_to_busnames_map[12] = std::set<std::string>({"BUSC"});
    icd_message_key_to_busnames_map[13] = std::set<std::string>({"BUSD"});
    icd_message_key_to_busnames_map[14] = std::set<std::string>({"BUSE"});
    icd_message_key_to_busnames_map[15] = std::set<std::string>({"BUSF"});

    tmats_chanid_to_source_map[1] = "BUS1";
    tmats_chanid_to_source_map[2] = "BUS2";
    tmats_chanid_to_source_map[3] = "BUS3";
    tmats_chanid_to_source_map[4] = "BUS4";

    b.InitializeMaps(&icd_message_key_to_busnames_map,
                     std::set<uint64_t>({1, 2, 3}),
                     mask,
                     vote_threshold,
                     vote_method_checks_tmats,
                     bus_name_exclusions,
                     tmats_chanid_to_source_map);

    ASSERT_TRUE(b.TmatsPresent());

    std::vector<uint64_t> transmit_cmds = std::vector<uint64_t>({0, 0, 0});
    std::vector<uint64_t> recieve_cmds = std::vector<uint64_t>({10, 11, 12});
    std::vector<uint64_t> channel_ids = std::vector<uint64_t>({0, 1, 2});

    EXPECT_TRUE(b.SubmitMessages(transmit_cmds, recieve_cmds, channel_ids));

    std::map<uint64_t, std::string> res;
    bool continue_translation = b.Finalize(res, 0, false);
    EXPECT_TRUE(iterable_tools_.GetKeys(res).size() == 2);
    EXPECT_TRUE(continue_translation);
    EXPECT_EQ(res[1], "BUSB");
    EXPECT_EQ(res[2], "BUSC");

    // Also check the source
    std::map<uint64_t, std::pair<std::string, std::string>> final_bus_map_with_source =
        b.GetFinalBusMap_withSource();

    EXPECT_TRUE(iterable_tools_.GetKeys(final_bus_map_with_source).size() == 2);
    EXPECT_EQ(final_bus_map_with_source[1].first, "BUSB");
    EXPECT_EQ(final_bus_map_with_source[1].second, "Vote Method");

    EXPECT_EQ(final_bus_map_with_source[2].first, "BUSC");
    EXPECT_EQ(final_bus_map_with_source[2].second, "Vote Method");
}

TEST_F(BusMapTest, VoteThreshold)
{
    icd_message_key_to_busnames_map[11] = std::set<std::string>({"BUSB"});
    icd_message_key_to_busnames_map[12] = std::set<std::string>({"BUSC"});
    icd_message_key_to_busnames_map[13] = std::set<std::string>({"BUSC"});

    uint64_t threshold = 2;

    b.InitializeMaps(&icd_message_key_to_busnames_map,
                     std::set<uint64_t>({1, 2}),
                     mask,
                     threshold);

    std::vector<uint64_t> transmit_cmds = std::vector<uint64_t>({0, 0});
    std::vector<uint64_t> recieve_cmds = std::vector<uint64_t>({11, 12});
    std::vector<uint64_t> channel_ids = std::vector<uint64_t>({1, 2});

    EXPECT_TRUE(b.SubmitMessages(transmit_cmds, recieve_cmds, channel_ids));

    std::map<uint64_t, std::string> res;

    // Even though votes were made, the votes did not
    // meet the threshold of >= 2 votes
    bool continue_translation = b.Finalize(res, 0, false);
    EXPECT_TRUE(iterable_tools_.GetKeys(res).size() == 0);

    // Check the excluded channel IDs map
    std::map<uint64_t, std::string> excludedChannelIDs = b.GetExcludedChannelIDs();
    EXPECT_TRUE(iterable_tools_.GetKeys(excludedChannelIDs).size() == 2);
    EXPECT_EQ(excludedChannelIDs[1], "Vote count not >= vote_threshold");
    EXPECT_EQ(excludedChannelIDs[2], "Vote count not >= vote_threshold");

    // add another vote for channel ID 2 and expect
    // a map for that channel ID 2 but not for channel
    // ID 1
    std::vector<uint64_t> transmit_cmds2 = std::vector<uint64_t>({0});
    std::vector<uint64_t> recieve_cmds2 = std::vector<uint64_t>({13});
    std::vector<uint64_t> channel_ids2 = std::vector<uint64_t>({2});

    EXPECT_TRUE(b.SubmitMessages(transmit_cmds2, recieve_cmds2, channel_ids2));

    continue_translation = b.Finalize(res, 0, false);
    EXPECT_TRUE(iterable_tools_.GetKeys(res).size() == 1);
    EXPECT_TRUE(continue_translation);
    EXPECT_EQ(res[2], "BUSC");

    // Also check the source
    std::map<uint64_t, std::pair<std::string, std::string>> final_bus_map_with_source =
        b.GetFinalBusMap_withSource();

    EXPECT_TRUE(iterable_tools_.GetKeys(final_bus_map_with_source).size() == 1);
    EXPECT_EQ(final_bus_map_with_source[2].first, "BUSC");
    EXPECT_EQ(final_bus_map_with_source[2].second, "Vote Method");

    // Check the excluded channel IDs map
    excludedChannelIDs = b.GetExcludedChannelIDs();
    EXPECT_TRUE(iterable_tools_.GetKeys(excludedChannelIDs).size() == 1);
    EXPECT_EQ(excludedChannelIDs[1], "Vote count not >= vote_threshold");
}

TEST_F(BusMapTest, FinalizeClearExistingMap)
{
    icd_message_key_to_busnames_map[11] = std::set<std::string>({"BUS1"});

    tmats_chanid_to_source_map[1] = "BUS1";

    b.InitializeMaps(&icd_message_key_to_busnames_map,
                     std::set<uint64_t>({1, 2, 3}),
                     mask,
                     vote_threshold,
                     vote_method_checks_tmats,
                     bus_name_exclusions,
                     tmats_chanid_to_source_map);

    std::map<uint64_t, std::string> res;
    res[10] = "junk";

    b.Finalize(res, 1, false);
    EXPECT_TRUE(iterable_tools_.GetKeys(res).size() == 1);
    EXPECT_EQ(res[1], "BUS1");
}

TEST_F(BusMapTest, FinalizeReturnsFalseIfNothingMappedAndUserStopIsFalse)
{
    icd_message_key_to_busnames_map[10] = std::set<std::string>({"BUSA"});
    icd_message_key_to_busnames_map[11] = std::set<std::string>({"BUSB"});

    b.InitializeMaps(&icd_message_key_to_busnames_map,
                     std::set<uint64_t>({1, 2, 3}),
                     mask,
                     vote_threshold,
                     vote_method_checks_tmats,
                     bus_name_exclusions,
                     tmats_chanid_to_source_map);

    std::vector<uint64_t> transmit_cmds = std::vector<uint64_t>({0, 0});
    std::vector<uint64_t> recieve_cmds = std::vector<uint64_t>({10, 11});
    std::vector<uint64_t> channel_ids = std::vector<uint64_t>({5, 8});

    EXPECT_TRUE(b.SubmitMessages(transmit_cmds, recieve_cmds, channel_ids));

    std::map<uint64_t, std::string> res;
    bool continue_translation = b.Finalize(res, 0, false);
    EXPECT_FALSE(continue_translation);
    EXPECT_TRUE(iterable_tools_.GetKeys(res).size() == 0);
}

TEST_F(BusMapTest, FinalizeReturnsFalseIfEmptyChannelIDs)
{
    icd_message_key_to_busnames_map[10] = std::set<std::string>({"BUSA"});
    icd_message_key_to_busnames_map[11] = std::set<std::string>({"BUSB"});

    b.InitializeMaps(&icd_message_key_to_busnames_map,
                     std::set<uint64_t>(),
                     mask,
                     vote_threshold,
                     vote_method_checks_tmats,
                     bus_name_exclusions,
                     tmats_chanid_to_source_map);

    std::vector<uint64_t> transmit_cmds = std::vector<uint64_t>({0, 0});
    std::vector<uint64_t> recieve_cmds = std::vector<uint64_t>({10, 11});
    std::vector<uint64_t> channel_ids = std::vector<uint64_t>({5, 8});

    EXPECT_TRUE(b.SubmitMessages(transmit_cmds, recieve_cmds, channel_ids));

    std::map<uint64_t, std::string> res;
    bool continue_translation = b.Finalize(res, 0, false);
    EXPECT_FALSE(continue_translation);
    EXPECT_TRUE(iterable_tools_.GetKeys(res).size() == 0);
}

TEST_F(BusMapTest, FinalizeReturnsFalseIfNoVotes)
{
    icd_message_key_to_busnames_map[10] = std::set<std::string>({"BUSA"});
    icd_message_key_to_busnames_map[11] = std::set<std::string>({"BUSB"});

    b.InitializeMaps(&icd_message_key_to_busnames_map,
                     std::set<uint64_t>({1, 4, 8}),
                     mask,
                     vote_threshold,
                     vote_method_checks_tmats,
                     bus_name_exclusions,
                     tmats_chanid_to_source_map);

    std::map<uint64_t, std::string> res;
    bool continue_translation = b.Finalize(res, 0, false);
    EXPECT_FALSE(continue_translation);
    EXPECT_TRUE(iterable_tools_.GetKeys(res).size() == 0);
}

TEST_F(BusMapTest, GetFinalMaps)
{
    std::map<uint64_t, std::pair<std::string, std::string>> final_map_with_source{
        {11, {"busa", "tmats"}},
        {22, {"BUSS", "vote and tmats"}},
        {27, {"spec", "user"}}
    };

    std::map<uint64_t, std::string> chanid_to_name;
    std::map<uint64_t, std::string> chanid_to_source;
    b.GetFinalMaps(final_map_with_source, chanid_to_name, chanid_to_source);

    std::map<uint64_t, std::string> expected_chanid_to_name{
        {11, "busa"},
        {22, "BUSS"},
        {27, "spec"}
    };
    std::map<uint64_t, std::string> expected_chanid_to_source{
        {11, "tmats"},
        {22, "vote and tmats"},
        {27, "user"}
    };

    EXPECT_THAT(expected_chanid_to_name, ::testing::ContainerEq(chanid_to_name));
    EXPECT_THAT(expected_chanid_to_source, ::testing::ContainerEq(chanid_to_source));
}