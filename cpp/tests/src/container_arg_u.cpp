#include <vector>
#include <map>
#include <memory>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "container_arg.h"


class ContainerArgTest : public ::testing::Test
{
   protected:
    std::shared_ptr<ContainerArg> carg_ptr_; 
    std::string user_str_;
    ContainerArg carg_;
    std::from_chars_result fcres_;

    ContainerArgTest() : carg_ptr_(nullptr), user_str_(""), carg_(), fcres_{}
    { }

};

TEST_F(ContainerArgTest, CheckFromCharsResultExactMatch)
{
    user_str_ = "43";
    int result = 0;
    fcres_ = std::from_chars(user_str_.data(), user_str_.data()+user_str_.size(), result);
    ASSERT_TRUE(carg_.CheckFromCharsResultExactMatch(fcres_, user_str_));
    EXPECT_EQ(43, result);

    user_str_ = "bad";
    fcres_ = std::from_chars(user_str_.data(), user_str_.data()+user_str_.size(), result);
    ASSERT_FALSE(carg_.CheckFromCharsResultExactMatch(fcres_, user_str_));

    user_str_ = "10 bad";
    fcres_ = std::from_chars(user_str_.data(), user_str_.data()+user_str_.size(), result);
    ASSERT_FALSE(carg_.CheckFromCharsResultExactMatch(fcres_, user_str_));
}

TEST_F(ContainerArgTest, ParseVecInt)
{
    user_str_ = "43";
    std::vector<int> v;
    int parsed_val = 0;
    carg_ptr_ = MakeContainerArg(v, parsed_val);
    ASSERT_TRUE(carg_ptr_->Parse(user_str_));
    ASSERT_EQ(1, v.size());
    EXPECT_EQ(43, v.at(0));

    user_str_ = "91";
    ASSERT_TRUE(carg_ptr_->Parse(user_str_));
    ASSERT_EQ(2, v.size());
    EXPECT_EQ(91, v.at(1));
}

TEST_F(ContainerArgTest, ParseVecIntFail)
{
    user_str_ = "43d";
    std::vector<int> v;
    int parsed_val = 0;
    carg_ptr_ = MakeContainerArg(v, parsed_val);
    ASSERT_FALSE(carg_ptr_->Parse(user_str_));
    ASSERT_EQ(0, v.size());
}

TEST_F(ContainerArgTest, ParseVecString)
{
    user_str_ = "43d";
    std::vector<std::string> v;
    std::string parsed_val = "";
    carg_ptr_ = MakeContainerArg(v, parsed_val);
    ASSERT_TRUE(carg_ptr_->Parse(user_str_));
    ASSERT_EQ(1, v.size());
    EXPECT_EQ(user_str_, v.at(0));
}

TEST_F(ContainerArgTest, ParseVecFloat)
{
    user_str_ = "90";
    std::vector<float> v;
    float parsed_val = 0;
    carg_ptr_ = MakeContainerArg(v, parsed_val);
    ASSERT_TRUE(carg_ptr_->Parse(user_str_));
    ASSERT_EQ(1, v.size());
    EXPECT_EQ(90.0, v.at(0));

    user_str_ = "65.3";
    ASSERT_TRUE(carg_ptr_->Parse(user_str_));
    ASSERT_EQ(2, v.size());
    float expected = 65.3F;
    EXPECT_EQ(expected, v.at(1));

    user_str_ = "notdouble";
    ASSERT_FALSE(carg_ptr_->Parse(user_str_));
}

TEST_F(ContainerArgTest, ParseVecDouble)
{
    user_str_ = "90";
    std::vector<double> v;
    double parsed_val = 0;
    carg_ptr_ = MakeContainerArg(v, parsed_val);
    ASSERT_TRUE(carg_ptr_->Parse(user_str_));
    ASSERT_EQ(1, v.size());
    double expected = 90.0;
    EXPECT_EQ(90.0, v.at(0));

    user_str_ = "65.3";
    ASSERT_TRUE(carg_ptr_->Parse(user_str_));
    ASSERT_EQ(2, v.size());
    expected = 65.3;
    EXPECT_EQ(expected, v.at(1));

    user_str_ = "notdouble";
    ASSERT_FALSE(carg_ptr_->Parse(user_str_));
}

TEST_F(ContainerArgTest, ParseVecFloatFail)
{
    user_str_ = "90 ff";
    std::vector<float> v;
    float parsed_val(0);
    carg_ptr_ = MakeContainerArg(v, parsed_val);
    ASSERT_FALSE(carg_ptr_->Parse(user_str_));
    ASSERT_EQ(0, v.size());
}

TEST_F(ContainerArgTest, SplitOnColonEmptyString)
{
    user_str_ = "90:";
    std::string key = "";
    std::string val = "";
    ASSERT_FALSE(carg_.SplitOnColon(user_str_, key, val));

    user_str_ = ":data";
    ASSERT_FALSE(carg_.SplitOnColon(user_str_, key, val));
}

