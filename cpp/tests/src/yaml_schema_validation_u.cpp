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

TEST_F(YamlSchemaValidationTest, AddLogItemFormatted)
{
	level_ = LogLevel::WARN;
	char buff[100];
	int val = 230;
	msg_ = "this message %d";
	snprintf(buff, 100, msg_.c_str(), val);
	std::string final_msg(buff);

	ysv_.AddLogItem(log_items_, level_, msg_.c_str(), val);

	EXPECT_EQ(log_items_.size(), 1);
	EXPECT_EQ(log_items_[0].log_level, level_);
	EXPECT_EQ(log_items_[0].message, final_msg);
}

TEST_F(YamlSchemaValidationTest, ValidateEmptyNodes)
{
	// Nodes are undefined/empty.
	res_ = ysv_.Validate(test_node_, user_schema_node_, log_items_);
	EXPECT_FALSE(res_);

	// Log output 
	EXPECT_EQ(log_items_.size(), 2);
}

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
	std::string str_type = "BOOL";
	std::string test_val = "data";
	res_ = ysv_.VerifyType(str_type, test_val);
	EXPECT_FALSE(res_);

	test_val = "True";
	res_ = ysv_.VerifyType(str_type, test_val);
	EXPECT_TRUE(res_);

	test_val = "true";
	res_ = ysv_.VerifyType(str_type, test_val);
	EXPECT_FALSE(res_);

	test_val = "False";
	res_ = ysv_.VerifyType(str_type, test_val);
	EXPECT_TRUE(res_);

	test_val = "false";
	res_ = ysv_.VerifyType(str_type, test_val);
	EXPECT_FALSE(res_);
}

TEST_F(YamlSchemaValidationTest, ProcessNodeSingleMappedValue)
{
	schema_node_ = YAML::Load("data: STR\n");
	test_node_ = YAML::Load("data: test\n");

	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_TRUE(res_);
	EXPECT_EQ(log_items_.size(), 0);
}

TEST_F(YamlSchemaValidationTest, ProcessNodeSingleMultipleMappedValue)
{
	schema_node_ = YAML::Load("data: STR\n"
							  "dog: INT\n"
							  "time: FLT\n"
							  "state: BOOL\n");
	test_node_ = YAML::Load("data: test\n"
						    "dog: 9\n"
						    "time: 23.4\n"
						    "state: True\n");

	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_TRUE(res_);
	EXPECT_EQ(log_items_.size(), 0);

	test_node_["dog"] = 9.8;
	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_FALSE(res_);
	EXPECT_EQ(log_items_.size(), 1);
	EXPECT_TRUE(log_items_[0].message.find("dog") != std::string::npos);

	log_items_.clear();
	test_node_["dog"] = 50;
	test_node_["time"] = "thirty";
	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_FALSE(res_);
	EXPECT_EQ(log_items_.size(), 1);
	EXPECT_TRUE(log_items_[0].message.find("time") != std::string::npos);

	log_items_.clear();
	test_node_["time"] = 19;
	test_node_["state"] = "false";
	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_FALSE(res_);
	EXPECT_EQ(log_items_.size(), 1);
	EXPECT_TRUE(log_items_[0].message.find("state") != std::string::npos);
}

TEST_F(YamlSchemaValidationTest, ProcessNodeMissingKey)
{
	schema_node_ = YAML::Load(
		"data: STR\n"
		"dog: INT\n"
		"time: FLT\n"
		"state: BOOL\n");
	test_node_ = YAML::Load(
		"data: test\n"
		"time: 23.4\n"
		"state: True\n");

	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_FALSE(res_);
	EXPECT_EQ(log_items_.size(), 1);
	EXPECT_TRUE(log_items_[0].message.find("dog") != std::string::npos);
}

TEST_F(YamlSchemaValidationTest, ProcessNodeIncorrectStructure)
{
	schema_node_ = YAML::Load(
		"data: STR\n"
		"time: FLT\n"
		"state: BOOL\n");
	test_node_ = YAML::Load(
		"data: test\n"
		"time: [23.4, 99.9]\n"
		"state: True\n");

	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_FALSE(res_);
	EXPECT_EQ(log_items_.size(), 1);
	EXPECT_TRUE(log_items_[0].message.find("time") != std::string::npos);

	test_node_ = YAML::Load(
		"data: {test: 10, other: 30}\n"
		"time: 23.4\n"
		"state: True\n");
	log_items_.clear();
	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_FALSE(res_);
	EXPECT_EQ(log_items_.size(), 1);
	EXPECT_TRUE(log_items_[0].message.find("data") != std::string::npos);

	test_node_ = YAML::Load(
		"data: test\n"
		"time: 23.4\n"
		"state:\n"
		"  d1: 9\n"
		"  d2: 10\n");
	log_items_.clear();
	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_FALSE(res_);
	EXPECT_EQ(log_items_.size(), 1);
	EXPECT_TRUE(log_items_[0].message.find("state") != std::string::npos);
}

