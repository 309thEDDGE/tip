#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "md_category_scalar.h"

class MDCategoryScalarTest : public ::testing::Test
{
protected:
    MDCategoryScalar mdscalar_;

public:
    MDCategoryScalarTest() : mdscalar_("type")
    {}
};

TEST_F(MDCategoryScalarTest, InitAsEmptyString)
{
    EXPECT_EQ("", mdscalar_.node.as<std::string>());
}

TEST_F(MDCategoryScalarTest, SetScalarValue)
{
    int val = 10;
    mdscalar_.SetScalarValue(val);
    EXPECT_EQ(val, mdscalar_.node.as<int>());
}

