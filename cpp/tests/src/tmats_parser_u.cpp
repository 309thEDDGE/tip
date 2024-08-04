#include <gtest/gtest.h>
#include "gmock/gmock.h"
#include "tmats_parser.h"

const std::string TMATS =
    R"(G\TA:PIT_1(14-002);)"
    "\n"
    R"(G\106:07;)"
    "\n"
    R"(G\DSI\N:1;)"
    "\n"
    R"(G\DSI-1:DATASOURCE;)"
    "\n"
    R"(G\DST-1:OTH;)"
    "\n"
    R"(G\OD:05/15/2015;)"
    "\n"
    R"(G\UD:05/15/2015;)"
    "\n"
    R"(G\POC\N:1;)"
    "\n"
    R"(G\POC1-1:ILIAD HeimRecorderGen;)"
    "\n"
    R"(G\COM:Generated by ILIAD Unit Generator build 802.12.0.284 on 2015/05/15 11:35:20;)"
    "\n"
    R"(R-1\ID:DATASOURCE;)"
    "\n"
    R"(R-1\TC1:SSR;)"
    "\n"
    R"(R-1\RI1:Heim;)"
    "\n"
    R"(R-1\RI2:D200F;)"
    "\n"
    R"(R-1\RI3:Y;)"
    "\n"
    R"(R-1\N:28;)"
    "\n"
    R"(R-1\EV\E:F;)"
    "\n"
    R"(R-1\IDX\E:F;)"
    "\n"
    R"(R-1\RID:HeimSystems;)"
    "\n"
    R"(R-1\NSB:4;)"
    "\n"
    R"(V-1\ID:DATASOURCE;)"
    "\n"
    R"(V-1\VN:HDS;)"
    "\n"
    R"(R-1\DSI-1:Time;)"
    "\n"
    R"(R-1\TK1-1:1;)"
    "\n"
    R"(R-1\CHE-1:T;)"
    "\n"
    R"(R-1\CDT-1:TIMEIN;)"
    "\n"
    R"(R-1\TK4-1:1;)"
    "\n"
    R"(R-1\TTF-1:1;)"
    "\n"
    R"(R-1\CDLN-1:TIMECHANNEL;)"
    "\n"
    R"(R-1\TFMT-1:B;)"
    "\n"
    R"(R-1\TSRC-1:E;)"
    "\n"
    R"(P-1\F1:16;)"
    "\n"
    R"(P-4\F1:20;)"
    "\n"
    R"(P-4\DLN:pcm4;)"
    "\n"
    R"(P-1\D4:N;)"
    "\n"
    R"(P-1\DLN:pcm1;)"
    "\n"
    R"(V-1\HDS\SYS:sY1a-;)"
    "\n"
    R"(V-1\HDS\SYS:solRmRa5b5e3hNiBk1pAr0u-n+sBy-z6;)"
    "\n"
    R"(V-1\HDS\SYS:sco3;)"
    "\n"
    R"(V-1\HDS\SYS:sfh5o5r-t-;)"
    "\n"
    R"(V-1\HDS\SYS:ssb25000c+e-g-hNiIkIn0q2a0;)"
    "\n"
    R"(V-1\HDS\SYS:ssr38400;)"
    "\n"
    R"(V-1\HDS\SYS:sam;)"
    "\n"
    R"(R-1\DSI-2:UAR-1;)"
    "\n"
    R"(R-1\TK1-2:2;)"
    "\n"
    R"(R-1\CHE-2:T;)"
    "\n"
    R"(R-1\CDT-2:1553IN;)"
    "\n"
    R"(R-1\TK4-2:2;)"
    "\n"
    R"(R-1\CDLN-2:UAR-1;)"
    "\n"
    R"(R-1\BTF-2:1;)"
    "\n"
    R"(B-2\DLN:UAR-1;)"
    "\n"
    R"(B-2\NBS\N:1;)"
    "\n"
    R"(B-2\BID-1:00000000;)"
    "\n"
    R"(B-2\BNA-1:F1;)"
    "\n"
    R"(B-2\BT-1:1553;)"
    "\n"
    R"(V-1\HDS\SYS:sB1cTdTWHw20x0v0t+u+;)"
    "\n"
    R"(R-1\DSI-3:UAR-2;)"
    "\n"
    R"(R-1\TK1-3:3;)"
    "\n"
    R"(R-1\CHE-3:T;)"
    "\n"
    R"(R-1\CDT-3:1553IN;)"
    "\n"
    R"(R-1\TK4-3:3;)"
    "\n"
    R"(R-1\CDLN-3:UAR-2;)"
    "\n"
    R"(R-1\BTF-3:1;)"
    "\n"
    R"(B-3\DLN:UAR-2;)"
    "\n"
    R"(B-3\NBS\N:1;)"
    "\n"
    R"(R-1\ASN-2-8:8;)"
    "\n"
    R"(R-1\ASN-2-9:9;)"
    "\n"
    R"(R-1\ASN-2-10:10;)"
    "\n"
    R"(R-1\ASN-18-6:6;)"
    "\n"
    R"(R-1\ANM-18-6:EEC3B;)"
    "\n"
    R"(V-1\HDS\SYS:sR6WH;)"
    "\n"
    R"(R-1\ASN-18-7:7;)"
    "\n"
    R"(R-1\ANM-18-7:EEC4A;)"
    "\n"
    R"(V-1\HDS\SYS:sR7WH;)"
    "\n"
    R"(R-1\ASN-18-8:8;)"
    "\n"
    R"(R-1\ANM-18-8:EEC4B;)"
    "\n"
    R"(V-1\HDS\SYS:sR8WH;)"
    "\n"
    R"(R-1\DSI-19:ARR40-1-2;)"
    "\n"
    R"(R-1\TK1-19:19;)"
    "\n"
    R"(R-1\CHE-19:T;)"
    "\n"
    R"(R-1\CDT-19:429IN;)"
    "\n"
    R"(R-1\TK4-19:19;)"
    "\n"
    R"(R-1\CDLN-19:ARR40-1-2;)"
    "\n";

