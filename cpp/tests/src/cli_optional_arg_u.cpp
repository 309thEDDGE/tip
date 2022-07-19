#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "cli_optional_arg.h"

class CLIOptionalArgTest : public ::testing::Test
{
   protected:
    
    std::string temp_label_;
    std::string temp_short_label_;
    std::string temp_help_str_;
    int default_;
    std::string user_str_;

    CLIOptionalArgTest() : temp_label_("test_arg"), temp_help_str_("none"), default_(10),
        user_str_(""), temp_short_label_("")
    { }

};

TEST_F(CLIOptionalArgTest, ValidateConfigPass)
{
    int output = 0;
    temp_label_ = "--Valu_09";  
    temp_short_label_ = "-A";
    CLIOptionalArg cli(temp_label_, temp_short_label_, temp_help_str_, default_, output);
    ASSERT_TRUE(cli.ValidateConfig());
}

TEST_F(CLIOptionalArgTest, ValidateConfigFail)
{
    int output = 0;
    temp_label_ = "--Valu-09";  // no hyphen
    temp_short_label_ = "-A";
    CLIOptionalArg cli(temp_label_, temp_short_label_, temp_help_str_, default_, output);
    ASSERT_FALSE(cli.ValidateConfig());
}

TEST_F(CLIOptionalArgTest, ValidateConfigFail2)
{
    int output = 0;
    temp_label_ = "-Valu09";  // single hyphen
    temp_short_label_ = "-A";
    CLIOptionalArg cli(temp_label_, temp_short_label_, temp_help_str_, default_, output);
    ASSERT_FALSE(cli.ValidateConfig());
}

TEST_F(CLIOptionalArgTest, ValidateConfigFailShortLabel)
{
    int output = 0;
    temp_label_ = "--Valu09";  
    temp_short_label_ = "-bA";  // only one char after hyphen
    CLIOptionalArg cli(temp_label_, temp_short_label_, temp_help_str_, default_, output);
    ASSERT_FALSE(cli.ValidateConfig());
}

TEST_F(CLIOptionalArgTest, ValidateConfigFailShortLabel2)
{
    int output = 0;
    temp_label_ = "--Valu09";  
    temp_short_label_ = "--b";  // two hyphens not allowed 
    CLIOptionalArg cli(temp_label_, temp_short_label_, temp_help_str_, default_, output);
    ASSERT_FALSE(cli.ValidateConfig());
}

TEST_F(CLIOptionalArgTest, ValidateConfigPassEmptyShortLabel)
{
    int output = 0;
    temp_label_ = "--Valu09"; // valid 
    temp_short_label_ = "";  
    CLIOptionalArg cli(temp_label_, temp_short_label_, temp_help_str_, default_, output);
    ASSERT_TRUE(cli.ValidateConfig());
}

TEST_F(CLIOptionalArgTest, ValidateConfigPassEmptyLabel)
{
    int output = 0;
    temp_label_ = "";  
    temp_short_label_ = "-8";  
    CLIOptionalArg cli(temp_label_, temp_short_label_, temp_help_str_, default_, output);
    ASSERT_TRUE(cli.ValidateConfig());
}

TEST_F(CLIOptionalArgTest, ValidateConfigFailBothLabelsEmpty)
{
    int output = 0;
    temp_label_ = "";  
    temp_short_label_ = "";  
    CLIOptionalArg cli(temp_label_, temp_short_label_, temp_help_str_, default_, output);
    ASSERT_FALSE(cli.ValidateConfig());
}

