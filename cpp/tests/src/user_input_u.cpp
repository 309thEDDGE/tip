#include "gtest/gtest.h"
#include "user_input.h"


TEST(UserInput, GetUnsignedIntNegative)
{
	UserInput user_input;
	uint64_t value = -1;
	std::vector<std::string> test_inputs = { "-1", "q"};
	bool valid = user_input.GetUnsignedInt(value, "Channel ID", NULL, &test_inputs);
	EXPECT_FALSE(valid);
}

TEST(UserInput, GetUnsignedIntValid)
{
	UserInput user_input;
	uint64_t value = -1;
	std::vector<std::string> test_inputs = { "0" };
	bool valid = user_input.GetUnsignedInt(value, "Channel ID", NULL, &test_inputs);
	EXPECT_TRUE(valid);
	EXPECT_EQ(value,0);
}

TEST(UserInput, GetUnsignedIntChar)
{
	UserInput user_input;
	uint64_t value = -1;
	std::vector<std::string> test_inputs = { "a", "q" };
	bool valid = user_input.GetUnsignedInt(value, "Channel ID", NULL, &test_inputs);
	EXPECT_FALSE(valid);
}

TEST(UserInput, GetUnsignedLoopUntilValid)
{
	UserInput user_input;
	uint64_t value = -1;
	std::vector<std::string> test_inputs = { "a" , "-1", "1"};
	bool valid = user_input.GetUnsignedInt(value, "Channel ID", NULL, &test_inputs);
	EXPECT_TRUE(valid);
	EXPECT_EQ(value, 1);
}

TEST(UserInput, GetUnsignedInvalidOptions)
{
	UserInput user_input;
	uint64_t value = -1;
	std::vector<std::string> test_inputs = { "a" , "-1", "1" };
	std::set<uint64_t> opt_set = { 1,2 };
	bool valid = user_input.GetUnsignedInt(value, "Channel ID", &opt_set, &test_inputs);
	EXPECT_TRUE(valid);
	EXPECT_EQ(value, 1);
}

TEST(UserInput, GetUnsignedInvalidOptions2)
{
	UserInput user_input;
	uint64_t value = -1;
	std::vector<std::string> test_inputs = { "a" , "-1", "q" };
	std::set<uint64_t> opt_set = { 1,2 };
	bool valid = user_input.GetUnsignedInt(value, "Channel ID", &opt_set, &test_inputs);
	EXPECT_FALSE(valid);
}

/*
TEST(UserInput, GetUnsignedConsoleTest1)
{
	UserInput user_input;
	uint64_t value = -1;
	bool valid = user_input.GetUnsignedInt(value, "Enter a Channel ID", std::set<uint64_t>({ 1,2 }));
	EXPECT_TRUE(valid);
	EXPECT_EQ(value, 1);
}

TEST(UserInput, GetUnsignedConsoleTestQuit)
{
	UserInput user_input;
	uint64_t value = -1;
	bool valid = user_input.GetUnsignedInt(value, "Enter a Channel ID", std::set<uint64_t>());
	EXPECT_FALSE(valid);
}*/


TEST(UserInput, GetStringNoValidOptions)
{
	UserInput user_input;
	std::string value = "";
	std::vector<std::string> test_inputs = { "-1" };
	bool valid = user_input.GetString(value, "Channel ID", NULL, &test_inputs);
	EXPECT_TRUE(valid);
	EXPECT_EQ(value, "-1");
}

TEST(UserInput, GetStringQuitRightOff)
{
	UserInput user_input;
	std::string value = "";
	std::vector<std::string> test_inputs = { "q" };
	bool valid = user_input.GetString(value, "Channel ID", NULL, &test_inputs);
	EXPECT_FALSE(valid);
}

TEST(UserInput, GetStringQuitInvalidOptions)
{
	UserInput user_input;
	std::string value = "";
	std::vector<std::string> test_inputs = { "-1", "q" };
	std::set<std::string> opt_set = { "op1", "op2" };
	bool valid = user_input.GetString(value, "Channel ID", &opt_set, &test_inputs);
	EXPECT_FALSE(valid);
}

TEST(UserInput, GetStringQuitValidOption)
{
	UserInput user_input;
	std::string value = "";
	std::vector<std::string> test_inputs = { "-1", "op2"};
	std::set<std::string> opt_set = { "op1", "op2" };
	bool valid = user_input.GetString(value, "Channel ID", &opt_set, &test_inputs);
	EXPECT_TRUE(valid);
	EXPECT_EQ(value, "op2");
}

/*
TEST(UserInput, GetStringConsoleTestOp2)
{
	UserInput user_input;
	std::string value = "";
	bool valid = user_input.GetString(value, "Enter a bus name:", std::set<std::string>({ "op1", "op2" }));
	EXPECT_TRUE(valid);
	EXPECT_EQ(value, "op2");
}

TEST(UserInput, GetStringConsoleTestQuit)
{
	UserInput user_input;
	std::string value = "";
	bool valid = user_input.GetString(value, "Enter a bus name:", std::set<std::string>({ "op1", "op2" }));
	EXPECT_FALSE(valid);
}

*/