class TMATSParserTest : public ::testing::Test
{
   protected:
    TMATSParser parser_;
    std::vector<subattr_data_tuple> keys_;
    std::vector<subattr_data_tuple> values_;
    subattr_index_map kmap_;
    subattr_index_map vmap_;
    subattr_map submap_;
    std::string kdata_;
    std::string vdata_;
    subattr_index_map::const_iterator index_map_it_;

   public:
    TMATSParserTest() : parser_(TMATS, false), kdata_(""), vdata_("")
    {
    }

    bool GetIndexMaps(size_t key_index, size_t val_index)
    {
        if((key_index > (int(keys_.size()) - 1)) || (val_index > (int(values_.size()) - 1)))
            return false;
        kmap_ = std::get<0>(keys_.at(key_index));
        vmap_ = std::get<0>(values_.at(val_index)); 
        return true;
    }

    bool GetData(size_t key_index, size_t val_index)
    {
        if((key_index > (int(keys_.size()) - 1)) || (val_index > (int(values_.size()) - 1)))
            return false;
        kdata_ = std::get<1>(keys_.at(key_index));
        vdata_ = std::get<1>(values_.at(val_index));
        return true;
    }

    bool FindKeyInIndexMap(const subattr_index_map& index_map, std::string key)
    {
        index_map_it_ = index_map.find(key);
        if(index_map_it_ == index_map.cend())
            return false;
        return true; 
    }
};

TEST(CodeNameTest, RegexCorrect)
{
    CodeName c = CodeName(R"(R-x\DSI-n)", false);

    EXPECT_EQ("R-([0-9]+)\\\\DSI-([0-9]+)", c.regex_string);
}

TEST(CodeNameTest, RegexCorrectCompound)
{
    CodeName c = CodeName(R"(R-x\ASN-n-m)", false);

    EXPECT_EQ("R-([0-9]+)\\\\ASN-([0-9]+)-([0-9]+)", c.regex_string);
}

TEST(CodeNameTest, RegexCorrectSimpleValue)
{
    CodeName c = CodeName(R"(P-d\F1)", false);

    EXPECT_EQ("P-([0-9]+)\\\\F1", c.regex_string);
}

TEST_F(TMATSParserTest, ParseLinesUnilateralNoRegex)
{
    ASSERT_FALSE(parser_.ParseLines("G\\106", submap_));
}

TEST_F(TMATSParserTest, ParseLinesUnilateral)
{
    ASSERT_TRUE(parser_.ParseLines("P-d\\F1", submap_));

    // R"(P-1\F1:16;)"
    // R"(P-4\F1:20;)"
    ASSERT_EQ(2, submap_.size());
    ASSERT_TRUE(submap_.count(1) == 1);
    ASSERT_TRUE(submap_.count(4) == 1);
    ASSERT_EQ("16", submap_.at(1));
    ASSERT_EQ("20", submap_.at(4));
}

