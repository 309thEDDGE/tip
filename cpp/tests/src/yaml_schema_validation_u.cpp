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
	std::vector<LogItem>::const_iterator log_it_;

	YamlSchemaValidationTest() : res_(false), ysv_()  {}

	void PrintLogs()
	{
		for (log_it_ = log_items_.begin(); log_it_ != log_items_.end(); ++log_it_)
		{
			log_it_->Print();
		}
	}

	LogItem GetFirstInfoLogEntry()
	{
		for (log_it_ = log_items_.begin(); log_it_ != log_items_.end(); ++log_it_)
		{
			if (log_it_->log_level == LogLevel::LLINFO)
				return *log_it_;
		}
		LogItem li;
		return li;
	}

	bool InFirstInfoEntry(std::string search)
	{
		return GetFirstInfoLogEntry().message.find(search) != std::string::npos;
	}

	int InfoLogEntryCount()
	{
		int count = 0;
		for (log_it_ = log_items_.begin(); log_it_ != log_items_.end(); ++log_it_)
		{
			if (log_it_->log_level == LogLevel::LLINFO)
				count++;
		}
		return count;
	}
};

TEST_F(YamlSchemaValidationTest, AddLogItem)
{
	level_ = LogLevel::LLINFO;
	msg_ = "this message";
	ysv_.AddLogItem(log_items_, level_, msg_);

	EXPECT_EQ(InfoLogEntryCount(), 1);
	EXPECT_EQ(log_items_[0].log_level, level_);
	EXPECT_EQ(log_items_[0].message, msg_);
}

TEST_F(YamlSchemaValidationTest, AddLogItemFormatted)
{
	level_ = LogLevel::LLINFO;
	char buff[100];
	int val = 230;
	msg_ = "this message %d";
	snprintf(buff, 100, msg_.c_str(), val);
	std::string final_msg(buff);

	ysv_.AddLogItem(log_items_, level_, msg_.c_str(), val);

	EXPECT_EQ(InfoLogEntryCount(), 1);
	EXPECT_EQ(log_items_[0].log_level, level_);
	EXPECT_EQ(log_items_[0].message, final_msg);
}

TEST_F(YamlSchemaValidationTest, ValidateEmptyNodes)
{
	// Nodes are undefined/empty.
	res_ = ysv_.Validate(test_node_, user_schema_node_, log_items_);
	EXPECT_FALSE(res_);

	// Log output 
	EXPECT_EQ(log_items_.size(), 1);
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
	EXPECT_TRUE(res_);

	test_val = "False";
	res_ = ysv_.VerifyType(str_type, test_val);
	EXPECT_TRUE(res_);

	test_val = "false";
	res_ = ysv_.VerifyType(str_type, test_val);
	EXPECT_TRUE(res_);
}

