#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "subchannel_map.h"

class SubchannelMapTest : public ::testing::Test
{
   protected:

};


// Test MapSubchannelNameAndNumberToChannelID
TEST_F(SubchannelMapTest, MapSubchannelNameAndNumberToChannelIDMultipleSubchannels)
{
    // Multiple subchannels to single channel id
    EXPECT_TRUE(false);
}

// Test MapSubchannelNameAndNumberToChannelID
TEST_F(SubchannelMapTest, MapSubchannelNameAndNumberToChannelIDNoSubchannelInfo)
{
    // no subchannel information available
    EXPECT_TRUE(false);
}

// Test MapSubchannelNameAndNumberToChannelID
TEST_F(SubchannelMapTest, MapSubchannelNameAndNumberToChannelIDMultipleChannelIDs)
{
    // multiple channel ids with one or more subchannels
    EXPECT_TRUE(false);
}



// Test GetNameOfARINC429Bus
TEST_F(SubchannelMapTest, GetNameOfARINC429BusMissingBus)
{
    // channel id not found


    // Channel ID and bus number found
    EXPECT_TRUE(false);
}

// Test GetNameOfARINC429Bus
TEST_F(SubchannelMapTest, GetNameOfARINC429BusMissingSubchannelNumber)
{
    //  Channelid found but
    // subchannel number not found
    EXPECT_TRUE(false);
}

// Test GetNameOfARINC429Bus
TEST_F(SubchannelMapTest, GetNameOfARINC429BusFoundName)
{
    // item found and returned
    EXPECT_TRUE(false);
}

// Test GetNameOfARINC429Bus
TEST_F(SubchannelMapTest, GetNameOfARINC429BusFoundName)
{
    // item found and returned where multiple subchannels map to single channel id
    EXPECT_TRUE(false);
}


TEST_F(SubchannelMapTest, Ingest429ParserMDDocNoTMATSChanTo429SubchanElement)
{
    // Expect false if no tmats_chanid_to_429_subchan_and_name element in metadata
    EXPECT_TRUE(false);
}


TEST_F(SubchannelMapTest, Ingest429ParserMDDocNoTMATSChanTo429SubchanElement)
{
    // Expect false if tmats_chanid_to_429_subchan_and_name is present but has no associated data
    EXPECT_TRUE(false);
}