TEST_F(TMATSParserTest, MapUnilateralAttrs)
{
    // R"(P-1\F1:16;)"
    // R"(P-4\F1:20;)"
    // R"(P-4\DLN:pcm4;)"
    // R"(P-1\DLN:pcm1;)"
    // R"(P-1\D4:N;)"
    std::vector<std::string> attrs{"P-d\\F1", "P-d\\DLN",
        "P-d\\D4"};
    unilateral_map output;
    parser_.MapUnilateralAttrs(attrs, output);

    ASSERT_EQ(3, output.size());
    ASSERT_EQ(1, output.count(1));
    ASSERT_EQ(1, output.count(4));
    ASSERT_EQ(3, output.at(4).size());
    ASSERT_EQ(3, output.at(1).size());
    ASSERT_EQ("16", output.at(1).at("P-d\\F1"));
    ASSERT_EQ("pcm1", output.at(1).at("P-d\\DLN"));
    ASSERT_EQ("N", output.at(1).at("P-d\\D4"));
    ASSERT_EQ("20", output.at(4).at("P-d\\F1"));
    ASSERT_EQ("pcm4", output.at(4).at("P-d\\DLN"));
}

TEST_F(TMATSParserTest, ParseLinesSingleVar)
{
    ASSERT_TRUE(parser_.ParseLines("G\\DSI-x", "G\\DST-x", keys_, values_));
    ASSERT_TRUE(GetIndexMaps(0, 0));
    ASSERT_EQ(1, kmap_.size());
    ASSERT_EQ(1, vmap_.size());
    ASSERT_TRUE(FindKeyInIndexMap(kmap_, "x"));
    ASSERT_TRUE(FindKeyInIndexMap(vmap_, "x"));
    EXPECT_EQ(kmap_.at("x"), vmap_.at("x"));
    EXPECT_EQ(1, kmap_.at("x"));
    ASSERT_TRUE(GetData(0, 0));
    EXPECT_EQ("DATASOURCE", kdata_);
    EXPECT_EQ("OTH", vdata_);
}

TEST_F(TMATSParserTest, ParseLinesNoVar)
{
    ASSERT_FALSE(parser_.ParseLines("G\\DSI\\N", "R-x\\TK1-n", keys_, values_));
}

TEST_F(TMATSParserTest, ParseLinesTwoVar)
{
    ASSERT_TRUE(parser_.ParseLines("R-x\\DSI-n", "R-x\\TK1-n", keys_, values_));
    ASSERT_EQ(4, keys_.size());
    ASSERT_EQ(4, values_.size());
    ASSERT_TRUE(GetIndexMaps(0, 0));
    ASSERT_EQ(2, kmap_.size());
    ASSERT_EQ(2, vmap_.size());
    ASSERT_TRUE(FindKeyInIndexMap(kmap_, "x"));
    ASSERT_TRUE(FindKeyInIndexMap(vmap_, "x"));
    ASSERT_TRUE(FindKeyInIndexMap(kmap_, "n"));
    ASSERT_TRUE(FindKeyInIndexMap(vmap_, "n"));

    // R-1\DSI-3:UAR-2;
    // R-1\TK1-3:3;
    ASSERT_TRUE(GetIndexMaps(2, 2));
    ASSERT_TRUE(GetData(2, 2));
    EXPECT_EQ(3, kmap_.at("n"));
    EXPECT_EQ(3, vmap_.at("n"));
    EXPECT_EQ("UAR-2", kdata_);
    EXPECT_EQ("3", vdata_);
}