TEST_F(YamlSchemaValidationTest, ProcessNodeSingleMappedValue)
{
	schema_node_ = YAML::Load("data: STR\n");
	test_node_ = YAML::Load("data: test\n");

	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_TRUE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 0);
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
	EXPECT_EQ(InfoLogEntryCount(), 0);

	test_node_["dog"] = 9.8;
	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_FALSE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 1);
	//EXPECT_TRUE(log_items_[0].message.find("dog") != std::string::npos);
	EXPECT_TRUE(InFirstInfoEntry("dog"));

	log_items_.clear();
	test_node_["dog"] = 50;
	test_node_["time"] = "thirty";
	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_FALSE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 1);
	//EXPECT_TRUE(log_items_[0].message.find("time") != std::string::npos);
	EXPECT_TRUE(InFirstInfoEntry("time"));

	log_items_.clear();
	test_node_["time"] = 19;
	test_node_["state"] = "fals";
	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_FALSE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 1);
	//EXPECT_TRUE(log_items_[0].message.find("state") != std::string::npos);
	EXPECT_TRUE(InFirstInfoEntry("state"));
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
	EXPECT_EQ(InfoLogEntryCount(), 1);
	//EXPECT_TRUE(log_items_[0].message.find("dog") != std::string::npos);
	EXPECT_TRUE(InFirstInfoEntry("dog"));
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
	EXPECT_EQ(InfoLogEntryCount(), 1);
	//EXPECT_TRUE(log_items_[0].message.find("time") != std::string::npos);
	EXPECT_TRUE(InFirstInfoEntry("time"));

	test_node_ = YAML::Load(
		"data: {test: 10, other: 30}\n"
		"time: 23.4\n"
		"state: True\n");
	log_items_.clear();
	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_FALSE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 1);
	//EXPECT_TRUE(log_items_[0].message.find("data") != std::string::npos);
	EXPECT_TRUE(InFirstInfoEntry("data"));

	test_node_ = YAML::Load(
		"data: test\n"
		"time: 23.4\n"
		"state:\n"
		"  d1: 9\n"
		"  d2: 10\n");
	log_items_.clear();
	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_FALSE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 1);
	//EXPECT_TRUE(log_items_[0].message.find("state") != std::string::npos);
	EXPECT_TRUE(InFirstInfoEntry("state"));
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
	EXPECT_EQ(InfoLogEntryCount(), 0);

	test_node_["dog"] = 9.8;
	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_FALSE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 1);
	//EXPECT_TRUE(log_items_[0].message.find("dog") != std::string::npos);
	EXPECT_TRUE(InFirstInfoEntry("dog"));

	log_items_.clear();
	test_node_["dog"] = 50;
	test_node_["time"]["state"] = "tru";
	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_FALSE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 1);
	//EXPECT_TRUE(log_items_[0].message.find("state") != std::string::npos);
	EXPECT_TRUE(InFirstInfoEntry("state"));
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
	EXPECT_EQ(InfoLogEntryCount(), 1);
	//EXPECT_TRUE(log_items_[0].message.find("meridian") != std::string::npos);
	EXPECT_TRUE(InFirstInfoEntry("meridian"));
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
	EXPECT_EQ(InfoLogEntryCount(), 0);

	test_node_["time"]["meridian"]["day"] = "Tuesday";
	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_FALSE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 1);
	//EXPECT_TRUE(log_items_[0].message.find("day") != std::string::npos);
	EXPECT_TRUE(InFirstInfoEntry("day"));

	log_items_.clear();
	test_node_["time"]["meridian"]["day"] = 27;
	test_node_["time"]["err"] = "no";
	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_FALSE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 1);
	//EXPECT_TRUE(log_items_[0].message.find("err") != std::string::npos);
	EXPECT_TRUE(InFirstInfoEntry("err"));
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
	EXPECT_EQ(InfoLogEntryCount(), 0);
}

TEST_F(YamlSchemaValidationTest, ProcessNodeHandleMapNotDefinedAtEnd)
{
	schema_node_ = YAML::Load(
		"data: STR\n"
		"_NOT_DEFINED_: INT\n");
	test_node_ = YAML::Load(
		"data: test\n"
		"dog: 9\n");

	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_TRUE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 0);
}

TEST_F(YamlSchemaValidationTest, ProcessNodeHandleMapNotDefinedRepeating1)
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
	EXPECT_EQ(InfoLogEntryCount(), 0);

	test_node_["abc"] = "hello";
	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_FALSE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 1);
	//EXPECT_TRUE(log_items_[0].message.find("abc") != std::string::npos);
	EXPECT_TRUE(InFirstInfoEntry("abc"));
}

TEST_F(YamlSchemaValidationTest, ProcessNodeTestMapTruncated)
{
	schema_node_ = YAML::Load(
		"data: STR\n"
		"dog: INT\n"
		"val: FLT\n");
	test_node_ = YAML::Load(
		"data: test\n"
		"dog: 9\n");

	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_FALSE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 1);
}

