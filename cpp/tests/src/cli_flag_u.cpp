#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "cli_flag.h"

class CLIFlagTest : public ::testing::Test
{
   protected:
    
    std::string temp_label_;
    std::string temp_short_label_;
    std::string temp_help_str_;
    bool default_;
    bool output_;
    std::string user_str_;

    CLIFlagTest() : temp_label_("test_arg"), temp_help_str_("none"), default_(false),
        user_str_(""), temp_short_label_("some_other_arg_"), output_(false)
    { }

};


// Note that ValidateConfig is exactly the same as CLIOptionalArg. Therefore,
// it's tested in that class. ValidateUser is also the same function. It
// utilizes different class vars for label_user_rgx_ and short_label_user_rgx_.

TEST_F(CLIFlagTest, ValidateUser)
{
    user_str_ = "val another_val --opt data --my_flag";
    default_ = true;
    output_ = false;
    temp_label_ = "--my_flag";  
    temp_short_label_ = "-B";
    CLIFlag cli(temp_label_, temp_short_label_, temp_help_str_, default_, output_);
    ASSERT_TRUE(cli.ValidateUser(user_str_));

    user_str_ = "val --my_flag another_val --opt data";
    ASSERT_TRUE(cli.ValidateUser(user_str_));

    user_str_ = "--my_flag another_val --opt data";
    ASSERT_TRUE(cli.ValidateUser(user_str_));
}

TEST_F(CLIFlagTest, ValidateUserShortLabel)
{
    user_str_ = "val another_val --opt data --my_flag -B";
    default_ = true;
    output_ = false;
    temp_label_ = "--myflag";  
    temp_short_label_ = "-B";
    CLIFlag cli(temp_label_, temp_short_label_, temp_help_str_, default_, output_);
    ASSERT_TRUE(cli.ValidateUser(user_str_));

    user_str_ = "val --my_flag -B another_val --opt data";
    ASSERT_TRUE(cli.ValidateUser(user_str_));

    user_str_ = "-B --my_flag another_val --opt data";
    ASSERT_TRUE(cli.ValidateUser(user_str_));
}

TEST_F(CLIFlagTest, ValidateUserFail)
{
    user_str_ = "val another_val --opt data --my_flag -B";
    default_ = true;
    output_ = false;
    temp_label_ = "--myflag";  
    temp_short_label_ = "-b";
    CLIFlag cli(temp_label_, temp_short_label_, temp_help_str_, default_, output_);
    ASSERT_FALSE(cli.ValidateUser(user_str_));
}

TEST_F(CLIFlagTest, ParseUseDefault)
{
    user_str_ = "val another_val --opt data --my_flag";
    default_ = true;
    output_ = false;
    temp_label_ = "--myflag";  // Ought to be "--my_flag"
    temp_short_label_ = "-o";
    CLIFlag cli(temp_label_, temp_short_label_, temp_help_str_, default_, output_);

    ASSERT_TRUE(cli.Parse(user_str_));
    EXPECT_TRUE(output_);
    EXPECT_FALSE(cli.IsPresent());
    EXPECT_TRUE(cli.IsValid());
}

TEST_F(CLIFlagTest, ParseNotDefault1)
{
    user_str_ = "val another_val --opt data --my_flag";
    default_ = true;
    output_ = true;
    temp_label_ = "--my_flag";  
    temp_short_label_ = "-o";
    CLIFlag cli(temp_label_, temp_short_label_, temp_help_str_, default_, output_);

    ASSERT_TRUE(cli.Parse(user_str_));
    EXPECT_FALSE(output_);
    EXPECT_TRUE(cli.IsPresent());
    EXPECT_TRUE(cli.IsValid());
}

TEST_F(CLIFlagTest, ParseNotDefaultShortLabel)
{
    user_str_ = "val another_val -m --opt data --my_flag";
    default_ = true;
    output_ = true;
    temp_label_ = "--myflag";  // Ought to be "--my_flag"
    temp_short_label_ = "-m";
    CLIFlag cli(temp_label_, temp_short_label_, temp_help_str_, default_, output_);

    ASSERT_TRUE(cli.Parse(user_str_));
    EXPECT_FALSE(output_);
    EXPECT_TRUE(cli.IsPresent());
    EXPECT_TRUE(cli.IsValid());
}

TEST_F(CLIFlagTest, GetUsageReprLabelOnly)
{
    default_ = true;
    output_ = true;
    temp_label_ = "--myflag";  
    temp_short_label_ = "";
    CLIFlag cli(temp_label_, temp_short_label_, temp_help_str_, default_, output_);
    std::string expected = "[--myflag]";
    ASSERT_EQ(expected, cli.GetUsageRepr());
}

TEST_F(CLIFlagTest, GetUsageReprShortLabelOnly)
{
    default_ = true;
    output_ = true;
    temp_label_ = "";  
    temp_short_label_ = "-m";
    CLIFlag cli(temp_label_, temp_short_label_, temp_help_str_, default_, output_);
    std::string expected = "[-m]";
    ASSERT_EQ(expected, cli.GetUsageRepr());
}

TEST_F(CLIFlagTest, GetUsageReprBothLabels)
{
    default_ = true;
    output_ = true;
    temp_label_ = "--myflag";  
    temp_short_label_ = "-m";
    CLIFlag cli(temp_label_, temp_short_label_, temp_help_str_, default_, output_);
    std::string expected = "[--myflag | -m]";
    ASSERT_EQ(expected, cli.GetUsageRepr());
}

TEST_F(CLIFlagTest, RepeatedArgIsCorrect)
{
    user_str_ = "val another_val -m --opt data --myflag";
    default_ = true;
    output_ = true;
    temp_label_ = "--myflag";  
    temp_short_label_ = "-m";
    CLIFlag cli(temp_label_, temp_short_label_, temp_help_str_, default_, output_);

    ASSERT_TRUE(cli.Parse(user_str_));
    EXPECT_TRUE(cli.ValidateConfig());
    EXPECT_TRUE(cli.IsPresent());
    EXPECT_TRUE(cli.IsValid());
    EXPECT_FALSE(output_);
}