TEST_F(TMATSParserTest, ParseLinesThreeVar)
{
    ASSERT_TRUE(parser_.ParseLines("R-x\\ASN-n-m", "R-x\\ANM-n-m", keys_, values_));
    ASSERT_EQ(6, keys_.size());
    ASSERT_EQ(3, values_.size());
    ASSERT_TRUE(GetIndexMaps(0, 0));
    ASSERT_EQ(3, kmap_.size());
    ASSERT_EQ(3, vmap_.size());
    ASSERT_TRUE(FindKeyInIndexMap(kmap_, "x"));
    ASSERT_TRUE(FindKeyInIndexMap(vmap_, "x"));
    ASSERT_TRUE(FindKeyInIndexMap(kmap_, "n"));
    ASSERT_TRUE(FindKeyInIndexMap(vmap_, "n"));
    ASSERT_TRUE(FindKeyInIndexMap(kmap_, "m"));
    ASSERT_TRUE(FindKeyInIndexMap(vmap_, "m"));

    // R-1\ASN-18-8:8;
    // R-1\ANM-18-8:EEC4B;
    ASSERT_TRUE(GetIndexMaps(5, 2));
    ASSERT_TRUE(GetData(5, 2));
    EXPECT_EQ(18, kmap_.at("n"));
    EXPECT_EQ(18, vmap_.at("n"));
    EXPECT_EQ(8, kmap_.at("m"));
    EXPECT_EQ(8, vmap_.at("m"));
    EXPECT_EQ("8", kdata_);
    EXPECT_EQ("EEC4B", vdata_);
}

TEST_F(TMATSParserTest, CheckVarCountOneVar)
{
    kmap_["n"] = 1;
    subattr_data_tuple temp_tuple(kmap_, "data");
    keys_.push_back(temp_tuple);
    subattr_data_tuple temp_tuple2(kmap_, "other");
    keys_.push_back(temp_tuple2);

    EXPECT_FALSE(parser_.CheckVarCount(keys_, 2));
    EXPECT_TRUE(parser_.CheckVarCount(keys_, 1));

    kmap_["x"] = 1;
    subattr_data_tuple temp_tuple3(kmap_, "bad");
    keys_.push_back(temp_tuple3);
    EXPECT_FALSE(parser_.CheckVarCount(keys_, 2));
    EXPECT_FALSE(parser_.CheckVarCount(keys_, 1));
}

TEST_F(TMATSParserTest, CheckVarCountTwoVar)
{
    kmap_["n"] = 4;
    kmap_["m"] = 17;
    subattr_data_tuple temp_tuple(kmap_, "data");
    keys_.push_back(temp_tuple);
    subattr_data_tuple temp_tuple2(kmap_, "other");
    keys_.push_back(temp_tuple2);

    EXPECT_TRUE(parser_.CheckVarCount(keys_, 2));
    EXPECT_FALSE(parser_.CheckVarCount(keys_, 1));

    kmap_["x"] = 1;
    subattr_data_tuple temp_tuple3(kmap_, "bad");
    keys_.push_back(temp_tuple3);
    EXPECT_FALSE(parser_.CheckVarCount(keys_, 2));
    EXPECT_FALSE(parser_.CheckVarCount(keys_, 3));
}

TEST_F(TMATSParserTest, MapKeyToValueOneToOne)
{
    kmap_["x"] = 4;
    subattr_data_tuple dt0(kmap_, "10");
    kmap_["x"] = 14;
    subattr_data_tuple dt1(kmap_, "12");
    keys_.push_back(dt0);
    keys_.push_back(dt1);

    vmap_["x"] = 14;
    subattr_data_tuple dt2(vmap_, "12");
    vmap_["x"] = 4;
    subattr_data_tuple dt3(vmap_, "10");
    values_.push_back(dt2);
    values_.push_back(dt3);

    std::map<std::string, std::string> expected = {
        {"10", "10"},
        {"12", "12"}
    };
    std::map<std::string, std::string> result;
    EXPECT_TRUE(parser_.MapKeyToValue(keys_, values_, result));
    EXPECT_THAT(expected, ::testing::ContainerEq(result));
}

TEST_F(TMATSParserTest, MapKeyToValueOneToOneIncomplete)
{
    kmap_["x"] = 4;
    subattr_data_tuple dt0(kmap_, "10");
    kmap_["x"] = 14;
    subattr_data_tuple dt1(kmap_, "12");
    keys_.push_back(dt0);
    keys_.push_back(dt1);

    // 13 is not one of the x values of the keys map
    vmap_["x"] = 13;
    subattr_data_tuple dt2(vmap_, "11");
    vmap_["x"] = 4;
    subattr_data_tuple dt3(vmap_, "10");
    values_.push_back(dt2);
    values_.push_back(dt3);

    std::map<std::string, std::string> result;
    std::map<std::string, std::string> expected = {
        {"10", "10"}
    };
    EXPECT_TRUE(parser_.MapKeyToValue(keys_, values_, result));
    EXPECT_THAT(expected, ::testing::ContainerEq(result));
}