TEST_F(YamlSchemaValidationTest, ProcessNodeHandleMapNotDefinedRepeating2)
{
	schema_node_ = YAML::Load(
		"data: STR\n"
		"_NOT_DEFINED_:\n"
		"  val: FLT\n"
		"  trio: INT\n"
		"status: BOOL\n");
	test_node_ = YAML::Load(
		"data: test\n"
		"dog:\n"
		"  val: 12.3\n"
		"  trio: 993\n"
		"one:\n"
		"  val: 1.3\n"
		"  trio: 9\n"
		"day:\n"
		"  val: 883.6\n"
		"  trio: 33\n"
		"status: True\n");

	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_TRUE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 0);

	test_node_["one"]["val"] = "hello";
	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_FALSE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 1);
	//EXPECT_TRUE(log_items_[0].message.find("val") != std::string::npos);
	EXPECT_TRUE(InFirstInfoEntry("val"));

	log_items_.clear();
	test_node_["one"]["val"] = 20.2;
	test_node_["day"]["trio"] = 0.2;
	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_FALSE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 1);
	//EXPECT_TRUE(log_items_[0].message.find("trio") != std::string::npos);
	EXPECT_TRUE(InFirstInfoEntry("trio"));

	log_items_.clear();
	test_node_["day"]["trio"] = 2;
	test_node_["status"] = "tru";
	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_FALSE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 1);
	//EXPECT_TRUE(log_items_[0].message.find("status") != std::string::npos);
	EXPECT_TRUE(InFirstInfoEntry("status"));
}

TEST_F(YamlSchemaValidationTest, TestSequenceSimpleBlock)
{
	schema_node_ = YAML::Load(
		"- INT\n"
	);
	test_node_ = YAML::Load(
		"- 3\n"
	);

	res_ = ysv_.TestSequence(schema_node_, test_node_, log_items_);
	EXPECT_TRUE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 0);

	test_node_.push_back(10);
	test_node_.push_back(12);
	res_ = ysv_.TestSequence(schema_node_, test_node_, log_items_);
	EXPECT_TRUE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 0);

	test_node_.push_back("dog");
	res_ = ysv_.TestSequence(schema_node_, test_node_, log_items_);
	EXPECT_FALSE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 1);
	//EXPECT_TRUE(log_items_[0].message.find("dog") != std::string::npos);
	EXPECT_TRUE(InFirstInfoEntry("dog"));
}

TEST_F(YamlSchemaValidationTest, TestSequenceSchemaMustHaveSingleElement)
{
	schema_node_ = YAML::Load(
		"- INT\n"
		"- FLT\n"
	);
	test_node_ = YAML::Load(
		"- 3\n"
	);

	res_ = ysv_.TestSequence(schema_node_, test_node_, log_items_);
	EXPECT_FALSE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 1);
	//EXPECT_TRUE(log_items_[0].message.find("Schema") != std::string::npos);
	EXPECT_TRUE(InFirstInfoEntry("Schema"));
}

TEST_F(YamlSchemaValidationTest, TestSequenceSimpleFlow)
{
	schema_node_ = YAML::Load(
		"[BOOL]\n"
	);
	test_node_ = YAML::Load(
		"[True, False, False]\n"
	);

	res_ = ysv_.TestSequence(schema_node_, test_node_, log_items_);
	EXPECT_TRUE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 0);

	test_node_[2] = "negative";
	res_ = ysv_.TestSequence(schema_node_, test_node_, log_items_);
	EXPECT_FALSE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 1);
	//EXPECT_TRUE(log_items_[0].message.find("false") != std::string::npos);
	EXPECT_TRUE(InFirstInfoEntry("negative"));
}

TEST_F(YamlSchemaValidationTest, TestSequenceNullSchema)
{
	schema_node_ = YAML::Load(
		"[]\n"
	);
	test_node_ = YAML::Load(
		"[True, False, False]\n"
	);

	res_ = ysv_.TestSequence(schema_node_, test_node_, log_items_);
	EXPECT_FALSE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 1);
	//EXPECT_TRUE(log_items_[0].message.find("Schema") != std::string::npos);
	EXPECT_TRUE(InFirstInfoEntry("Schema"));
}

TEST_F(YamlSchemaValidationTest, TestSequenceNullTest)
{
	schema_node_ = YAML::Load(
		"[BOOL]\n"
	);
	test_node_ = YAML::Load(
		"[]\n"
	);

	res_ = ysv_.TestSequence(schema_node_, test_node_, log_items_);
	EXPECT_FALSE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 1);
	//EXPECT_TRUE(log_items_[0].message.find("Test") != std::string::npos);
	EXPECT_TRUE(InFirstInfoEntry("Test"));
}