TEST_F(CLIOptionalArgTest, ValidateUserPass)
{
    int output = 0;
    temp_label_ = "--data";  
    temp_short_label_ = "";  
    CLIOptionalArg cli(temp_label_, temp_short_label_, temp_help_str_, default_, output);
    user_str_ = "blah blah --temp thing --data The_Data03";
    ASSERT_TRUE(cli.ValidateUser(user_str_));
    EXPECT_EQ(cli.GetSMatch()[2].str(), "The_Data03");

    user_str_ = "blah blah --data The_Data03 --temp thing";
    ASSERT_TRUE(cli.ValidateUser(user_str_));
    EXPECT_EQ(cli.GetSMatch()[2].str(), "The_Data03");

    user_str_ = "--data The_Data03 blah blah --temp thing";
    ASSERT_TRUE(cli.ValidateUser(user_str_));
    EXPECT_EQ(cli.GetSMatch()[2].str(), "The_Data03");

    user_str_ = "--data D:\\my\\dir\\file.txt blah blah --temp thing";
    ASSERT_TRUE(cli.ValidateUser(user_str_));
    EXPECT_EQ(cli.GetSMatch()[2].str(), "D:\\my\\dir\\file.txt");

    user_str_ = "--data /my/dir/file.txt blah blah --temp thing";
    ASSERT_TRUE(cli.ValidateUser(user_str_));
    EXPECT_EQ(cli.GetSMatch()[2].str(), "/my/dir/file.txt");

    user_str_ = "--data /my/dir/better-file.txt blah blah --temp thing";
    ASSERT_TRUE(cli.ValidateUser(user_str_));
    EXPECT_EQ(cli.GetSMatch()[2].str(), "/my/dir/better-file.txt");
}

TEST_F(CLIOptionalArgTest, ValidateUserPassShortLabel)
{
    int output = 0;
    temp_label_ = "";  
    temp_short_label_ = "-d";  
    CLIOptionalArg cli(temp_label_, temp_short_label_, temp_help_str_, default_, output);
    user_str_ = "blah blah --temp thing -d The_Data03";
    ASSERT_TRUE(cli.ValidateUser(user_str_));
    EXPECT_EQ(cli.GetSMatch()[2].str(), "The_Data03");

    user_str_ = "blah blah -d The_Data03 --temp thing";
    ASSERT_TRUE(cli.ValidateUser(user_str_));
    EXPECT_EQ(cli.GetSMatch()[2].str(), "The_Data03");

    user_str_ = "-d The_Data03 blah blah --temp thing";
    ASSERT_TRUE(cli.ValidateUser(user_str_));
    EXPECT_EQ(cli.GetSMatch()[2].str(), "The_Data03");

    user_str_ = "-d D:\\my\\dir\\file.txt blah blah --temp thing";
    ASSERT_TRUE(cli.ValidateUser(user_str_));
    EXPECT_EQ(cli.GetSMatch()[2].str(), "D:\\my\\dir\\file.txt");

    user_str_ = "-d /my/dir/file.txt blah blah --temp thing";
    ASSERT_TRUE(cli.ValidateUser(user_str_));
    EXPECT_EQ(cli.GetSMatch()[2].str(), "/my/dir/file.txt");

    user_str_ = "-d /my/dir/better-file.txt blah blah --temp thing";
    ASSERT_TRUE(cli.ValidateUser(user_str_));
    EXPECT_EQ(cli.GetSMatch()[2].str(), "/my/dir/better-file.txt");
}

TEST_F(CLIOptionalArgTest, ValidateUserFail)
{
    int output = 0;
    temp_label_ = "--data";  
    temp_short_label_ = "-d";  
    CLIOptionalArg cli(temp_label_, temp_short_label_, temp_help_str_, default_, output);
    user_str_ = "blah --d8ta blah --temp thing -D The_Data03";
    ASSERT_FALSE(cli.ValidateUser(user_str_));
}

TEST_F(CLIOptionalArgTest, ValidateUserLowestPositionMatchFirst)
{
    int output = 0;
    temp_label_ = "--data";  
    temp_short_label_ = "-d";  
    CLIOptionalArg cli(temp_label_, temp_short_label_, temp_help_str_, default_, output);
    user_str_ = "blah --data 100 --temp thing -d 200";
    ASSERT_TRUE(cli.ValidateUser(user_str_));
    EXPECT_EQ("100", cli.GetSMatch()[2].str());
}

TEST_F(CLIOptionalArgTest, ParseIntCastArgumentFail)
{
    int output = 0;
    user_str_ = "here are --my args --value a95";   // "a95" not castable to int
    temp_label_ = "--value";  
    temp_short_label_ = "-m";
    CLIOptionalArg cli(temp_label_, temp_short_label_, temp_help_str_, default_, output);
    ASSERT_FALSE(cli.Parse(user_str_));
    EXPECT_TRUE(cli.IsPresent());
    EXPECT_FALSE(cli.IsValid());
}