TEST_F(YamlSchemaValidationTest, ProcessNodeNestedMap1)
{
	schema_node_ = YAML::Load(
		"data: STR\n"
		"dog: INT\n"
		"time:\n"
		"  state: BOOL\n");
	test_node_ = YAML::Load(
		"data: test\n"
		"dog: 9\n"
		"time:\n"
		"  state: True\n");

	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_TRUE(res_);
	EXPECT_EQ(log_items_.size(), 0);

	test_node_["dog"] = 9.8;
	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_FALSE(res_);
	EXPECT_EQ(log_items_.size(), 1);
	EXPECT_TRUE(log_items_[0].message.find("dog") != std::string::npos);

	log_items_.clear();
	test_node_["dog"] = 50;
	test_node_["time"]["state"] = "true";
	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_FALSE(res_);
	EXPECT_EQ(log_items_.size(), 1);
	EXPECT_TRUE(log_items_[0].message.find("state") != std::string::npos);
}

TEST_F(YamlSchemaValidationTest, ProcessNodeNestedMap2)
{
	schema_node_ = YAML::Load(
		"data: STR\n"
		"dog: INT\n"
		"time:\n"
		"  state: BOOL\n"
		"  meridian: STR\n"
		"  day: INT\n");
	test_node_ = YAML::Load(
		"data: test\n"
		"dog: 9\n"
		"time:\n"
		"  state: True\n"
		"  day: 12\n");

	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_FALSE(res_);
	EXPECT_EQ(log_items_.size(), 1);
	EXPECT_TRUE(log_items_[0].message.find("meridian") != std::string::npos);
}

TEST_F(YamlSchemaValidationTest, ProcessNodeNestedMap3)
{
	schema_node_ = YAML::Load(
		"data: STR\n"
		"dog: INT\n"
		"time:\n"
		"  state: BOOL\n"
		"  meridian:\n"
		"    day: INT\n"
		"    year: INT\n"
		"  err: FLT\n");
	test_node_ = YAML::Load(
		"data: test\n"
		"dog: 9\n"
		"time:\n"
		"  state: True\n"
		"  meridian:\n"
		"    day: 12\n"
		"    year: 2021\n"
		"  err: 6.3\n");

	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_TRUE(res_);
	EXPECT_EQ(log_items_.size(), 0);

	test_node_["time"]["meridian"]["day"] = "Tuesday";
	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_FALSE(res_);
	EXPECT_EQ(log_items_.size(), 1);
	EXPECT_TRUE(log_items_[0].message.find("day") != std::string::npos);

	log_items_.clear();
	test_node_["time"]["meridian"]["day"] = 27;
	test_node_["time"]["err"] = "no";
	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_FALSE(res_);
	EXPECT_EQ(log_items_.size(), 1);
	EXPECT_TRUE(log_items_[0].message.find("err") != std::string::npos);
}

TEST_F(YamlSchemaValidationTest, ProcessNodeHandleMapNotDefined)
{
	schema_node_ = YAML::Load(
		"data: STR\n"
		"_NOT_DEFINED_: INT\n"
		"val: FLT\n");
	test_node_ = YAML::Load(
		"data: test\n"
		"dog: 9\n"
		"val: 23.0\n");

	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_TRUE(res_);
	EXPECT_EQ(log_items_.size(), 0);
}

TEST_F(YamlSchemaValidationTest, ProcessNodeHandleMapNotDefinedRepeating)
{
	schema_node_ = YAML::Load(
		"data: STR\n"
		"_NOT_DEFINED_: INT\n"
		"val: FLT\n");
	test_node_ = YAML::Load(
		"data: test\n"
		"dog: 9\n"
		"abc: 12\n"
		"def: 993\n"
		"val: 23.0\n");

	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_TRUE(res_);
	EXPECT_EQ(log_items_.size(), 0);

	test_node_["abc"] = "hello";
	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_FALSE(res_);
	EXPECT_EQ(log_items_.size(), 1);
	EXPECT_TRUE(log_items_[0].message.find("abc") != std::string::npos);
}