TEST_F(TMATSParserTest, MapKeyToValueOneToOneVarMismatch)
{
    // Two variables not permitted for 1-to-1
    kmap_["x"] = 4;
    kmap_["n"] = 2;
    subattr_data_tuple dt0(kmap_, "10");
    kmap_["x"] = 14;
    subattr_data_tuple dt1(kmap_, "12");
    keys_.push_back(dt0);
    keys_.push_back(dt1);

    // 13 is not one of the x values of the keys map
    vmap_["x"] = 14;
    subattr_data_tuple dt2(vmap_, "11");
    vmap_["x"] = 4;
    subattr_data_tuple dt3(vmap_, "10");
    values_.push_back(dt2);
    values_.push_back(dt3);

    std::map<std::string, std::string> result;
    EXPECT_FALSE(parser_.MapKeyToValue(keys_, values_, result));
    EXPECT_TRUE(result.size() == 0);
}

TEST_F(TMATSParserTest, MapKeyToValueTwoToTwo)
{
    // Two variables not permitted for 1-to-1
    kmap_["x"] = 4;
    kmap_["n"] = 2;
    subattr_data_tuple dt0(kmap_, "10");
    kmap_["x"] = 14;
    kmap_["n"] = 7;
    subattr_data_tuple dt1(kmap_, "12");
    keys_.push_back(dt0);
    keys_.push_back(dt1);

    vmap_["x"] = 14;
    vmap_["n"] = 7;
    subattr_data_tuple dt2(vmap_, "11");
    vmap_["x"] = 4;
    vmap_["n"] = 2;
    subattr_data_tuple dt3(vmap_, "data");
    values_.push_back(dt2);
    values_.push_back(dt3);

    std::map<std::string, std::string> expected = {
        {"10", "data"},
        {"12", "11"}
    };

    std::map<std::string, std::string> result;
    EXPECT_TRUE(parser_.MapKeyToValue(keys_, values_, result));
    EXPECT_THAT(expected, ::testing::ContainerEq(result));
}

TEST_F(TMATSParserTest, MapKeyToValueOneToMany)
{
    kmap_["x"] = 1;
    kmap_["n"] = 3;
    subattr_data_tuple dt0(kmap_, "data0");
    kmap_["x"] = 1;
    kmap_["n"] = 15;
    subattr_data_tuple dt1(kmap_, "data1");
    keys_.push_back(dt0);
    keys_.push_back(dt1);

    vmap_["x"] = 1;
    vmap_["n"] = 15;
    vmap_["m"] = 100;
    subattr_data_tuple dt2(vmap_, "label1a");
    vmap_["m"] = 101;
    subattr_data_tuple dt4(vmap_, "label1b");
    vmap_["x"] = 1;
    vmap_["n"] = 3; 
    vmap_["m"] = 23;
    subattr_data_tuple dt3(vmap_, "label0a");
    values_.push_back(dt2);
    values_.push_back(dt3);
    values_.push_back(dt4);

    std::map<std::string, std::vector<std::string>> expected = {
        {"data0", {"label0a"}},
        {"data1", {"label1a", "label1b"}}
    };
    std::map<std::string, std::vector<std::string>> result;
    EXPECT_TRUE(parser_.MapKeyToValue(keys_, values_, result));
    EXPECT_THAT(expected, ::testing::ContainerEq(result));
}

TEST_F(TMATSParserTest, MapKeyToValueOneToManySingle)
{
    kmap_["x"] = 1;
    subattr_data_tuple dt0(kmap_, "data0");
    kmap_["x"] = 2;
    subattr_data_tuple dt1(kmap_, "data1");
    keys_.push_back(dt0);
    keys_.push_back(dt1);

    vmap_["x"] = 2;
    vmap_["n"] = 15;
    subattr_data_tuple dt2(vmap_, "label1a");
    vmap_["n"] = 101;
    subattr_data_tuple dt4(vmap_, "label1b");
    vmap_["x"] = 1;
    vmap_["n"] = 3; 
    subattr_data_tuple dt3(vmap_, "label0a");
    values_.push_back(dt2);
    values_.push_back(dt3);
    values_.push_back(dt4);

    std::map<std::string, std::vector<std::string>> expected = {
        {"data0", {"label0a"}},
        {"data1", {"label1a", "label1b"}}
    };
    std::map<std::string, std::vector<std::string>> result;
    EXPECT_TRUE(parser_.MapKeyToValue(keys_, values_, result));
    EXPECT_THAT(expected, ::testing::ContainerEq(result));
}