TEST_F(YamlSchemaValidationTest, ProcessNodeSimpleSequence)
{
	schema_node_ = YAML::Load("[FLT]\n");
	test_node_ = YAML::Load("[20.3, 3400.1]\n");

	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_TRUE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 0);

	test_node_.push_back("err");
	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_FALSE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 1);
	//EXPECT_TRUE(log_items_[0].message.find("err") != std::string::npos);
	EXPECT_TRUE(InFirstInfoEntry("err"));
}

TEST_F(YamlSchemaValidationTest, ProcessNodeNestedSequence1)
{
	schema_node_ = YAML::Load(
		"- [INT]\n"
		"- [BOOL]\n");
	test_node_ = YAML::Load(
		"- [20, 3400, 10]\n"
		"- [True, False, True]\n");

	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_TRUE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 0);
}

TEST_F(YamlSchemaValidationTest, ProcessNodeEmptySchemaSequence)
{
	schema_node_ = YAML::Load(
		"-\n");
	test_node_ = YAML::Load(
		"- [20, 3400, 10]\n");

	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_FALSE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 1);
	//EXPECT_TRUE(log_items_[0].message.find("null") != std::string::npos);
	EXPECT_TRUE(InFirstInfoEntry("null"));
}

TEST_F(YamlSchemaValidationTest, ProcessNodeEmptyTestSequence)
{
	schema_node_ = YAML::Load(
		"- [INT]\n");
	test_node_ = YAML::Load(
		"- []\n");

	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_FALSE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 1);
	//EXPECT_TRUE(log_items_[0].message.find("zero") != std::string::npos);
	EXPECT_TRUE(InFirstInfoEntry("zero"));
}

TEST_F(YamlSchemaValidationTest, ProcessNodeNestedSequence2)
{
	schema_node_ = YAML::Load(
		"- [INT]\n"
		"-\n"
		"  - [FLT]\n"
		"  - [BOOL]\n"
		"- [BOOL]\n");
	test_node_ = YAML::Load(
		"- [20, 3400, 10]\n"
		"-\n"
		"  - [19.3, 19.8]\n"
		"  - [True]\n"
		"- [True, False, True]\n");

	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_TRUE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 0);

	test_node_[0][1] = "badval";
	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_FALSE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 1);
	//EXPECT_TRUE(log_items_[0].message.find("badval") != std::string::npos);
	EXPECT_TRUE(InFirstInfoEntry("badval"));

	log_items_.clear();
	test_node_[0][1] = "3511";
	test_node_[1][0][0] = "badval";
	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_FALSE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 1);
	//EXPECT_TRUE(log_items_[0].message.find("badval") != std::string::npos);
	EXPECT_TRUE(InFirstInfoEntry("badval"));

	log_items_.clear();
	test_node_[1][0][0] = "54.4";
	test_node_[1][1][0] = "positive";
	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_FALSE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 1);
	//EXPECT_TRUE(log_items_[0].message.find("true") != std::string::npos);
	EXPECT_TRUE(InFirstInfoEntry("positive"));

	log_items_.clear();
	test_node_[1][1][0] = "False";
	test_node_[2][1] = "negative";
	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_FALSE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 1);
	//EXPECT_TRUE(log_items_[0].message.find("false") != std::string::npos);
	EXPECT_TRUE(InFirstInfoEntry("negative"));
}

