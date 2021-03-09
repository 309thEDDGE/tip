#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "yaml_schema_validation.h"

class YamlSchemaValidationTest : public ::testing::Test
{
protected:

	bool res_;
	YAML::Node schema_node_;
	YAML::Node test_node_;
	YAML::Node user_schema_node_;
	std::vector<LogItem> log_items_;
	YamlSV ysv_;
	LogLevel level_;
	std::string msg_;

	YamlSchemaValidationTest() : res_(false), ysv_() {}
};

TEST_F(YamlSchemaValidationTest, AddLogItem)
{
	level_ = LogLevel::WARN;
	msg_ = "this message";
	ysv_.AddLogItem(log_items_, level_, msg_);

	EXPECT_EQ(log_items_.size(), 1);
	EXPECT_EQ(log_items_[0].log_level, level_);
	EXPECT_EQ(log_items_[0].message, msg_);
}

TEST_F(YamlSchemaValidationTest, ValidateEmptyNodes)
{
	// Nodes are undefined/empty.
	res_ = ysv_.Validate(test_node_, user_schema_node_, log_items_);
	EXPECT_FALSE(res_);

	// Log output 
	EXPECT_EQ(log_items_.size(), 2);
}

//TEST_F(YamlSchemaValidationTest, MakeSchemaNodeOutputNotEmpty)
//{
//	// Nodes are undefined/empty.
//	res_ = ysv_.MakeSchemaNode(schema_node_, user_schema_node_, log_items_);
//	EXPECT_FALSE(res_);
//
//	// Log output 
//	EXPECT_EQ(log_items_.size(), 1);
//}

TEST_F(YamlSchemaValidationTest, VerifyTypeNotAType)
{
	std::string str_type = "ABC";
	std::string test_val = "data";
	res_ = ysv_.VerifyType(str_type, test_val);
	EXPECT_FALSE(res_);
}

TEST_F(YamlSchemaValidationTest, VerifyTypeString)
{
	std::string str_type = "STR";
	std::string test_val = "data";
	res_ = ysv_.VerifyType(str_type, test_val);
	EXPECT_TRUE(res_);

	test_val = "39.9";
	res_ = ysv_.VerifyType(str_type, test_val);
	EXPECT_TRUE(res_);
}

TEST_F(YamlSchemaValidationTest, VerifyTypeInt)
{
	std::string str_type = "INT";
	std::string test_val = "data";
	res_ = ysv_.VerifyType(str_type, test_val);
	EXPECT_FALSE(res_);

	test_val = "39.9";
	res_ = ysv_.VerifyType(str_type, test_val);
	EXPECT_FALSE(res_);

	test_val = "39";
	res_ = ysv_.VerifyType(str_type, test_val);
	EXPECT_TRUE(res_);
}

TEST_F(YamlSchemaValidationTest, VerifyTypeFloat)
{
	std::string str_type = "FLT";
	std::string test_val = "data";
	res_ = ysv_.VerifyType(str_type, test_val);
	EXPECT_FALSE(res_);

	test_val = "39.9";
	res_ = ysv_.VerifyType(str_type, test_val);
	EXPECT_TRUE(res_);

	test_val = "39";
	res_ = ysv_.VerifyType(str_type, test_val);
	EXPECT_TRUE(res_);
}

TEST_F(YamlSchemaValidationTest, VerifyTypeBool)
{
	std::string str_type = "FLT";
	std::string test_val = "data";
	res_ = ysv_.VerifyType(str_type, test_val);
	EXPECT_FALSE(res_);

	test_val = "39.9";
	res_ = ysv_.VerifyType(str_type, test_val);
	EXPECT_TRUE(res_);

	test_val = "39";
	res_ = ysv_.VerifyType(str_type, test_val);
	EXPECT_TRUE(res_);
}

TEST_F(YamlSchemaValidationTest, ProcessNodeSingleMappedValue)
{
	schema_node_ = YAML::Load("data: STR\n");
	test_node_ = YAML::Load("data: test\n");

	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_TRUE(res_);
	EXPECT_TRUE(schema_node_.IsMap());
	EXPECT_EQ(log_items_.size(), 0);
}