TEST_F(TMATSParserTest, MapKeyToValueOneToMaps)
{
    kmap_["x"] = 1;
    kmap_["n"] = 3;
    subattr_data_tuple dt0(kmap_, "data0");
    kmap_["x"] = 1;
    kmap_["n"] = 15;
    subattr_data_tuple dt1(kmap_, "data1");
    keys_.push_back(dt0);
    keys_.push_back(dt1);

    vmap_["x"] = 1;
    vmap_["n"] = 15;
    vmap_["m"] = 100;
    subattr_data_tuple dt2(vmap_, "label1a");
    vmap_["m"] = 101;
    subattr_data_tuple dt4(vmap_, "label1b");
    vmap_["x"] = 1;
    vmap_["n"] = 3; 
    vmap_["m"] = 23;
    subattr_data_tuple dt3(vmap_, "label0a");
    values_.push_back(dt2);
    values_.push_back(dt3);
    values_.push_back(dt4);

    std::map<std::string, std::map<std::string, std::string>> expected = {
        {"data0", {{"23", "label0a"}}},
        {"data1", {{"100", "label1a"}, {"101", "label1b"}}}
    };
    std::map<std::string, std::map<std::string, std::string>> result;
    EXPECT_TRUE(parser_.MapKeyToValue(keys_, values_, result));
    EXPECT_THAT(expected, ::testing::ContainerEq(result));
}

TEST_F(TMATSParserTest, MapKeyToValueOneToMapsSingle)
{
    kmap_["x"] = 1;
    subattr_data_tuple dt0(kmap_, "data0");
    kmap_["x"] = 2;
    subattr_data_tuple dt1(kmap_, "data1");
    keys_.push_back(dt0);
    keys_.push_back(dt1);

    vmap_["x"] = 2;
    vmap_["n"] = 15;
    subattr_data_tuple dt2(vmap_, "label1a");
    vmap_["n"] = 101;
    subattr_data_tuple dt4(vmap_, "label1b");
    vmap_["x"] = 1;
    vmap_["n"] = 3; 
    subattr_data_tuple dt3(vmap_, "label0a");
    values_.push_back(dt2);
    values_.push_back(dt3);
    values_.push_back(dt4);

    std::map<std::string, std::map<std::string, std::string>> expected = {
        {"data0", {{"3", "label0a"}}},
        {"data1", {{"15", "label1a"}, {"101", "label1b"}}}
    };
    std::map<std::string, std::map<std::string, std::string>> result;
    EXPECT_TRUE(parser_.MapKeyToValue(keys_, values_, result));
    EXPECT_THAT(expected, ::testing::ContainerEq(result));
}

TEST_F(TMATSParserTest, MapKeyToValueOneToManyInvalidVarCount)
{
    kmap_["x"] = 1;
    kmap_["n"] = 3;
    subattr_data_tuple dt0(kmap_, "data0");
    kmap_["x"] = 1;
    kmap_["n"] = 15;
    subattr_data_tuple dt1(kmap_, "data1");
    keys_.push_back(dt0);
    keys_.push_back(dt1);

    // vmap must have count + 1 vars
    vmap_["x"] = 1;
    vmap_["n"] = 15;
    subattr_data_tuple dt2(vmap_, "label1a");
    vmap_["x"] = 1;
    vmap_["n"] = 3; 
    subattr_data_tuple dt3(vmap_, "label0a");
    values_.push_back(dt2);
    values_.push_back(dt3);

    std::map<std::string, std::vector<std::string>> result;
    EXPECT_FALSE(parser_.MapKeyToValue(keys_, values_, result));
}

TEST_F(TMATSParserTest, GetIndependentVarName)
{
    kmap_["x"] = 1;
    kmap_["n"] = 30;
    subattr_data_tuple dt0(kmap_, "data0");
    keys_.push_back(dt0);

    vmap_["x"] = 1;
    vmap_["n"] = 15;
    vmap_["m"] = 33;
    subattr_data_tuple dt2(vmap_, "label1a");
    values_.push_back(dt2);

    ASSERT_EQ("m", parser_.GetIndependentVarName(keys_, values_));
}

