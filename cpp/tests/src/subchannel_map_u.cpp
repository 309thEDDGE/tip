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
}

// Test MapSubchannelNameAndNumberToChannelID
TEST_F(SubchannelMapTest, MapSubchannelNameAndNumberToChannelIDNoSubchannelInfo)
{
    // no subchannel information available
}

// Test MapSubchannelNameAndNumberToChannelID
TEST_F(SubchannelMapTest, MapSubchannelNameAndNumberToChannelIDMultipleChannelIDs)
{
    // multiple channel ids with one or more subchannels
}



// Test GetNameOfARINC429Bus
TEST_F(SubchannelMapTest, GetNameOfARINC429BusMissingBus)
{
    // channel id not found


    // Channel ID and bus number found
    EXPECT_TRUE(continue_translation);
}

// Test GetNameOfARINC429Bus
TEST_F(SubchannelMapTest, GetNameOfARINC429BusMissingSubchannelNumber)
{
    //  Channelid found but
    // subchannel number not found
}

// Test GetNameOfARINC429Bus
TEST_F(SubchannelMapTest, GetNameOfARINC429BusFoundName)
{
    // item found and returned
}

// Test GetNameOfARINC429Bus
TEST_F(SubchannelMapTest, GetNameOfARINC429BusFoundName)
{
    // item found and returned where multiple subchannels map to single channel id
}