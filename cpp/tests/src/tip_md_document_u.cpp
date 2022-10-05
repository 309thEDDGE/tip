#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <sstream>
#include "tip_md_document.h"
#include "md_category_sequence.h"
#include "md_category_scalar.h"
#include "md_category_map.h"

class TIPMDDocumentTest : public ::testing::Test
{
   protected:
    TIPMDDocument md_;
    std::string expected_md_string_;
    std::shared_ptr<MDCategoryScalar> cat_scalar_;
    std::shared_ptr<MDCategoryMap> cat_map_;
    std::shared_ptr<MDCategorySequence> cat_seq_;

    TIPMDDocumentTest() : md_(), expected_md_string_(""), cat_scalar_(nullptr),
        cat_map_(nullptr), cat_seq_(nullptr)
    {
    }
};

TEST_F(TIPMDDocumentTest, AddResource)
{
    md_.AddResource("test_type", "/the/path", "abdedfg123");
    ASSERT_TRUE(md_.prov_category_->node.IsMap());
    ASSERT_TRUE(md_.prov_category_->node["resource"].IsSequence());
    ASSERT_TRUE(md_.prov_category_->node["resource"].size() == 1);
    EXPECT_EQ("test_type", md_.prov_category_->node["resource"][0]["type"].as<std::string>());
    EXPECT_EQ("/the/path", md_.prov_category_->node["resource"][0]["path"].as<std::string>());
    EXPECT_EQ("abdedfg123", md_.prov_category_->node["resource"][0]["uid"].as<std::string>());
}

TEST_F(TIPMDDocumentTest, AddResourceMultiple)
{
    std::string type = "typeA";
    std::string path = "/path/A";
    std::string uid = "uid00";
    md_.AddResource(type, path, uid);

    type = "typeB";
    path = "/path/B";
    uid = "uid01";
    md_.AddResource(type, path, uid);

    ASSERT_TRUE(md_.prov_category_->node.IsMap());
    ASSERT_TRUE(md_.prov_category_->node["resource"].IsSequence());
    ASSERT_TRUE(md_.prov_category_->node["resource"].size() == 2);
    EXPECT_EQ("typeA", md_.prov_category_->node["resource"][0]["type"].as<std::string>());
    EXPECT_EQ("typeB", md_.prov_category_->node["resource"][1]["type"].as<std::string>());
    EXPECT_EQ(uid, md_.prov_category_->node["resource"][1]["uid"].as<std::string>());
}


TEST_F(TIPMDDocumentTest, CreateDocument)
{
    md_.type_category_->SetScalarValue("parsed_data_v0");
    md_.uid_category_->SetScalarValue("adfB43");
    std::string time("2021-11-30");
    md_.prov_category_->SetMappedValue("time", time);
    std::string version("v00.12");
    md_.prov_category_->SetMappedValue("version", version);
    md_.AddResource("input_file", "/path/to/input", "bhaY10");
    YAML::Node runtime_node;
    runtime_node["runtime_data"].push_back(10);
    runtime_node["runtime_data"].push_back(22);
    runtime_node.SetStyle(YAML::EmitterStyle::Block);
    md_.runtime_category_->SetValue(runtime_node);

    md_.CreateDocument();
    expected_md_string_ =
        "---\n"
        "type: parsed_data_v0\n"
        "uid: adfB43\n"
        "provenance:\n"
        "  time: 2021-11-30\n"
        "  version: v00.12\n"
        "  resource:\n"
        "    - type: input_file\n"
        "      path: /path/to/input\n"
        "      uid: bhaY10\n"
        "config:\n"
        "  {}\n"
        "runtime:\n"
        "  runtime_data:\n"
        "    - 10\n"
        "    - 22\n"
        "...\n";
    EXPECT_EQ(expected_md_string_, md_.GetMetadataString());
}

TEST_F(TIPMDDocumentTest, AddResourcesEmptyResources)
{
    TIPMDDocument input_doc;
    ASSERT_FALSE(md_.AddResources(input_doc));
    ASSERT_FALSE(md_.prov_category_->node["resource"].IsSequence());
}

TEST_F(TIPMDDocumentTest, AddResourcesSingleResource)
{
    TIPMDDocument input_doc;
    std::string type = "mytype";
    std::string path = "the/path";
    std::string uid = "abcde123";
    input_doc.AddResource(type, path, uid);
    ASSERT_TRUE(md_.AddResources(input_doc));
    ASSERT_TRUE(md_.prov_category_->node["resource"].IsSequence());
    ASSERT_EQ(md_.prov_category_->node["resource"].size(), 1);
    ASSERT_TRUE(md_.prov_category_->node["resource"][0].IsMap());
    EXPECT_EQ(path, md_.prov_category_->node["resource"][0]["path"].as<std::string>());
}

TEST_F(TIPMDDocumentTest, AddResourcesWithResourcePresent)
{
    TIPMDDocument input_doc;
    std::string type = "mytype";
    std::string path = "the/path";
    std::string uid = "abcde123";
    input_doc.AddResource(type, path, uid);
    type = "anothertype";
    path = "a/b/c";
    uid = "GHatY";
    input_doc.AddResource(type, path, uid);

    type = "origtype";
    path = "/mnt/data";
    uid = "000111";
    md_.AddResource(type, path, uid);

    ASSERT_TRUE(md_.AddResources(input_doc));
    ASSERT_TRUE(md_.prov_category_->node["resource"].IsSequence());
    ASSERT_EQ(md_.prov_category_->node["resource"].size(), 3);
    ASSERT_TRUE(md_.prov_category_->node["resource"][0].IsMap());
    ASSERT_TRUE(md_.prov_category_->node["resource"][1].IsMap());
    ASSERT_TRUE(md_.prov_category_->node["resource"][2].IsMap());
    EXPECT_EQ(path, md_.prov_category_->node["resource"][0]["path"].as<std::string>());
    EXPECT_EQ("GHatY", md_.prov_category_->node["resource"][2]["uid"].as<std::string>());
}