TEST_F(YamlSchemaValidationTest, ProcessNodeMapAndSequence1)
{
	schema_node_ = YAML::Load(
		"m1: INT\n"
		"m2: [FLT]\n"
		"m3:\n"
		"  - mm1: BOOL\n"
		"  - mm2: [BOOL]\n"
		"m4: INT\n");
	test_node_ = YAML::Load(
		"m1: 23\n"
		"m2:\n"
		"  - 2.0\n"
		"  - 9.2\n"
		"m3:\n"
		"  - mm1: True\n"
		"  - mm2: [True, False]\n"
		"m4: 1002\n");

	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_TRUE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 0);

	test_node_["m1"] = "err";
	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_FALSE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 1);
	//EXPECT_TRUE(log_items_[0].message.find("m1") != std::string::npos);
	EXPECT_TRUE(InFirstInfoEntry("m1"));

	log_items_.clear();
	test_node_["m1"] = "21";
	test_node_["m2"][0] = "bad";
	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_FALSE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 1);
	//EXPECT_TRUE(log_items_[0].message.find("bad") != std::string::npos);
	EXPECT_TRUE(InFirstInfoEntry("bad"));

	log_items_.clear();
	test_node_["m2"][0] = "33.11";
	test_node_["m3"][1]["mm2"][0] = "pos";
	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_FALSE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 1);
	//EXPECT_TRUE(log_items_[0].message.find("true") != std::string::npos);
	EXPECT_TRUE(InFirstInfoEntry("pos"));

	log_items_.clear();
	test_node_["m3"][1]["mm2"][0] = "True";
	test_node_["m4"] = "str";
	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_FALSE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 1);
	//EXPECT_TRUE(log_items_[0].message.find("m4") != std::string::npos);
	EXPECT_TRUE(InFirstInfoEntry("m4"));
}

TEST_F(YamlSchemaValidationTest, ProcessNodeMapAndSequenceNotDefined)
{
	schema_node_ = YAML::Load(
		"m1: INT\n"
		"_NOT_DEFINED_: [FLT]\n"
		"m3: BOOL\n");
	test_node_ = YAML::Load(
		"m1: 23\n"
		"m2:\n"
		"  - 2.0\n"
		"  - 9.2\n"
		"mm2: [32, 100, 31.1]\n"
		"mm3: [99.8]\n"
		"m3: True\n");

	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_TRUE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 0);

	test_node_["m2"] = "err";
	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_FALSE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 1);
	//EXPECT_TRUE(log_items_[0].message.find("err") != std::string::npos);
	EXPECT_TRUE(InFirstInfoEntry("err"));

	log_items_.clear();
	test_node_["m2"] = YAML::Load("[2.0, 9.2]");
	test_node_["mm2"][0] = "notflt";
	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_FALSE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 1);
	//EXPECT_TRUE(log_items_[0].message.find("notflt") != std::string::npos);
	EXPECT_TRUE(InFirstInfoEntry("notflt"));

	log_items_.clear();
	test_node_["mm2"][0] = "1.0";
	test_node_["mm3"][0] = "data";
	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_FALSE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 1);
	//EXPECT_TRUE(log_items_[0].message.find("data") != std::string::npos);
	EXPECT_TRUE(InFirstInfoEntry("data"));

	log_items_.clear();
	test_node_["mm3"][0] = "25.1";
	test_node_["m3"] = "affirmed";
	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_FALSE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 1);
	//EXPECT_TRUE(log_items_[0].message.find("m3") != std::string::npos);
	EXPECT_TRUE(InFirstInfoEntry("m3"));
}

TEST_F(YamlSchemaValidationTest, ProcessNodeHandleMapNotDefinedOptional)
{
	schema_node_ = YAML::Load(
		"data: STR\n"
		"_NOT_DEFINED_OPT_: INT\n"
		"val: FLT\n");
	test_node_ = YAML::Load(
		"data: test\n"
		"val: 23.0\n");

	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_TRUE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 0);

	test_node_ = YAML::Load(
		"data: test\n"
		"dog: 9\n"
		"val: 23.0\n");
	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_TRUE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 0);

	test_node_["dog"] = "woof";
	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_FALSE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 1);
	EXPECT_TRUE(InFirstInfoEntry("dog"));
}

TEST_F(YamlSchemaValidationTest, ProcessNodeHandleMapNotDefinedOptional2)
{
	schema_node_ = YAML::Load(
		"data: STR\n"
		"m1: {_NOT_DEFINED_OPT_: BOOL}\n"
		"val: FLT\n");
	test_node_ = YAML::Load(
		"data: test\n"
		"m1: {}\n"
		"val: 23.0\n");

	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_TRUE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 0);
}