TEST_F(CLIOptionalArgTest, ParseIntCastArgumentShortArgFail)
{
    int output = 0;
    user_str_ = "here are --my args -v a95";   // "a95" not castable to int
    temp_label_ = "--value";  
    temp_short_label_ = "-v";
    CLIOptionalArg cli(temp_label_, temp_short_label_, temp_help_str_, default_, output);
    ASSERT_FALSE(cli.Parse(user_str_));
    EXPECT_TRUE(cli.IsPresent());
    EXPECT_FALSE(cli.IsValid());
}

TEST_F(CLIOptionalArgTest, ParseIntPass)
{
    int output = 0;
    user_str_ = "here are --my 5 -value 95";
    temp_label_ = "--my";  
    temp_short_label_ = "-v"; 
    CLIOptionalArg cli(temp_label_, temp_short_label_, temp_help_str_, default_, output);
    ASSERT_TRUE(cli.Parse(user_str_));
    EXPECT_TRUE(cli.IsPresent());
    EXPECT_TRUE(cli.IsValid());
    EXPECT_EQ(5, output);
}

TEST_F(CLIOptionalArgTest, ParseIntInvalidLabel)
{
    int output = 0;
    user_str_ = "here are --my 5 -value 95";
    temp_label_ = "--value";  
    temp_short_label_ = "-v"; 
    CLIOptionalArg cli(temp_label_, temp_short_label_, temp_help_str_, default_, output);
    ASSERT_TRUE(cli.Parse(user_str_));
    EXPECT_FALSE(cli.IsPresent());
    EXPECT_FALSE(cli.IsValid());
    EXPECT_EQ(default_, output);
}

TEST_F(CLIOptionalArgTest, ParseIntInvalidShortLabel)
{
    int output = 0;
    user_str_ = "here are -my 5 --value 95";
    temp_label_ = "--val";  
    temp_short_label_ = "-m";  
    CLIOptionalArg cli(temp_label_, temp_short_label_, temp_help_str_, default_, output);
    ASSERT_TRUE(cli.Parse(user_str_));
    EXPECT_FALSE(cli.IsPresent());
    EXPECT_FALSE(cli.IsValid());
    EXPECT_EQ(default_, output);
}

TEST_F(CLIOptionalArgTest, ParseIntFailMissingArg)
{
    int output = 0;
    user_str_ = "--test -v";
    temp_label_ = "--test_opt";  
    temp_short_label_ = "-t";  
    CLIOptionalArg cli(temp_label_, temp_short_label_, temp_help_str_, default_, output);
    ASSERT_TRUE(cli.Parse(user_str_));
    EXPECT_FALSE(cli.IsPresent());
    EXPECT_FALSE(cli.IsValid());
    EXPECT_EQ(default_, output);
}

TEST_F(CLIOptionalArgTest, ParseStringWhiteSpacePass)
{
    std::string output = "";
    user_str_ = "-t my" + CLIArg::whitespace_code + "data" + CLIArg::whitespace_code + "str";
    temp_label_ = "--test_opt";  
    temp_short_label_ = "-t";  
    std::string default_str = "none";
    CLIOptionalArg<std::string> cli(temp_label_, temp_short_label_, temp_help_str_, 
        default_str, output);
    ASSERT_TRUE(cli.Parse(user_str_));
    EXPECT_TRUE(cli.IsPresent());
    EXPECT_TRUE(cli.IsValid());
    EXPECT_EQ("my data str", output);
}

TEST_F(CLIOptionalArgTest, ParseStringUseSpecialConfigDefaultUseParentDirOf)
{
    std::string output = "";
    user_str_ = "--test -v";
    temp_label_ = "--test_opt";  
    temp_short_label_ = "-t";  

    // Configured args not in user string. So use expect default, but modified by the 
    // special configuration.
#ifdef __linux__
    std::string special_config_input = "/data/path/file.txt";
    std::string expected = "/data/path";
#else
    std::string special_config_input = "C:\\data\\path\\file.txt";
    std::string expected = "C:\\data\\path";
#endif
    std::string default_val = "nothing";
    std::shared_ptr<CLIArg> optarg = CLIOptionalArg<std::string>::Make(temp_label_, 
        temp_short_label_, temp_help_str_, default_val, output);
    optarg->DefaultUseParentDirOf(special_config_input);
    ASSERT_TRUE(optarg->Parse(user_str_));
    EXPECT_FALSE(optarg->IsPresent());
    EXPECT_FALSE(optarg->IsValid());
    EXPECT_EQ(expected, output);
}