TEST_F(TMATSParserTest, CheckVarCountEqualityEmptyVec)
{
    vmap_["x"] = 4;
    subattr_data_tuple dt2(vmap_, "11");
    values_.push_back(dt2);

    // keys_ is empty.
    ASSERT_FALSE(parser_.CheckVarCountEquality(keys_, values_));

    kmap_["x"] = 4;
    subattr_data_tuple dt0(kmap_, "10");
    keys_.push_back(dt0);
    values_.clear();

    // values_ is empty
    ASSERT_FALSE(parser_.CheckVarCountEquality(keys_, values_));
}

TEST_F(TMATSParserTest, CheckVarCountEqualityKeysNotConsistent)
{
    kmap_["x"] = 4;
    subattr_data_tuple dt0(kmap_, "10");

    // First tuple has map with one var
    keys_.push_back(dt0);

    kmap_["n"] = 1;
    subattr_data_tuple dt1(kmap_, "data");

    // Second tuple has map with two vars
    keys_.push_back(dt1);

    vmap_["x"] = 4;
    subattr_data_tuple dt2(vmap_, "11");
    values_.push_back(dt2);

    ASSERT_FALSE(parser_.CheckVarCountEquality(keys_, values_));
}

TEST_F(TMATSParserTest, CheckVarCountEqualityValuesNotEqual)
{
    kmap_["x"] = 4;
    subattr_data_tuple dt0(kmap_, "10");
    keys_.push_back(dt0);
    kmap_["x"] = 7;
    subattr_data_tuple dt1(kmap_, "data");
    keys_.push_back(dt1);

    // two vars 
    vmap_["x"] = 4;
    vmap_["n"] = 5;
    subattr_data_tuple dt2(vmap_, "11");
    values_.push_back(dt2);

    ASSERT_FALSE(parser_.CheckVarCountEquality(keys_, values_));
}

TEST_F(TMATSParserTest, CheckVarCountEquality)
{
    kmap_["x"] = 4;
    kmap_["n"] = 10;
    subattr_data_tuple dt0(kmap_, "10");
    keys_.push_back(dt0);
    kmap_["x"] = 7;
    kmap_["n"] = 11;
    subattr_data_tuple dt1(kmap_, "data");
    keys_.push_back(dt1);

    // two vars 
    vmap_["x"] = 4;
    vmap_["n"] = 5;
    subattr_data_tuple dt2(vmap_, "11");
    values_.push_back(dt2);

    ASSERT_TRUE(parser_.CheckVarCountEquality(keys_, values_));
}

TEST_F(TMATSParserTest, CheckVarCountEqualityUseAdjustment)
{
    kmap_["x"] = 4;
    kmap_["n"] = 10;
    subattr_data_tuple dt0(kmap_, "10");
    keys_.push_back(dt0);
    kmap_["x"] = 7;
    kmap_["n"] = 11;
    subattr_data_tuple dt1(kmap_, "data");
    keys_.push_back(dt1);

    // two vars 
    vmap_["x"] = 4;
    vmap_["n"] = 5;
    vmap_["m"] = 3;
    subattr_data_tuple dt2(vmap_, "11");
    values_.push_back(dt2);

    ASSERT_TRUE(parser_.CheckVarCountEquality(keys_, values_, 1));
}

TEST_F(TMATSParserTest, MapAttrsOneToMany)
{
    std::map<std::string, std::vector<std::string>> result;
    std::map<std::string, std::vector<std::string>> expected = {
        {"2", {"8", "9", "10"}}
    };

    ASSERT_TRUE(parser_.MapAttrs("R-x\\TK1-n", "R-x\\ASN-n-m", result));
    EXPECT_EQ(expected, result);
}

TEST_F(TMATSParserTest, MapAttrsOneToMaps)
{
    std::map<std::string, std::map<std::string, std::string>> result;
    std::map<std::string, std::map<std::string, std::string>> expected = {
        {"2", {{"8", "8"}, {"9", "9"}, {"10", "10"}}}
    };

    ASSERT_TRUE(parser_.MapAttrs("R-x\\TK1-n", "R-x\\ASN-n-m", result));
    EXPECT_EQ(expected, result);
}

TEST_F(TMATSParserTest, MapAttrsMatchesGroups)
{
    std::map<std::string, std::string> expected;
    std::map<std::string, std::string> result;
    expected["ARR40-1-2"] = "19";
    expected["Time"] = "1";
    expected["UAR-1"] = "2";
    expected["UAR-2"] = "3";

    ASSERT_TRUE(parser_.MapAttrs("R-x\\DSI-n", "R-x\\TK1-n", result));
    EXPECT_EQ(expected, result);
}

