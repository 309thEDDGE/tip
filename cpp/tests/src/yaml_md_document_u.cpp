#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <sstream>
#include "yaml_md_document.h"
#include "md_category_sequence.h"
#include "md_category_scalar.h"
#include "md_category_map.h"

class YamlMDDocumentTest : public ::testing::Test
{
   protected:
    YamlMDDocument md_;
    std::string expected_md_string_;
    std::shared_ptr<MDCategoryScalar> cat_scalar_;
    std::shared_ptr<MDCategoryMap> cat_map_;
    std::shared_ptr<MDCategorySequence> cat_seq_;

    YamlMDDocumentTest() : md_(), expected_md_string_(""), cat_scalar_(nullptr),
        cat_map_(nullptr), cat_seq_(nullptr)
    {
    }
};

TEST_F(YamlMDDocumentTest, EmitScalarCategory)
{
    cat_scalar_ = std::make_shared<MDCategoryScalar>("type");
    cat_scalar_->SetScalarValue("data");
    md_.EmitCategory(cat_scalar_);
    expected_md_string_ = 
        "---\n"
        "type: data\n"
        "...\n";
    EXPECT_EQ(expected_md_string_, md_.GetMetadataString());
}

TEST_F(YamlMDDocumentTest, EmitMapCategory)
{
    cat_map_ = std::make_shared<MDCategoryMap>("name");
    std::vector<std::string> keys{"one", "two"};
    cat_map_->SetMapKeys(keys);
    cat_map_->SetMappedValue(keys.at(0), 1);
    cat_map_->SetMappedValue(keys.at(1), 2);

    md_.EmitCategory(cat_map_);
    expected_md_string_ = 
        "---\n"
        "name:\n"
        "  one: 1\n"
        "  two: 2\n"
        "...\n";
    EXPECT_EQ(expected_md_string_, md_.GetMetadataString());
}

TEST_F(YamlMDDocumentTest, EmitSequenceCategory)
{
    cat_seq_ = std::make_shared<MDCategorySequence>("type");
    cat_seq_->Insert("something");
    md_.EmitCategory(cat_seq_);
    expected_md_string_ = 
        "---\n"
        "type:\n"
        "  - something\n"
        "...\n";
    EXPECT_EQ(expected_md_string_, md_.GetMetadataString());
}

TEST_F(YamlMDDocumentTest, CreateDocumentFromCategories)
{
    cat_seq_ = std::make_shared<MDCategorySequence>("list");
    cat_seq_->Insert("something");
    cat_seq_->Insert("another thing");

    cat_map_ = std::make_shared<MDCategoryMap>("name");
    std::vector<std::string> keys{"one", "two"};
    cat_map_->SetMapKeys(keys);
    cat_map_->SetMappedValue(keys.at(0), 1);
    cat_map_->SetMappedValue(keys.at(1), 2);

    cat_scalar_ = std::make_shared<MDCategoryScalar>("type");
    cat_scalar_->SetScalarValue("data");
    
    md_.AddCategory(cat_scalar_);
    md_.AddCategory(cat_map_);
    md_.AddCategory(cat_seq_);
    md_.CreateDocument();

    expected_md_string_ = 
        "---\n"
        "type: data\n"
        "name:\n"
        "  one: 1\n"
        "  two: 2\n"
        "list:\n"
        "  - something\n"
        "  - another thing\n"
        "...\n";
    EXPECT_EQ(expected_md_string_, md_.GetMetadataString());
}

TEST_F(YamlMDDocumentTest, ReadDocumentTopLevelMustBeMap)
{
    // Document is a sequence at the top level
    std::string md_string = 
        "- 1\n"
        "- 2\n";

    EXPECT_FALSE(md_.ReadDocument(md_string));
}

TEST_F(YamlMDDocumentTest, ExtractTopLevelKeys)
{
    std::string md_string = 
        "data: zero\n"
        "other:\n"
        "  uno: tree\n"
        "  dos: house\n"
        "const:\n"
        "  - alpha\n"
        "  - beta\n";

    std::vector<std::string> expected_keys{"data", "other", "const"};
    YAML::Node node = YAML::Load(md_string);
    ASSERT_TRUE(node.IsMap());
    std::vector<std::string> keys;
    EXPECT_TRUE(md_.ExtractTopLevelKeys(node, keys));
    EXPECT_THAT(expected_keys, ::testing::UnorderedElementsAreArray(keys));
}

