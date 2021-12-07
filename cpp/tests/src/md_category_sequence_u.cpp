#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "md_category_sequence.h"

class MDCategorySequenceTest : public ::testing::Test
{
protected:
    MDCategorySequence mdcat_;

public:
    MDCategorySequenceTest() : mdcat_("type")
    {}
};

TEST_F(MDCategorySequenceTest, InitAsEmptySequence)
{
    EXPECT_TRUE(mdcat_.node.IsSequence());
    EXPECT_TRUE(mdcat_.node.size() == 0);
}

TEST_F(MDCategorySequenceTest, Insert)
{
    int val = 214;
    mdcat_.Insert(val);
    EXPECT_EQ(val, mdcat_.node[0].as<int>());
}