TEST_F(YamlSchemaValidationTest, CheckDataTypeStringRawTypes)
{
	std::string test_type = "INT";
	std::string str_type;
	bool is_opt;

	res_ = ysv_.CheckDataTypeString(test_type, str_type, is_opt);
	EXPECT_TRUE(res_);
	EXPECT_EQ(str_type, test_type);
	EXPECT_FALSE(is_opt);

	test_type = "FLT";
	res_ = ysv_.CheckDataTypeString(test_type, str_type, is_opt);
	EXPECT_TRUE(res_);
	EXPECT_EQ(str_type, test_type);
	EXPECT_FALSE(is_opt);

	test_type = "BOOL";
	res_ = ysv_.CheckDataTypeString(test_type, str_type, is_opt);
	EXPECT_TRUE(res_);
	EXPECT_EQ(str_type, test_type);
	EXPECT_FALSE(is_opt);

	test_type = "STR";
	res_ = ysv_.CheckDataTypeString(test_type, str_type, is_opt);
	EXPECT_TRUE(res_);
	EXPECT_EQ(str_type, test_type);
	EXPECT_FALSE(is_opt);

	test_type = "IN";
	res_ = ysv_.CheckDataTypeString(test_type, str_type, is_opt);
	EXPECT_FALSE(res_);
	EXPECT_EQ(str_type, "");
	EXPECT_FALSE(is_opt);
}

TEST_F(YamlSchemaValidationTest, CheckDataTypeStringOptModifier)
{
	std::string test_type = "OPTINT";
	std::string str_type;
	bool is_opt;

	res_ = ysv_.CheckDataTypeString(test_type, str_type, is_opt);
	EXPECT_TRUE(res_);
	EXPECT_EQ(str_type, "INT");
	EXPECT_TRUE(is_opt);

	test_type = "OPTFLT";
	res_ = ysv_.CheckDataTypeString(test_type, str_type, is_opt);
	EXPECT_TRUE(res_);
	EXPECT_EQ(str_type, "FLT");
	EXPECT_TRUE(is_opt);

	test_type = "OPTBOOL";
	res_ = ysv_.CheckDataTypeString(test_type, str_type, is_opt);
	EXPECT_TRUE(res_);
	EXPECT_EQ(str_type, "BOOL");
	EXPECT_TRUE(is_opt);

	test_type = "OPTSTR";
	res_ = ysv_.CheckDataTypeString(test_type, str_type, is_opt);
	EXPECT_TRUE(res_);
	EXPECT_EQ(str_type, "STR");
	EXPECT_TRUE(is_opt);

	test_type = "OPTIN";
	res_ = ysv_.CheckDataTypeString(test_type, str_type, is_opt);
	EXPECT_FALSE(res_);
	EXPECT_EQ(str_type, "");
	EXPECT_FALSE(is_opt);

	test_type = "OPBOOL";
	res_ = ysv_.CheckDataTypeString(test_type, str_type, is_opt);
	EXPECT_FALSE(res_);
	EXPECT_EQ(str_type, "");
	EXPECT_FALSE(is_opt);
}

TEST_F(YamlSchemaValidationTest, TestSequenceOptBool)
{
	schema_node_ = YAML::Load("[BOOL]\n");
	test_node_ = YAML::Load("[]\n");

	res_ = ysv_.TestSequence(schema_node_, test_node_, log_items_);
	EXPECT_FALSE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 1);
	EXPECT_TRUE(InFirstInfoEntry("greater than"));

	log_items_.clear();
	schema_node_[0] = "OPTBOOL";
	res_ = ysv_.TestSequence(schema_node_, test_node_, log_items_);
	EXPECT_TRUE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 0);
}

TEST_F(YamlSchemaValidationTest, TestSequenceOpt)
{
	schema_node_ = YAML::Load("[OPTINT]\n");
	test_node_ = YAML::Load("[]\n");

	res_ = ysv_.TestSequence(schema_node_, test_node_, log_items_);
	EXPECT_TRUE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 0);

	schema_node_[0] = "OPTSTR";
	res_ = ysv_.TestSequence(schema_node_, test_node_, log_items_);
	EXPECT_TRUE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 0);

	schema_node_[0] = "OPTFLT";
	res_ = ysv_.TestSequence(schema_node_, test_node_, log_items_);
	EXPECT_TRUE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 0);

	test_node_ = YAML::Load("[23.2, 321]\n");
	res_ = ysv_.TestSequence(schema_node_, test_node_, log_items_);
	EXPECT_TRUE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 0);
}


