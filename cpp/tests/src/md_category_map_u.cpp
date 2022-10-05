#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "md_category_map.h"

class MDCategoryMapTest : public ::testing::Test
{
protected:
    MDCategoryMap mdcat_;
    std::string key_;
    YAML::Node node_;

public:
    MDCategoryMapTest() : mdcat_("type"), key_("one")
    {}
};

TEST_F(MDCategoryMapTest, InitAsEmptyMap)
{
    EXPECT_TRUE(mdcat_.node.IsMap());
    EXPECT_TRUE(mdcat_.node.size() == 0);
}

TEST_F(MDCategoryMapTest, SetMapKeys)
{
    std::vector<std::string> keys{"one", "two"};
    ASSERT_TRUE(mdcat_.SetMapKeys(keys));
    EXPECT_TRUE(mdcat_.node.IsMap());
    EXPECT_EQ(keys.size(), mdcat_.node.size());
    EXPECT_EQ(keys.size(), mdcat_.keys.size());
}

TEST_F(MDCategoryMapTest, SetMapKeysAlreadyConfigured)
{
    std::vector<std::string> keys{"one", "two"};
    ASSERT_TRUE(mdcat_.SetMapKeys(keys));
    EXPECT_FALSE(mdcat_.SetMapKeys(keys));
}

TEST_F(MDCategoryMapTest, SetMappedValueKeysNotDefined)
{
    int val = 10;
    ASSERT_FALSE(mdcat_.SetMappedValue(key_, val));
}

TEST_F(MDCategoryMapTest, SetMappedValueKeyNotInKeys)
{
    int val = 10;
    std::vector<std::string> keys{"one", "two"};
    ASSERT_TRUE(mdcat_.SetMapKeys(keys));
    key_ = "three";
    EXPECT_FALSE(mdcat_.SetMappedValue(key_, val));
}

TEST_F(MDCategoryMapTest, SetMappedValue)
{
    int val = 10;
    std::vector<std::string> keys{"one", "two"};
    ASSERT_TRUE(mdcat_.SetMapKeys(keys));
    key_ = "one";
    EXPECT_TRUE(mdcat_.SetMappedValue(key_, val));
    EXPECT_EQ(val, mdcat_.node[key_].as<int>());
}

TEST_F(MDCategoryMapTest, PopulateNodeSet)
{
    std::set<int> data{3, 5, 6};
    mdcat_.PopulateNode(node_, data);

    ASSERT_TRUE(node_.IsSequence());
    ASSERT_TRUE(node_.size() == 3);
    EXPECT_EQ(node_[0].as<int>(), 3);
}

TEST_F(MDCategoryMapTest, PopulateNodeVector)
{
    std::vector<int> data{3, 5, 6};
    mdcat_.PopulateNode(node_, data);

    ASSERT_TRUE(node_.IsSequence());
    ASSERT_TRUE(node_.size() == 3);
    EXPECT_EQ(node_[0].as<int>(), 3);
}

TEST_F(MDCategoryMapTest, PopulateNodeScalar)
{
    int data = 32;
    mdcat_.PopulateNode(node_, data);
    ASSERT_TRUE(node_.IsScalar());
    EXPECT_EQ(node_.as<int>(), 32);
}

TEST_F(MDCategoryMapTest, PopulateNodeMap)
{
    std::map<int, int> data{
        {1, 10},
        {2, 20}
    };
    mdcat_.PopulateNode(node_, data);
    ASSERT_TRUE(node_.IsMap());
    ASSERT_TRUE(node_.size() == 2);
    EXPECT_EQ(node_["2"].as<int>(), 20);
}

TEST_F(MDCategoryMapTest, PopulateNodeMapToVec)
{
    std::map<int, std::vector<int>> data{
        {90, {1, 10}},
        {70, {2, 20}}
    };
    mdcat_.PopulateNode(node_, data);
    ASSERT_TRUE(node_.IsMap());
    ASSERT_TRUE(node_.size() == 2);
    ASSERT_TRUE(node_["90"].IsSequence());
    ASSERT_TRUE(node_["90"].size() == 2);
    EXPECT_EQ(10, node_["90"][1].as<int>());
}

TEST_F(MDCategoryMapTest, PopulateNodeMapToSet)
{
    std::map<int, std::set<int>> data{
        {90, {1, 10}},
        {70, {2, 20}}
    };
    mdcat_.PopulateNode(node_, data);
    ASSERT_TRUE(node_.IsMap());
    ASSERT_TRUE(node_.size() == 2);
    ASSERT_TRUE(node_["90"].IsSequence());
    ASSERT_TRUE(node_["90"].size() == 2);
    EXPECT_EQ(10, node_["90"][1].as<int>());
}

TEST_F(MDCategoryMapTest, PopulateNodeMapToVecOfVec)
{
    std::map<int, std::vector<std::vector<int>>> data{
        {90, {{4, 13}, {1, 10}}},
        {100, {{5, 23}, {2, 20}}}
    };
    mdcat_.PopulateNode(node_, data);
    ASSERT_TRUE(node_.IsMap());
    ASSERT_TRUE(node_.size() == 2);
    ASSERT_TRUE(node_["90"].IsSequence());
    ASSERT_TRUE(node_["90"].size() == 2);
    ASSERT_TRUE(node_["90"][0].IsSequence());
    ASSERT_TRUE(node_["90"][1].IsSequence());
    EXPECT_EQ(4, node_["90"][0][0].as<int>());
    EXPECT_EQ(10, node_["90"][1][1].as<int>());
}