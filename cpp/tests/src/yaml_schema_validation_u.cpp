#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "yaml_schema_validation.h"

class YamlSchemaValidationTest : public ::testing::Test
{
protected:

	bool res_;
	YAML::Node schema_node_;
	YAML::Node test_node_;
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
	res_ = ysv_.Validate(test_node_, schema_node_, log_items_);
	EXPECT_FALSE(res_);

	// Log output 
	EXPECT_EQ(log_items_.size(), 2);
}