TEST_F(CLIOptionalArgTest, ParseStringUseSpecialConfigDefaultUseParentDirOfLocalFile)
{
    std::string output = "";
    user_str_ = "--test -v";
    temp_label_ = "--test_opt";  
    temp_short_label_ = "-t";  

    ManagedPath cwd;
    ManagedPath file_name("file.txt");

    std::string special_config_input = file_name.RawString();
    std::string expected = cwd.RawString();
    std::string default_val = "nothing";
    std::shared_ptr<CLIArg> optarg = CLIOptionalArg<std::string>::Make(temp_label_, 
        temp_short_label_, temp_help_str_, default_val, output);
    optarg->DefaultUseParentDirOf(special_config_input);
    ASSERT_TRUE(optarg->Parse(user_str_));
    EXPECT_FALSE(optarg->IsPresent());
    EXPECT_FALSE(optarg->IsValid());
    EXPECT_EQ(expected, output);
}

TEST_F(CLIOptionalArgTest, ParseStringUseSpecialConfigDefaultUseValueOf)
{
    int output = 0;
    user_str_ = "--test -v";
    temp_label_ = "--test_opt";  
    temp_short_label_ = "-t";  

    // Configured args not in user string. So use expect default, but modified by the 
    // special configuration.
    int special_config_input = 12345;
    int default_val = 991100;
    std::shared_ptr<CLIArg> optarg = CLIOptionalArg<int>::Make(temp_label_, 
        temp_short_label_, temp_help_str_, default_val, output);
    optarg->DefaultUseValueOf(special_config_input);
    ASSERT_TRUE(optarg->Parse(user_str_));
    EXPECT_FALSE(optarg->IsPresent());
    EXPECT_FALSE(optarg->IsValid());
    EXPECT_EQ(special_config_input, output);
}

TEST_F(CLIOptionalArgTest, GetPrintNameBothPresent)
{
    int output = 0;
    temp_label_ = "--val";  
    temp_short_label_ = "-m";  
    CLIOptionalArg cli(temp_label_, temp_short_label_, temp_help_str_, default_, output);
    ASSERT_EQ("--val, -m", cli.GetPrintName());
}

TEST_F(CLIOptionalArgTest, GetPrintNameLabelOnly)
{
    int output = 0;
    temp_label_ = "--val";  
    temp_short_label_ = "";  
    CLIOptionalArg cli(temp_label_, temp_short_label_, temp_help_str_, default_, output);
    ASSERT_EQ("--val", cli.GetPrintName());
}

TEST_F(CLIOptionalArgTest, GetPrintNameShortLabelOnly)
{
    int output = 0;
    temp_label_ = "";  
    temp_short_label_ = "-v";  
    CLIOptionalArg cli(temp_label_, temp_short_label_, temp_help_str_, default_, output);
    ASSERT_EQ("-v", cli.GetPrintName());
}

TEST_F(CLIOptionalArgTest, GetMatchString)
{
    int output = 0;
    user_str_ = "here are --my args -v a95";   // "a95" not castable to int
    temp_label_ = "--value";  
    temp_short_label_ = "-v";
    CLIOptionalArg cli(temp_label_, temp_short_label_, temp_help_str_, default_, output);
    ASSERT_FALSE(cli.Parse(user_str_));
    EXPECT_FALSE(cli.IsValid());
    std::string match_str = "-v a95";
    EXPECT_EQ(match_str, cli.GetMatchString());
}

TEST_F(CLIOptionalArgTest, GetUsageReprLabelOnly)
{
    int output = 0;
    temp_label_ = "--value";  
    temp_short_label_ = "";
    CLIOptionalArg cli(temp_label_, temp_short_label_, temp_help_str_, default_, output);
    std::string expected = "[--value <value>]";
    ASSERT_EQ(expected, cli.GetUsageRepr());
}

TEST_F(CLIOptionalArgTest, GetUsageReprShortLabelOnly)
{
    int output = 0;
    temp_label_ = "";  
    temp_short_label_ = "-v";
    CLIOptionalArg cli(temp_label_, temp_short_label_, temp_help_str_, default_, output);
    std::string expected = "[-v <value>]";
    ASSERT_EQ(expected, cli.GetUsageRepr());
}

