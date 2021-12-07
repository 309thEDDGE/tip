#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "md_category.h"

class MDCategoryTest : public ::testing::Test
{
protected:
    MDCategory mdcat_;
public:
    MDCategoryTest() : mdcat_("cat")
    {}
};

TEST_F(MDCategoryTest, SetValue)
{
    YAML::Node n;
    int val = 59843;
    n.push_back(val);
    ASSERT_TRUE(n.IsSequence());
    mdcat_.SetValue(n);
    ASSERT_EQ(1, mdcat_.node.size());
    EXPECT_EQ(val, mdcat_.node[0].as<int>());
}