TEST_F(YamlMDDocumentTest, ReadDocumentNondefinedCategory)
{
    cat_seq_ = std::make_shared<MDCategorySequence>("list");
    cat_map_ = std::make_shared<MDCategoryMap>("name");
    cat_scalar_ = std::make_shared<MDCategoryScalar>("type");
    
    md_.AddCategory(cat_scalar_);
    md_.AddCategory(cat_map_);
    md_.AddCategory(cat_seq_);

    std::string md_string = 
        "---\n"
        "type: data\n"
        "name:\n"
        "  one: 1\n"
        "  two: 2\n"
        "list:\n"
        "  - something\n"
        "  - another thing\n"
        "extra: some_more_here\n"
        "...\n";

    EXPECT_TRUE(md_.ReadDocument(md_string));
    EXPECT_EQ("data", cat_scalar_->node.as<std::string>());
    ASSERT_EQ(2, cat_map_->node.size());
    EXPECT_EQ(1, cat_map_->node["one"].as<int>());
    EXPECT_EQ(2, cat_map_->node["two"].as<int>());
    ASSERT_EQ(2, cat_seq_->node.size());
    EXPECT_EQ("something", cat_seq_->node[0].as<std::string>());
    EXPECT_EQ("another thing", cat_seq_->node[1].as<std::string>());
}

// TEST_F(YamlMDDocumentTest, ReadDocumentExtraCategory)
// {
//     cat_seq_ = std::make_shared<MDCategorySequence>("list");
//     cat_map_ = std::make_shared<MDCategoryMap>("name");
//     cat_scalar_ = std::make_shared<MDCategoryScalar>("type");
//     std::shared_ptr<MDCategoryMap> cat_map2 = std::make_shared<MDCategoryMap>("a_map");
    
//     md_.AddCategory(cat_scalar_);
//     md_.AddCategory(cat_map_);
//     md_.AddCategory(cat_seq_);
//     md_.AddCategory(cat_map2);

//     std::string md_string = 
//         "---\n"
//         "type: data\n"
//         "name:\n"
//         "  one: 1\n"
//         "  two: 2\n"
//         "list:\n"
//         "  - something\n"
//         "  - another thing\n"
//         "...\n";

//     EXPECT_TRUE(md_.ReadDocument(md_string));
//     EXPECT_EQ("data", cat_scalar_->node.as<std::string>());
//     ASSERT_EQ(2, cat_map_->node.size());
//     EXPECT_EQ(1, cat_map_->node["one"].as<int>());
//     EXPECT_EQ(2, cat_map_->node["two"].as<int>());
//     ASSERT_EQ(2, cat_seq_->node.size());
//     EXPECT_EQ("something", cat_seq_->node[0].as<std::string>());
//     EXPECT_EQ("another thing", cat_seq_->node[1].as<std::string>());
//     EXPECT_EQ(0, cat_map2->node.size());
// }

TEST_F(YamlMDDocumentTest, SetCategoryValueFromInputNodeTypeMismatch)
{
    // Set as sequence type.
    YAML::Node input_node = YAML::Load("[]");
    cat_scalar_ = std::make_shared<MDCategoryScalar>("scalar_type");
    EXPECT_FALSE(md_.SetCategoryValueFromInputNode(cat_scalar_, input_node));

    cat_seq_ = std::make_shared<MDCategorySequence>("seq_type");
    EXPECT_TRUE(md_.SetCategoryValueFromInputNode(cat_seq_, input_node));
}

TEST_F(YamlMDDocumentTest, SetCategoryValueFromInputNode)
{
    // Set as sequence type.
    YAML::Node input_node = YAML::Load("[]");
    input_node.push_back(50);
    input_node.push_back(621);
    cat_seq_ = std::make_shared<MDCategorySequence>("seq_type");

    ASSERT_TRUE(md_.SetCategoryValueFromInputNode(cat_seq_, input_node));
    ASSERT_TRUE(cat_seq_->node.size() == 2);
    EXPECT_EQ(50, cat_seq_->node[0].as<int>());
    EXPECT_EQ(621, cat_seq_->node[1].as<int>());
}