TEST_F(CLIOptionalArgTest, GetUsageReprBothLabels)
{
    int output = 0;
    temp_label_ = "--value";  
    temp_short_label_ = "-v";
    CLIOptionalArg cli(temp_label_, temp_short_label_, temp_help_str_, default_, output);
    std::string expected = "[--value | -v <value>]";
    ASSERT_EQ(expected, cli.GetUsageRepr());
}

TEST_F(CLIOptionalArgTest, GetDefaultString)
{
    int output = 0;
    temp_label_ = "--value";  
    temp_short_label_ = "-v";
    default_ = 1551;
    CLIOptionalArg cli(temp_label_, temp_short_label_, temp_help_str_, default_, output);
    std::string expected = "default: 1551";
    ASSERT_EQ(expected, cli.GetDefaultString());
}

TEST_F(CLIOptionalArgTest, GetDefaultStringAsString)
{
    std::string output = "";
    temp_label_ = "--value";  
    temp_short_label_ = "-v";
    std::string def_value = "my/path/to";
    CLIOptionalArg cli(temp_label_, temp_short_label_, temp_help_str_, def_value, output);
    std::string expected = "default: my/path/to";
    ASSERT_EQ(expected, cli.GetDefaultString());
}

TEST_F(CLIOptionalArgTest, GetDefaultStringAsBool)
{
    bool output = false;
    temp_label_ = "--value";  
    temp_short_label_ = "-v";
    bool def_value = false;
    CLIOptionalArg cli(temp_label_, temp_short_label_, temp_help_str_, def_value, output);
    std::string expected = "default: false";
    ASSERT_EQ(expected, cli.GetDefaultString());

    def_value = true;
    CLIOptionalArg cli2(temp_label_, temp_short_label_, temp_help_str_, def_value, output);
    expected = "default: true";
    ASSERT_EQ(expected, cli2.GetDefaultString());
}

TEST_F(CLIOptionalArgTest, ValidatePermittedValuesAreUseDefault)
{
    std::string output = "";
    temp_label_ = "--value";  
    temp_short_label_ = "-v";
    std::string def_value = "TEST";
    std::shared_ptr<CLIArg> cli = CLIOptionalArg<std::string>::Make(temp_label_, 
        temp_short_label_, temp_help_str_, def_value, output);
    std::string user_input = "--other_opt OPT some_pos_arg";
    std::set<std::string> permitted{"AA", "BB", "Apple"};
    cli->ValidatePermittedValuesAre(permitted);
    ASSERT_TRUE(cli->ValidateConfig());
    ASSERT_TRUE(cli->Parse(user_input));
    ASSERT_FALSE(cli->IsValid());
    ASSERT_EQ(def_value, output);
}

TEST_F(CLIOptionalArgTest, ValidatePermittedValuesAreNotPermitted)
{
    std::string output = "";
    temp_label_ = "--value";  
    temp_short_label_ = "-v";
    std::string def_value = "TEST";
    std::shared_ptr<CLIArg> cli = CLIOptionalArg<std::string>::Make(temp_label_, 
        temp_short_label_, temp_help_str_, def_value, output);

    // "Bb" not a permitted value.
    std::string user_input = "--other_opt OPT --value Bb some_pos_arg";
    std::set<std::string> permitted{"AA", "BB", "Apple"};
    cli->ValidatePermittedValuesAre(permitted);
    ASSERT_TRUE(cli->ValidateConfig());
    ASSERT_FALSE(cli->Parse(user_input));
    ASSERT_FALSE(cli->IsValid());
    ASSERT_EQ(def_value, output);
}

TEST_F(CLIOptionalArgTest, ValidatePermittedValuesAreValidated)
{
    std::string output = "";
    temp_label_ = "--value";  
    temp_short_label_ = "-v";
    std::string def_value = "TEST";
    std::shared_ptr<CLIArg> cli = CLIOptionalArg<std::string>::Make(temp_label_, 
        temp_short_label_, temp_help_str_, def_value, output);

    // "BB" is a permitted value.
    std::string user_input = "--other_opt OPT --value BB some_pos_arg";
    std::set<std::string> permitted{"AA", "BB", "Apple"};
    cli->ValidatePermittedValuesAre(permitted);
    ASSERT_TRUE(cli->ValidateConfig());
    ASSERT_TRUE(cli->Parse(user_input));
    ASSERT_TRUE(cli->IsValid());
    std::string expected = "BB";
    ASSERT_EQ(expected, output);
}