TEST_F(YamlSchemaValidationTest, ProcessNodeOptBool)
{
	schema_node_ = YAML::Load(
		"data: [BOOL]\n");
	test_node_ = YAML::Load(
		"data: []\n");

	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_FALSE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 1);

	log_items_.clear();
	schema_node_["data"][0] = "OPTBOOL";
	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_TRUE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 0);
}

TEST_F(YamlSchemaValidationTest, ProcessNodeSequenceOptGeneral)
{
	schema_node_ = YAML::Load(
		"data: BOOL\n"
		"m1:\n"
		"  item: [FLT]\n"
		"  it2: [OPTINT]\n"
		"val:\n"
		"  - OPTSTR\n");
	test_node_ = YAML::Load(
		"data: true\n"
		"m1:\n"
		"  item: [19.2, 3.2]\n"
		"  it2: [100]\n"
		"val: [data1, data2, data3]\n");

	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_TRUE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 0);

	test_node_["m1"]["it2"] = YAML::Load("[]\n");
	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_TRUE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 0);

	test_node_["m1"]["it2"][0] = 24.8;
	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_FALSE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 1);
	EXPECT_TRUE(InFirstInfoEntry("24.8"));

	log_items_.clear();
	test_node_["m1"]["it2"][0] = 24;
	test_node_["val"] = YAML::Load("[]\n");
	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_TRUE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 0);
}

TEST_F(YamlSchemaValidationTest, TestMapElementOptGeneral)
{
	schema_node_ = YAML::Load(
		"data: OPTBOOL\n");
	test_node_ = YAML::Load(
		"data: true\n");

	YAML::const_iterator it_schema = schema_node_.begin();
	YAML::const_iterator it_test = test_node_.begin();

	res_ = ysv_.TestMapElement(it_schema, it_test, log_items_);
	EXPECT_TRUE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 0);

	test_node_ = YAML::Load(
		"data:\n");
	it_test = test_node_.begin();
	res_ = ysv_.TestMapElement(it_schema, it_test, log_items_);
	EXPECT_TRUE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 0);

	test_node_["data"] = "fals";
	res_ = ysv_.TestMapElement(it_schema, it_test, log_items_);
	EXPECT_FALSE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 1);
	
	log_items_.clear();
	schema_node_["data"] = "OPTINT";
	test_node_["data"] = 500;
	res_ = ysv_.TestMapElement(it_schema, it_test, log_items_);
	EXPECT_TRUE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 0);

	test_node_ = YAML::Load(
		"data:\n");
	it_test = test_node_.begin();
	res_ = ysv_.TestMapElement(it_schema, it_test, log_items_);
	EXPECT_TRUE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 0);

	test_node_["data"] = 30.3;
	res_ = ysv_.TestMapElement(it_schema, it_test, log_items_);
	EXPECT_FALSE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 1);
}

TEST_F(YamlSchemaValidationTest, ProcessNodeMapOptGeneral)
{
	schema_node_ = YAML::Load(
		"data: OPTBOOL\n"
		"m1:\n"
		"  item: [FLT]\n"
		"  it2: [OPTINT]\n"
		"val: OPTINT\n");
	test_node_ = YAML::Load(
		"data: true\n"
		"m1:\n"
		"  item: [19.2, 3.2]\n"
		"  it2: [100]\n"
		"val: 23\n");

	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_TRUE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 0);

	test_node_["data"] = YAML::Node(YAML::NodeType::value::Null);
	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_TRUE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 0);

	test_node_["m1"]["val"] = YAML::Node(YAML::NodeType::value::Null);
	res_ = ysv_.ProcessNode(test_node_, schema_node_, log_items_);
	EXPECT_TRUE(res_);
	EXPECT_EQ(InfoLogEntryCount(), 0);
}