TEST_F(ContainerArgTest, SplitOnColonNoColon)
{
    user_str_ = "90data";
    std::string key = "";
    std::string val = "";
    ASSERT_FALSE(carg_.SplitOnColon(user_str_, key, val));
}

TEST_F(ContainerArgTest, SplitOnColonMultipleColon)
{
    user_str_ = "90data:day:rate";
    std::string key = "";
    std::string val = "";
    ASSERT_FALSE(carg_.SplitOnColon(user_str_, key, val));
}

TEST_F(ContainerArgTest, SplitOnColon)
{
    user_str_ = "90:data";
    std::string key = "";
    std::string val = "";
    ASSERT_TRUE(carg_.SplitOnColon(user_str_, key, val));
    EXPECT_EQ("90", key);
    EXPECT_EQ("data", val);

    user_str_ = "a good day:friday";
    ASSERT_TRUE(carg_.SplitOnColon(user_str_, key, val));
    EXPECT_EQ("a good day", key);
    EXPECT_EQ("friday", val);
}

TEST_F(ContainerArgTest, ParseMapInt)
{
    user_str_ = "90:100";
    std::map<int, int> m;
    int k, v;
    carg_ptr_ = MakeContainerArg(m, k, v);
    ASSERT_TRUE(carg_ptr_->Parse(user_str_));
    ASSERT_EQ(1, m.size());
    EXPECT_EQ(1, m.count(90));
    EXPECT_EQ(100, m.at(90));

    user_str_ = "33421:-21";
    ASSERT_TRUE(carg_ptr_->Parse(user_str_));
    ASSERT_EQ(2, m.size());
    EXPECT_EQ(1, m.count(33421));
    EXPECT_EQ(-21, m.at(33421));
}

TEST_F(ContainerArgTest, ParseMapIntFail)
{
    std::map<int, int> m;
    int k, v;
    carg_ptr_ = MakeContainerArg(m, k, v);
    user_str_ = "65:";
    ASSERT_FALSE(carg_ptr_->Parse(user_str_));
    ASSERT_EQ(0, m.size());

    user_str_ = "3:data";
    ASSERT_FALSE(carg_ptr_->Parse(user_str_));
    ASSERT_EQ(0, m.size());

    user_str_ = "a:0";
    ASSERT_FALSE(carg_ptr_->Parse(user_str_));
    ASSERT_EQ(0, m.size());
}

TEST_F(ContainerArgTest, ParseMapString)
{
    user_str_ = "90:100";
    std::map<std::string, std::string> m;
    std::string k, v;
    carg_ptr_ = MakeContainerArg(m, k, v);
    ASSERT_TRUE(carg_ptr_->Parse(user_str_));
    ASSERT_EQ(1, m.size());
    EXPECT_EQ(1, m.count("90"));
    EXPECT_EQ("100", m.at("90"));

    user_str_ = "test:data";
    ASSERT_TRUE(carg_ptr_->Parse(user_str_));
    ASSERT_EQ(2, m.size());
    EXPECT_EQ(1, m.count("test"));
    EXPECT_EQ("data", m.at("test"));
}

TEST_F(ContainerArgTest, ParseMapStringFail)
{
    user_str_ = ":abd0";
    std::map<std::string, std::string> m;
    std::string k, v;
    carg_ptr_ = MakeContainerArg(m, k, v);
    ASSERT_FALSE(carg_ptr_->Parse(user_str_));
}

TEST_F(ContainerArgTest, ParseMapMixedType)
{
    user_str_ = "90:100";
    std::map<std::string, double> m;
    std::string k;
    double v;
    carg_ptr_ = MakeContainerArg(m, k, v);
    ASSERT_TRUE(carg_ptr_->Parse(user_str_));
    ASSERT_EQ(1, m.size());
    EXPECT_EQ(1, m.count("90"));
    EXPECT_EQ(100.0, m.at("90"));

    user_str_ = "test:33.8";
    ASSERT_TRUE(carg_ptr_->Parse(user_str_));
    ASSERT_EQ(2, m.size());
    EXPECT_EQ(1, m.count("test"));
    EXPECT_EQ(33.8, m.at("test"));
}

TEST_F(ContainerArgTest, ParseMapMixedTypeFail1)
{
    user_str_ = "90:data";
    std::map<std::string, double> m;
    std::string k;
    double v;
    carg_ptr_ = MakeContainerArg(m, k, v);
    ASSERT_FALSE(carg_ptr_->Parse(user_str_));
    ASSERT_EQ(0, m.size());
}

TEST_F(ContainerArgTest, ParseMapMixedTypeFail2)
{
    user_str_ = "FAil:9.1";
    std::map<int, double> m;
    int k;
    double v;
    carg_ptr_ = MakeContainerArg(m, k, v);
    ASSERT_FALSE(carg_ptr_->Parse(user_str_));
    ASSERT_EQ(0, m.size());
}