TEST_F(CLIOptionalArgTest, ValidatePermittedValuesAreNotPermittedInt)
{
    int output = 0;
    temp_label_ = "--value";  
    temp_short_label_ = "-v";
    int def_value = 1000;
    std::shared_ptr<CLIArg> cli = CLIOptionalArg<int>::Make(temp_label_, 
        temp_short_label_, temp_help_str_, def_value, output);

    std::string user_input = "--other_opt OPT --value 12 some_pos_arg";
    std::set<int> permitted{1, 10, 100, 1000};
    cli->ValidatePermittedValuesAre(permitted);
    ASSERT_TRUE(cli->ValidateConfig());
    ASSERT_FALSE(cli->Parse(user_input));
    ASSERT_FALSE(cli->IsValid());
    ASSERT_EQ(def_value, output);
}

TEST_F(CLIOptionalArgTest, ValidateInclusiveRangeIsFail)
{
    int output = 0;
    temp_label_ = "--value";  
    temp_short_label_ = "-v";
    int def_value = 1000;
    std::shared_ptr<CLIArg> cli = CLIOptionalArg<int>::Make(temp_label_, 
        temp_short_label_, temp_help_str_, def_value, output);

    std::string user_input = "--other_opt OPT --value 12 some_pos_arg";
    cli->ValidateInclusiveRangeIs(20, 100);
    ASSERT_TRUE(cli->ValidateConfig());
    ASSERT_FALSE(cli->Parse(user_input));
    ASSERT_FALSE(cli->IsValid());
    ASSERT_EQ(def_value, output);
}

TEST_F(CLIOptionalArgTest, ValidateInclusiveRangeIsPass)
{
    int output = 0;
    temp_label_ = "--value";  
    temp_short_label_ = "-v";
    int def_value = 1000;
    std::shared_ptr<CLIArg> cli = CLIOptionalArg<int>::Make(temp_label_, 
        temp_short_label_, temp_help_str_, def_value, output);

    std::string user_input = "--other_opt OPT -v 33 some_pos_arg";
    cli->ValidateInclusiveRangeIs(20, 100);
    ASSERT_TRUE(cli->ValidateConfig());
    ASSERT_TRUE(cli->Parse(user_input));
    ASSERT_TRUE(cli->IsValid());
    ASSERT_EQ(33, output);
}

TEST_F(CLIOptionalArgTest, RepeatedArgLowestPositionUsed)
{
    int output = 0;
    temp_label_ = "--value";  
    temp_short_label_ = "-v";
    int def_value = 1000;
    std::shared_ptr<CLIArg> cli = CLIOptionalArg<int>::Make(temp_label_, 
        temp_short_label_, temp_help_str_, def_value, output);

    std::string user_input = "--other_opt OPT -v 33 some_pos_arg --value 90";
    cli->ValidateInclusiveRangeIs(20, 100);
    ASSERT_TRUE(cli->ValidateConfig());
    ASSERT_TRUE(cli->Parse(user_input));
    ASSERT_TRUE(cli->IsValid());
    ASSERT_EQ(33, output);
}

TEST_F(CLIOptionalArgTest, VectorParse)
{
    temp_label_ = "--value";  
    temp_short_label_ = "-v";
    std::vector<int> output;
    std::vector<int> def_value{};
    std::shared_ptr<CLIArg> cli = CLIOptionalArg<std::vector<int>>::Make(temp_label_, 
        temp_short_label_, temp_help_str_, def_value, output);

    std::string user_input = "--other_opt OPT some_pos_arg --value 90";
    ASSERT_TRUE(cli->ValidateConfig());
    ASSERT_TRUE(cli->Parse(user_input));
    ASSERT_TRUE(cli->IsValid());
    ASSERT_EQ(1, output.size());
    ASSERT_EQ(90, output.at(0));
}

TEST_F(CLIOptionalArgTest, VectorParseRepeatedIsAdditive)
{
    temp_label_ = "--value";  
    temp_short_label_ = "-v";
    std::vector<int> output;
    std::vector<int> def_value{};
    std::shared_ptr<CLIArg> cli = CLIOptionalArg<std::vector<int>>::Make(temp_label_, 
        temp_short_label_, temp_help_str_, def_value, output);

    std::string user_input = "--other_opt OPT -v 33 some_pos_arg --value 90 -v 64";
    ASSERT_TRUE(cli->ValidateConfig());
    ASSERT_TRUE(cli->Parse(user_input));
    ASSERT_TRUE(cli->IsValid());
    ASSERT_EQ(3, output.size());
    ASSERT_EQ(33, output.at(0));
    ASSERT_EQ(64, output.at(2));
}

