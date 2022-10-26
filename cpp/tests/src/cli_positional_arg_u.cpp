#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "cli_positional_arg.h"

class CLIPositionalArgTest : public ::testing::Test
{
   protected:
    
    std::string temp_label_;
    std::string temp_help_str_;
    std::string user_str_;

    CLIPositionalArgTest() : temp_label_("test_arg"), temp_help_str_("none"),
        user_str_("")
    { }

};

TEST_F(CLIPositionalArgTest, ValidateUserPass)
{
    int output = 0;
    CLIPositionalArg cli(temp_label_, temp_help_str_, output);
    user_str_ = "3lblZh50_rt0-data\\path/five";
    ASSERT_TRUE(cli.ValidateUser(user_str_));
}

TEST_F(CLIPositionalArgTest, ValidateUserFail1)
{
    int output = 0;
    CLIPositionalArg cli(temp_label_, temp_help_str_, output);
    user_str_ = "blahA/Bab01]";  // no "]"
    ASSERT_FALSE(cli.ValidateUser(user_str_));
}

TEST_F(CLIPositionalArgTest, ValidateUserFail2)
{
    int output = 0;
    CLIPositionalArg cli(temp_label_, temp_help_str_, output);
    user_str_ = "blahAi*Bab01";  // no asterisk 
    ASSERT_FALSE(cli.ValidateUser(user_str_));
}

TEST_F(CLIPositionalArgTest, ParseIntGetPositionalArgFail)
{
    int output = 0;
    CLIPositionalArg cli(temp_label_, temp_help_str_, output);
    user_str_ = "user-five[]"; 
    ASSERT_FALSE(cli.Parse(user_str_));
    EXPECT_FALSE(cli.IsPresent());
    EXPECT_FALSE(cli.IsValid());
}

TEST_F(CLIPositionalArgTest, ParseIntCastArgumentFail)
{
    int output = 0;
    CLIPositionalArg cli(temp_label_, temp_help_str_, output);
    user_str_ = "data";  // "string" is not castable to int
    ASSERT_FALSE(cli.Parse(user_str_));
    EXPECT_TRUE(cli.IsPresent());
    EXPECT_FALSE(cli.IsValid());
}

TEST_F(CLIPositionalArgTest, ParseInt)
{
    int output = 0;
    CLIPositionalArg cli(temp_label_, temp_help_str_, output);
    user_str_ = "100";  
    ASSERT_TRUE(cli.Parse(user_str_));
    EXPECT_TRUE(cli.IsPresent());
    EXPECT_TRUE(cli.IsValid());
    EXPECT_EQ(100, output);
}

TEST_F(CLIPositionalArgTest, ParseStringPath)
{
    std::string output = "";
    CLIPositionalArg cli(temp_label_, temp_help_str_, output);
    user_str_ = "D:\\data\\located\\here.txt";  
    ASSERT_TRUE(cli.Parse(user_str_));
    EXPECT_TRUE(cli.IsPresent());
    EXPECT_TRUE(cli.IsValid());
    EXPECT_EQ(user_str_, output);
}

TEST_F(CLIPositionalArgTest, ParseStringPathWhiteSpace)
{
    std::string output = "";
    CLIPositionalArg cli(temp_label_, temp_help_str_, output);
    user_str_ = "D:\\data\\located at\\here.txt";  
    ASSERT_TRUE(cli.Parse(user_str_));
    EXPECT_TRUE(cli.IsPresent());
    EXPECT_TRUE(cli.IsValid());
    EXPECT_EQ(user_str_, output);
}

TEST_F(CLIPositionalArgTest, GetUsageRepr)
{
    int output = 0;
    CLIPositionalArg cli(temp_label_, temp_help_str_, output);
    std::string expected = "TEST_ARG";
    ASSERT_EQ(expected, cli.GetUsageRepr());
}

TEST_F(CLIPositionalArgTest, GetDefaultString)
{
    int output = 0;
    CLIPositionalArg cli(temp_label_, temp_help_str_, output);
    std::string expected = "default: none";
    ASSERT_EQ(expected, cli.GetDefaultString());
}

TEST_F(CLIPositionalArgTest, GetPrintName)
{
    int output = 0;
    temp_label_ = "libpath";
    CLIPositionalArg cli(temp_label_, temp_help_str_, output);
    std::string expected = "LIBPATH";
    ASSERT_EQ(expected, cli.GetPrintName());
}

// See test of same name from CLIOptionalArgTest suite
TEST_F(CLIPositionalArgTest, ParseAllowSpecialCharacters)
{
    // Add #, (, ), $
    std::string output = "";
    CLIPositionalArg cli(temp_label_, temp_help_str_, output);
    user_str_ = "D:\\data$\\located(at)\\here.#txt";  
    ASSERT_TRUE(cli.Parse(user_str_));
    EXPECT_TRUE(cli.IsPresent());
    EXPECT_TRUE(cli.IsValid());
    EXPECT_EQ(user_str_, output);
}