TEST_F(TMATSParserTest, MapAttrs3Subattrs)
{
    std::map<std::string, std::string> expected;
    std::map<std::string, std::string> result;
    expected["6"] = "EEC3B";
    expected["7"] = "EEC4A";
    expected["8"] = "EEC4B";

    ASSERT_TRUE(parser_.MapAttrs("R-x\\ASN-n-m", "R-x\\ANM-n-m", result));
    EXPECT_EQ(expected, result);
}

TEST_F(TMATSParserTest, MapAttrsHandleRepeatData)
{
    // Note about creating test lines from a tmats text file:
    // 1) Read in with Python via with open(...), a = f.readlines();
    // 2) b = ['R\"(' + x.rstrip('\n') + ')\" \"\\r\\n\"\n' for x in a]
    // 3) Write to temp output file via with open(...), f.writelines(b)
    // 4) paste into C++ file
    //
    std::string custom_tmats =
        R"(G\COM:********************** Time Channel ************************;)"
        "\r\n"
        R"(G\COM:************************************************************;)"
        "\r\n"
        R"(R-1\CDT-1:TIMEIN;)"
        "\r\n"
        R"(R-1\TK1-1:1;)"
        "\r\n"
        R"()"
        "\r\n"
        R"(G\COM:************************************************************;)"
        "\r\n"
        R"(G\COM:********************* Video Channels ***********************;)"
        "\r\n"
        R"(G\COM:************************************************************;)"
        "\r\n"
        R"()"
        "\r\n"
        R"(R-1\DSI-2:VIDEO_1;)"
        "\r\n"
        R"(R-1\CDT-2:VIDIN;)"
        "\r\n"
        R"(R-1\TK1-2:2;)"
        "\r\n"
        R"()"
        "\r\n"
        R"(R-1\DSI-3:VIDEO_2;)"
        "\r\n"
        R"(R-1\CDT-3:VIDIN;)"
        "\r\n"
        R"(R-1\TK1-3:3;)"
        "\r\n"
        R"()"
        "\r\n"
        R"(R-1\DSI-4:VIDEO_3;)"
        "\r\n"
        R"(R-1\CDT-4:VIDIN;)"
        "\r\n"
        R"(R-1\TK1-4:4;)"
        "\r\n"
        R"()"
        "\r\n"
        R"(R-1\DSI-5:VIDEO_4;)"
        "\r\n"
        R"(R-1\CDT-5:VIDIN;)"
        "\r\n"
        R"(R-1\TK1-5:5;)"
        "\r\n"
        R"()"
        "\r\n"
        R"(G\COM:************************************************************;)"
        "\r\n"
        R"(G\COM:***************** MIL-STD-1553B Channels *******************;)"
        "\r\n"
        R"(G\COM:************************************************************;)"
        "\r\n"
        R"(R-1\DSI-6:1553_1;)"
        "\r\n"
        R"(R-1\CDT-6:1553IN;)"
        "\r\n"
        R"(R-1\TK1-6:6;)"
        "\r\n"
        R"()"
        "\r\n"
        R"(R-1\DSI-7:1553_2;)"
        "\r\n"
        R"(R-1\CDT-7:1553IN;)"
        "\r\n"
        R"(R-1\TK1-7:7;)"
        "\r\n"
        R"()"
        "\r\n"
        R"(R-1\DSI-8:1553_3;)"
        "\r\n"
        R"(R-1\CDT-8:1553IN;)"
        "\r\n"
        R"(R-1\TK1-8:8;)"
        "\r\n"
        R"()"
        "\r\n"
        R"(R-1\DSI-9:1553_4;)"
        "\r\n"
        R"(R-1\CDT-9:1553IN;)"
        "\r\n"
        R"(R-1\TK1-9:9;)"
        "\r\n";

    std::map<std::string, std::string> expected = {
        {"1", "TIMEIN"},
        {"2", "VIDIN"},
        {"3", "VIDIN"},
        {"4", "VIDIN"},
        {"5", "VIDIN"},
        {"6", "1553IN"},
        {"7", "1553IN"},
        {"8", "1553IN"},
        {"9", "1553IN"}};
    std::map<std::string, std::string> result;

    TMATSParser parser = TMATSParser(custom_tmats, false);
    ASSERT_TRUE(parser.MapAttrs("R-x\\TK1-n", "R-x\\CDT-n", result));
    EXPECT_EQ(expected, result);
}