TEST_F(CLIOptionalArgTest, VectorParseRepeatedIsAdditiveWithWhiteSpace)
{
    temp_label_ = "--value";  
    temp_short_label_ = "-v";
    std::vector<std::string> output;
    std::vector<std::string> def_value{};
    std::shared_ptr<CLIArg> cli = CLIOptionalArg<std::vector<std::string>>::Make(temp_label_, 
        temp_short_label_, temp_help_str_, def_value, output);

    std::string user_input = "--other_opt OPT -v 33 some_pos_arg --value 90--..--days -v 64";
    ASSERT_TRUE(cli->ValidateConfig());
    ASSERT_TRUE(cli->Parse(user_input));
    ASSERT_TRUE(cli->IsValid());
    ASSERT_EQ(3, output.size());
    ASSERT_EQ("33", output.at(0));
    ASSERT_EQ("90 days", output.at(1));
    ASSERT_EQ("64", output.at(2));
}

TEST_F(CLIOptionalArgTest, VectorParseDefault)
{
    temp_label_ = "--value";  
    temp_short_label_ = "-v";
    std::vector<int> output;
    std::vector<int> def_value{-10, 8991, 90};
    std::shared_ptr<CLIArg> cli = CLIOptionalArg<std::vector<int>>::Make(temp_label_, 
        temp_short_label_, temp_help_str_, def_value, output);

    std::string user_input = "--other_opt OPT some_pos_arg";
    ASSERT_TRUE(cli->ValidateConfig());
    ASSERT_TRUE(cli->Parse(user_input));
    ASSERT_FALSE(cli->IsValid());
    ASSERT_EQ(3, output.size());
    ASSERT_EQ(-10, output.at(0));
    ASSERT_EQ(90, output.at(2));
}

TEST_F(CLIOptionalArgTest, VectorParseEmptyDefault)
{
    temp_label_ = "--value";  
    temp_short_label_ = "-v";
    std::vector<int> output;
    std::vector<int> def_value{};
    std::shared_ptr<CLIArg> cli = CLIOptionalArg<std::vector<int>>::Make(temp_label_, 
        temp_short_label_, temp_help_str_, def_value, output);

    std::string user_input = "--other_opt OPT some_pos_arg";
    ASSERT_TRUE(cli->ValidateConfig());
    ASSERT_TRUE(cli->Parse(user_input));
    ASSERT_FALSE(cli->IsValid());
    ASSERT_EQ(0, output.size());
}

TEST_F(CLIOptionalArgTest, VectorGetDefaultString)
{
    temp_label_ = "--value";  
    temp_short_label_ = "-v";
    std::vector<int> output;
    std::vector<int> def_value{-10, 8991, 90};
    std::shared_ptr<CLIArg> cli = CLIOptionalArg<std::vector<int>>::Make(temp_label_, 
        temp_short_label_, temp_help_str_, def_value, output);

    std::string expected = "default: [-10, 8991, 90]";
    ASSERT_EQ(expected, cli->GetDefaultString());
}

TEST_F(CLIOptionalArgTest, MapParse)
{
    temp_label_ = "--value";  
    temp_short_label_ = "-v";
    std::map<std::string, int> output;
    std::map<std::string, int> def_value{{"A", 10}, {"bee", 2}, {"SEE", 17}};
    std::shared_ptr<CLIArg> cli = CLIOptionalArg<std::map<std::string, int>>::Make(temp_label_, 
        temp_short_label_, temp_help_str_, def_value, output);

    std::string user_input = "--other_opt OPT some_pos_arg --value Ten:5";
    ASSERT_TRUE(cli->ValidateConfig());
    ASSERT_TRUE(cli->Parse(user_input));
    ASSERT_TRUE(cli->IsValid());
    ASSERT_EQ(1, output.size());
    ASSERT_EQ(1, output.count("Ten"));
    ASSERT_EQ(5, output.at("Ten"));
}

