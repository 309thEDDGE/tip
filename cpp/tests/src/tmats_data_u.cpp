#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "tmats_data.h"

TEST(TMATSDataTest, CombineMaps)
{
    TMATSData td;

    cmapmap map1 = {
        {"1", {{"2", "a"}, {"3", "b"}}},
        {"12", {{"22", "aa"}, {"33", "bb"}}}
    };
    
    cmapmap map2 = {
        {"1", {{"2", "A"}, {"3", "B"}}}
    };

    cmapmap expected = {
        {"1", {{"a", "A"}, {"b", "B"}}}
    };

    cmapmap result;
    td.CombineMaps(map1, map2, result);

    EXPECT_THAT(expected, ::testing::ContainerEq(result));
}