TEST_F(CLIOptionalArgTest, MapParseRepeatedIsAdditive)
{
    temp_label_ = "--value";  
    temp_short_label_ = "-v";
    std::map<std::string, int> output;
    std::map<std::string, int> def_value{{"A", 10}, {"bee", 2}, {"SEE", 17}};
    std::shared_ptr<CLIArg> cli = CLIOptionalArg<std::map<std::string, int>>::Make(temp_label_, 
        temp_short_label_, temp_help_str_, def_value, output);

    std::string user_input = "--value C:203 --other_opt OPT -v data:9 some_pos_arg --value Ten:5";
    ASSERT_TRUE(cli->ValidateConfig());
    ASSERT_TRUE(cli->Parse(user_input));
    ASSERT_TRUE(cli->IsValid());
    ASSERT_EQ(3, output.size());
    ASSERT_EQ(1, output.count("Ten"));
    ASSERT_EQ(5, output.at("Ten"));
    EXPECT_EQ(1, output.count("C"));
    EXPECT_EQ(1, output.count("data"));
    EXPECT_EQ(203, output.at("C"));
}

TEST_F(CLIOptionalArgTest, MapParseRepeatedIsAdditiveWhiteSpace)
{
    temp_label_ = "--value";  
    temp_short_label_ = "-v";
    std::map<std::string, std::string> output;
    std::map<std::string, std::string> def_value{{"A", "10"}, {"bee", "dd2"}, {"SEE", "blah17"}};
    std::shared_ptr<CLIArg> cli = CLIOptionalArg<std::map<std::string, std::string>>::Make(temp_label_, 
        temp_short_label_, temp_help_str_, def_value, output);

    std::string user_input = "--value C:d203 --other_opt OPT -v data:9--..--oh--..--five some_pos_arg --value Ten:5";
    ASSERT_TRUE(cli->ValidateConfig());
    ASSERT_TRUE(cli->Parse(user_input));
    ASSERT_TRUE(cli->IsValid());
    ASSERT_EQ(3, output.size());
    ASSERT_EQ(1, output.count("Ten"));
    ASSERT_EQ("5", output.at("Ten"));
    EXPECT_EQ(1, output.count("C"));
    EXPECT_EQ(1, output.count("data"));
    EXPECT_EQ("9 oh five", output.at("data"));
}

TEST_F(CLIOptionalArgTest, MapParseDefault)
{
    temp_label_ = "--value";  
    temp_short_label_ = "-v";
    std::map<std::string, int> output;
    std::map<std::string, int> def_value{{"A", 10}, {"bee", 2}, {"SEE", 17}};
    std::shared_ptr<CLIArg> cli = CLIOptionalArg<std::map<std::string, int>>::Make(temp_label_, 
        temp_short_label_, temp_help_str_, def_value, output);

    std::string user_input = "--other_opt OPT some_pos_arg --val Ten:5";
    ASSERT_TRUE(cli->ValidateConfig());
    ASSERT_TRUE(cli->Parse(user_input));
    ASSERT_FALSE(cli->IsValid());
    ASSERT_EQ(3, output.size());
    ASSERT_EQ(1, output.count("bee"));
    ASSERT_EQ(2, output.at("bee"));
}

TEST_F(CLIOptionalArgTest, MapParseEmptyDefault)
{
    temp_label_ = "--value";  
    temp_short_label_ = "-v";
    std::map<std::string, int> output;
    std::map<std::string, int> def_value{};
    std::shared_ptr<CLIArg> cli = CLIOptionalArg<std::map<std::string, int>>::Make(temp_label_, 
        temp_short_label_, temp_help_str_, def_value, output);

    std::string user_input = "--other_opt OPT some_pos_arg --val Ten:5";
    ASSERT_TRUE(cli->ValidateConfig());
    ASSERT_TRUE(cli->Parse(user_input));
    ASSERT_FALSE(cli->IsValid());
    ASSERT_EQ(0, output.size());
}

TEST_F(CLIOptionalArgTest, MapGetDefaultString)
{
    temp_label_ = "--value";  
    temp_short_label_ = "-v";
    std::map<std::string, int> output;
    std::map<std::string, int> def_value{{"A", 10}, {"bee", 2}, {"SEE", 17}};
    std::shared_ptr<CLIArg> cli = CLIOptionalArg<std::map<std::string, int>>::Make(temp_label_, 
        temp_short_label_, temp_help_str_, def_value, output);

    std::string expected = "default: [(A, 10), (SEE, 17), (bee, 2)]";
    ASSERT_EQ(expected, cli->GetDefaultString());
}