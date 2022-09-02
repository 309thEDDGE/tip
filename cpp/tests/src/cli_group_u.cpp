#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "cli.h"
#include "cli_group.h"
#include "parse_text.h"

class CLIGroupTest : public ::testing::Test
{
   protected:
    std::string cli_prog_name_;
    std::string cli_desc_;
    std::string cli_nickname_;
    CLIGroup cli_group_; 

    CLIGroupTest() : cli_group_(), cli_prog_name_(""), cli_desc_(""), cli_nickname_("")
    { }

};

// TEST_F(CLIGroupTest, AddContainersAppendend)
// {
//     cli_prog_name_ = "mytestprog";
//     cli_desc_ = "not much of a description";
//     CLI cli(cli_prog_name_, cli_desc_);
//     int output = 0;
//     cli.AddOption("posarg1", "pos arg one help", output);
//     ASSERT_TRUE(cli_group_.Add(cli, "tp1"));
//     ASSERT_EQ(1, cli_group_.GetGroupMap().size());
//     ASSERT_EQ(1, cli_group_.GetGroupLabels().size());
//     EXPECT_EQ(cli_prog_name_, cli_group_.GetGroupMap().at("tp1")->GetProgramName());
//     EXPECT_EQ("tp1", cli_group_.GetGroupLabels().at(0));
// }

// TEST_F(CLIGroupTest, AddConfigFail)
// {
//     cli_prog_name_ = "mytestprog";
//     cli_desc_ = "not much of a description";
//     CLI cli(cli_prog_name_, cli_desc_);
//     int output = 0;
//     int def = 10;

//     // "--a" invalid short label format
//     cli.AddOption("--arg1", "--a", "pos arg one help", def, output);
//     ASSERT_FALSE(cli_group_.Add(cli, "tp1"));
//     ASSERT_EQ(0, cli_group_.GetGroupMap().size());
//     ASSERT_EQ(0, cli_group_.GetGroupLabels().size());
// }

TEST_F(CLIGroupTest, AddCLI)
{
    cli_prog_name_ = "mytestprog";
    cli_desc_ = "not much of a description";
    cli_nickname_ = "1stcli";
    std::shared_ptr<CLIGroupMember> clig = cli_group_.AddCLI(cli_prog_name_, cli_desc_, cli_nickname_);
    ASSERT_TRUE(clig != nullptr);
    EXPECT_EQ("mytestprog", clig->GetProgramName());
    ASSERT_EQ(1, cli_group_.GetGroupMap().size());
    EXPECT_EQ(1, cli_group_.GetGroupMap().count(cli_nickname_));
}

TEST_F(CLIGroupTest, CheckConfigurationFailNoArgs)
{
    cli_prog_name_ = "mytestprog";
    cli_desc_ = "not much of a description";
    cli_nickname_ = "1stcli";
    std::shared_ptr<CLIGroupMember> clig = cli_group_.AddCLI(cli_prog_name_, cli_desc_, cli_nickname_);
    ASSERT_FALSE(cli_group_.CheckConfiguration());
}

TEST_F(CLIGroupTest, CheckConfigurationFail)
{
    cli_prog_name_ = "mytestprog";
    cli_desc_ = "not much of a description";
    cli_nickname_ = "1stcli";
    std::shared_ptr<CLIGroupMember> clig = cli_group_.AddCLI(cli_prog_name_, cli_desc_, cli_nickname_);
    int output = 0;
    // Ought to fail on "--t", invalid short label. 
    clig->AddOption<int>("--test_opt", "--t", "help string", 23, output, true);
    ASSERT_FALSE(cli_group_.CheckConfiguration());
}

TEST_F(CLIGroupTest, CheckConfigurationPass)
{
    cli_prog_name_ = "mytestprog";
    cli_desc_ = "not much of a description";
    cli_nickname_ = "1stcli";
    std::shared_ptr<CLIGroupMember> clig = cli_group_.AddCLI(cli_prog_name_, cli_desc_, cli_nickname_);
    int output = 0;
    clig->AddOption<int>("--test_opt", "-t", "help string", 23, output, true);
    bool bool_output = false;
    clig->AddOption("--verbose", "", "verbose help string", false, bool_output);
    ASSERT_TRUE(cli_group_.CheckConfiguration());
}

TEST_F(CLIGroupTest, ParseFailNotConfigured)
{
    cli_prog_name_ = "mytestprog";
    cli_desc_ = "not much of a description";
    cli_nickname_ = "1stcli";
    std::shared_ptr<CLIGroupMember> clig = cli_group_.AddCLI(cli_prog_name_, cli_desc_, cli_nickname_);
    int output = 0;
    clig->AddOption<int>("--test_opt", "-t", "help string", 23, output, true);
    bool bool_output = false;
    clig->AddOption("--verbose", "", "verbose help string", false, bool_output);
    const int arg_count = 0;
    int argc = arg_count;
    char* argv[2];
    cli_nickname_ = "";
    std::shared_ptr<CLIGroupMember> active_cli;
    ASSERT_FALSE(cli_group_.Parse(argc, argv, cli_nickname_, active_cli));
}

TEST_F(CLIGroupTest, ParseFailNotPresent)
{
    cli_prog_name_ = "mytestprog";
    cli_desc_ = "not much of a description";
    cli_nickname_ = "1stcli";
    std::shared_ptr<CLIGroupMember> clig = cli_group_.AddCLI(cli_prog_name_, cli_desc_, cli_nickname_);
    int output = 0;
    clig->AddOption<int>("--test_opt", "-t", "help string", 23, output, true);
    bool bool_output = false;
    clig->AddOption("--verbose", "", "verbose help string", false, bool_output);
    ASSERT_TRUE(cli_group_.CheckConfiguration());
    const int arg_count = 2;
    int argc = arg_count;
    char a1[] = "--test";
    char a2[] = "-v";
    char* argv[arg_count] = {a1, a2};
    cli_nickname_ = "";
    std::shared_ptr<CLIGroupMember> active_cli;
    ASSERT_FALSE(cli_group_.Parse(argc, argv, cli_nickname_, active_cli));
}

TEST_F(CLIGroupTest, ParseFailNonIndicatorPresent)
{
    cli_prog_name_ = "mytestprog";
    cli_desc_ = "not much of a description";
    cli_nickname_ = "1stcli";
    std::shared_ptr<CLIGroupMember> clig = cli_group_.AddCLI(cli_prog_name_, cli_desc_, cli_nickname_);
    int output = 0;
    clig->AddOption<int>("--test_opt", "-t", "help string", 23, output, true);
    bool bool_output = false;
    clig->AddOption("--verbose", "", "verbose help string", false, bool_output);
    ASSERT_TRUE(cli_group_.CheckConfiguration());
    const int arg_count = 3;
    int argc = arg_count;
    char a1[] = "--test";
    char a2[] = "23";
    char a3[] = "--verbose";
    char* argv[arg_count] = {a1, a2, a3};
    cli_nickname_ = "";
    std::shared_ptr<CLIGroupMember> active_cli;
    ASSERT_FALSE(cli_group_.Parse(argc, argv, cli_nickname_, active_cli));
}

TEST_F(CLIGroupTest, ParsePassIndicatorPresent)
{
    cli_prog_name_ = "mytestprog";
    cli_desc_ = "not much of a description";
    cli_nickname_ = "1stcli";
    std::shared_ptr<CLIGroupMember> clig = cli_group_.AddCLI(cli_prog_name_, cli_desc_, cli_nickname_);
    int output = 0;
    clig->AddOption<int>("--test_opt", "-t", "help string", 23, output, true);
    bool bool_output = false;
    clig->AddOption("--verbose", "", "verbose help string", false, bool_output);
    ASSERT_TRUE(cli_group_.CheckConfiguration());
    const int arg_count = 4;
    int argc = arg_count;
    char a0[] = "progname";
    char a1[] = "--test_opt";
    char a2[] = "999";
    char a3[] = "--verbose";
    char* argv[arg_count] = {a0, a1, a2, a3};
    cli_nickname_ = "";
    std::shared_ptr<CLIGroupMember> active_cli;
    ASSERT_TRUE(cli_group_.Parse(argc, argv, cli_nickname_, active_cli));
    EXPECT_EQ("1stcli", cli_nickname_);
    EXPECT_EQ(cli_prog_name_, active_cli->GetProgramName());
    EXPECT_EQ(999, output);
}

TEST_F(CLIGroupTest, ParseFailNonIndicatorPresent2)
{
    cli_prog_name_ = "mytestprog";
    cli_desc_ = "not much of a description";
    cli_nickname_ = "1stcli";
    std::shared_ptr<CLIGroupMember> clig = cli_group_.AddCLI(cli_prog_name_, cli_desc_, cli_nickname_);
    int output = 0;
    clig->AddOption<int>("--test_opt", "-t", "help string", 23, output);
    bool bool_output = false;
    clig->AddOption("--verbose", "", "verbose help string", false, bool_output, true);
    ASSERT_TRUE(cli_group_.CheckConfiguration());
    const int arg_count = 3;
    int argc = arg_count;
    char a1[] = "--test_opt";
    char a2[] = "300";
    char a3[] = "-v";
    char* argv[arg_count] = {a1, a2, a3};
    cli_nickname_ = "";
    std::shared_ptr<CLIGroupMember> active_cli;
    ASSERT_FALSE(cli_group_.Parse(argc, argv, cli_nickname_, active_cli));
}

TEST_F(CLIGroupTest, ParsePassIndicatorPresent2)
{
    cli_prog_name_ = "mytestprog";
    cli_desc_ = "not much of a description";
    cli_nickname_ = "1stcli";
    std::shared_ptr<CLIGroupMember> clig = cli_group_.AddCLI(cli_prog_name_, cli_desc_, cli_nickname_);
    int output = 0;
    clig->AddOption<int>("--test_opt", "-t", "help string", 23, output);
    bool bool_output = false;
    clig->AddOption("--verbose", "", "verbose help string", false, bool_output, true);
    ASSERT_TRUE(cli_group_.CheckConfiguration());
    const int arg_count = 3;
    int argc = arg_count;
    char a1[] = "--test";
    char a2[] = "300";
    char a3[] = "--verbose";
    char* argv[arg_count] = {a1, a2, a3};
    cli_nickname_ = "";
    std::shared_ptr<CLIGroupMember> active_cli;
    ASSERT_TRUE(cli_group_.Parse(argc, argv, cli_nickname_, active_cli));
    EXPECT_EQ("1stcli", cli_nickname_);
    EXPECT_EQ(cli_prog_name_, active_cli->GetProgramName());
    EXPECT_EQ(true, bool_output);
}

TEST_F(CLIGroupTest, ParseFailNonIndicatorPresent3)
{
    cli_prog_name_ = "mytestprog";
    cli_desc_ = "not much of a description";
    cli_nickname_ = "1stcli";
    std::shared_ptr<CLIGroupMember> clig = cli_group_.AddCLI(cli_prog_name_, cli_desc_, cli_nickname_);
    int output = 0;
    clig->AddOption<int>("--test_opt", "-t", "help string", 23, output);
    bool bool_output = false;
    clig->AddOption("--verbose", "-v", "verbose help string", false, bool_output, true);
    ASSERT_TRUE(cli_group_.CheckConfiguration());
    const int arg_count = 3;
    int argc = arg_count;
    char a1[] = "-t";
    char a2[] = "300";
    char a3[] = "--veto";
    char* argv[arg_count] = {a1, a2, a3};
    cli_nickname_ = "";
    std::shared_ptr<CLIGroupMember> active_cli;
    ASSERT_FALSE(cli_group_.Parse(argc, argv, cli_nickname_, active_cli));
}

TEST_F(CLIGroupTest, ParsePassIndicatorPresent3)
{
    cli_prog_name_ = "mytestprog";
    cli_desc_ = "not much of a description";
    cli_nickname_ = "1stcli";
    std::shared_ptr<CLIGroupMember> clig = cli_group_.AddCLI(cli_prog_name_, cli_desc_, cli_nickname_);
    int output = 0;
    clig->AddOption<int>("--test_opt", "-t", "help string", 23, output);
    bool bool_output = false;
    clig->AddOption("--verbose", "-v", "verbose help string", false, bool_output, true);
    ASSERT_TRUE(cli_group_.CheckConfiguration());
    const int arg_count = 3;
    int argc = arg_count;
    char a1[] = "--truth";
    char a2[] = "300";
    char a3[] = "-v";
    char* argv[arg_count] = {a1, a2, a3};
    cli_nickname_ = "";
    std::shared_ptr<CLIGroupMember> active_cli;
    ASSERT_TRUE(cli_group_.Parse(argc, argv, cli_nickname_, active_cli));
    EXPECT_EQ("1stcli", cli_nickname_);
    EXPECT_EQ(cli_prog_name_, active_cli->GetProgramName());
    EXPECT_EQ(true, bool_output);
}

TEST_F(CLIGroupTest, ParseTwoCLIsPass1)
{
    cli_prog_name_ = "mytestprog";
    cli_desc_ = "not much of a description";
    cli_nickname_ = "1stcli";
    std::shared_ptr<CLIGroupMember> clig = cli_group_.AddCLI(cli_prog_name_, cli_desc_, cli_nickname_);
    int output = 0;
    clig->AddOption<int>("--test_opt", "-t", "help string", 23, output);
    bool bool_output = false;
    clig->AddOption("--verbose", "-v", "verbose help string", false, bool_output, true);

    cli_nickname_ = "2ndcli";
    clig = cli_group_.AddCLI(cli_prog_name_, cli_desc_, cli_nickname_);
    std::string str_output = "";
    clig->AddOption("dir_path", "dir_path help", str_output, true);
    int checksum_output = 0;
    clig->AddOption("", "-C", "checksum help string", 2345, checksum_output);

    ASSERT_TRUE(cli_group_.CheckConfiguration());

    // Expect second cli to be accepted
    const int arg_count = 4;
    int argc = arg_count;
    char a0[] = "progname";
    char a1[] = "--test_opt";
    char a2[] = "300";
    char a3[] = "file:///path/todata";
    char* argv[arg_count] = {a0, a1, a2, a3};
    cli_nickname_ = "";
    std::shared_ptr<CLIGroupMember> active_cli;
    ASSERT_TRUE(cli_group_.Parse(argc, argv, cli_nickname_, active_cli));
    EXPECT_EQ("2ndcli", cli_nickname_);
    EXPECT_EQ(cli_prog_name_, active_cli->GetProgramName());
    EXPECT_EQ("file:///path/todata", str_output);
}

TEST_F(CLIGroupTest, ParseTwoCLIsPass2)
{
    cli_prog_name_ = "mytestprog";
    cli_desc_ = "not much of a description";
    cli_nickname_ = "1stcli";
    std::shared_ptr<CLIGroupMember> clig = cli_group_.AddCLI(cli_prog_name_, cli_desc_, cli_nickname_);
    int output = 0;
    clig->AddOption<int>("--test_opt", "-t", "help string", 23, output);
    bool bool_output = false;
    clig->AddOption("--verbose", "-v", "verbose help string", false, bool_output, true);

    cli_nickname_ = "2ndcli";
    clig = cli_group_.AddCLI(cli_prog_name_, cli_desc_, cli_nickname_);
    std::string str_output = "";
    clig->AddOption("dir_path", "dir_path help", str_output, true);
    int checksum_output = 0;
    clig->AddOption("", "-C", "checksum help string", 2345, checksum_output);

    ASSERT_TRUE(cli_group_.CheckConfiguration());

    // Expect first cli to be accepted
    const int arg_count = 6;
    int argc = arg_count;
    char a0[] = "mytestprog";
    char a1[] = "--test_opt";
    char a2[] = "300";
    char a3[] = "-C";
    char a4[] = "81548";
    char a5[] = "-v";
    char* argv[arg_count] = {a0, a1, a2, a3, a4, a5};
    cli_nickname_ = "";
    std::shared_ptr<CLIGroupMember> active_cli;
    ASSERT_TRUE(cli_group_.Parse(argc, argv, cli_nickname_, active_cli));
    EXPECT_EQ("1stcli", cli_nickname_);
    EXPECT_EQ(cli_prog_name_, active_cli->GetProgramName());
    EXPECT_EQ(300, output);
    EXPECT_EQ(true, bool_output);
}

TEST_F(CLIGroupTest, ParseTwoCLIsFailParse)
{
    cli_prog_name_ = "mytestprog";
    cli_desc_ = "not much of a description";
    cli_nickname_ = "1stcli";
    std::shared_ptr<CLIGroupMember> clig = cli_group_.AddCLI(cli_prog_name_, cli_desc_, cli_nickname_);
    int output = 0;
    clig->AddOption<int>("--test_opt", "-t", "help string", 23, output);
    bool bool_output = false;
    clig->AddOption("--verbose", "-v", "verbose help string", false, bool_output, true);

    cli_nickname_ = "2ndcli";
    clig = cli_group_.AddCLI(cli_prog_name_, cli_desc_, cli_nickname_);
    std::string str_output = "";
    clig->AddOption("dir_path", "dir_path help", str_output, true);
    int checksum_output = 0;

    // Fail on this arg
    clig->AddOption("", "-C", "checksum help string", 2345, checksum_output);

    ASSERT_TRUE(cli_group_.CheckConfiguration());
    const int arg_count = 5;
    int argc = arg_count;
    char a1[] = "--test_opt";
    char a2[] = "300";
    char a3[] = "-C";
    char a4[] = "data";
    char a5[] = "--verb";
    char* argv[arg_count] = {a1, a2, a3, a4, a5};
    cli_nickname_ = "";
    std::shared_ptr<CLIGroupMember> active_cli;
    ASSERT_FALSE(cli_group_.Parse(argc, argv, cli_nickname_, active_cli));
    EXPECT_EQ("", cli_nickname_);
    EXPECT_EQ(nullptr, active_cli);
}

TEST_F(CLIGroupTest, ParseTwoCLIsFailNoIndicators)
{
    cli_prog_name_ = "mytestprog";
    cli_desc_ = "not much of a description";
    cli_nickname_ = "1stcli";
    std::shared_ptr<CLIGroupMember> clig = cli_group_.AddCLI(cli_prog_name_, cli_desc_, cli_nickname_);
    int output = 0;
    clig->AddOption<int>("--test_opt", "-t", "help string", 23, output);
    bool bool_output = false;
    clig->AddOption("--verbose", "-v", "verbose help string", false, bool_output);

    cli_nickname_ = "2ndcli";
    clig = cli_group_.AddCLI(cli_prog_name_, cli_desc_, cli_nickname_);
    std::string str_output = "";
    clig->AddOption("dir_path", "dir_path help", str_output);
    int checksum_output = 0;
    clig->AddOption("", "-C", "checksum help string", 2345, checksum_output);

    ASSERT_TRUE(cli_group_.CheckConfiguration());
    const int arg_count = 4;
    int argc = arg_count;
    char a1[] = "--test_opt";
    char a2[] = "300";
    char a3[] = "--verb";
    char a4[] = "/yet/another/path";
    char* argv[arg_count] = {a1, a2, a3, a4};
    cli_nickname_ = "";
    std::shared_ptr<CLIGroupMember> active_cli;
    ASSERT_FALSE(cli_group_.Parse(argc, argv, cli_nickname_, active_cli));
    EXPECT_EQ("", cli_nickname_);
    EXPECT_EQ(nullptr, active_cli);
}

TEST_F(CLIGroupTest, MakeHelpStringNotConfigured)
{
    cli_prog_name_ = "mytestprog";
    cli_desc_ = "not much of a description";
    cli_nickname_ = "1stcli";
    std::shared_ptr<CLIGroupMember> clig = cli_group_.AddCLI(cli_prog_name_, cli_desc_, cli_nickname_);
    int output = 0;
    clig->AddOption<int>("--test_opt", "-t", "help string", 23, output);
    bool bool_output = false;
    clig->AddOption("--verbose", "-v", "verbose help string", false, bool_output);

    cli_nickname_ = "2ndcli";
    clig = cli_group_.AddCLI(cli_prog_name_, cli_desc_, cli_nickname_);
    std::string str_output = "";
    clig->AddOption("dir_path", "dir_path help", str_output);
    int checksum_output = 0;
    clig->AddOption("", "-C", "checksum help string", 2345, checksum_output);

    // ASSERT_TRUE(cli_group_.CheckConfiguration());
    std::string help_str = cli_group_.MakeHelpString();
    EXPECT_EQ("", help_str);
}

TEST_F(CLIGroupTest, MakeHelpString)
{
    // Note: MakeHelpString is called prior to SortArgs, which normally occurs
    // automatically in Parse(). Without sorting, args are placed in the order
    // in which they are added to the CLIGroupMembers.
    cli_prog_name_ = "mytestprog";
    cli_desc_ = "not much of a description";
    cli_nickname_ = "1stcli";
    std::shared_ptr<CLIGroupMember> clig = cli_group_.AddCLI(cli_prog_name_, cli_desc_, cli_nickname_);
    int output = 0;
    clig->AddOption<int>("--test_opt", "-t", "help string", 23, output);
    bool bool_output = false;
    clig->AddOption("--verbose", "-v", "verbose help string", false, bool_output);

    cli_nickname_ = "2ndcli";
    clig = cli_group_.AddCLI(cli_prog_name_, cli_desc_, cli_nickname_);
    std::string str_output = "";
    int checksum_output = 0;
    clig->AddOption("", "-C", "checksum help string", 2345, checksum_output);
    clig->AddOption("dir_path", "dir_path help", str_output);

    ASSERT_TRUE(cli_group_.CheckConfiguration());
    std::string help_str = cli_group_.MakeHelpString();
    std::string expected = (
        "Usage: mytestprog <positional arguments> [options / flags]\n"
        "       mytestprog [--test_opt | -t <value>] [--verbose | -v]\n"
        "       mytestprog DIR_PATH [-C <value>]\n\n"
        "not much of a description\n\n"
        "--test_opt, -t help string\n"
        "               default: 23\n\n"
        "--verbose, -v  verbose help string\n"
        "               default: false\n\n"
        "DIR_PATH       dir_path help\n"
        "               default: none\n\n"
        "-C             checksum help string\n"
        "               default: 2345\n\n"
    );
    // printf("\nhelp_str:\n%s\n", help_str.c_str());
    EXPECT_EQ(expected, help_str);
}

TEST_F(CLIGroupTest, ParseTwoCLIsPassNoArgsFail)
{
    cli_prog_name_ = "mytestprog";
    cli_desc_ = "not much of a description";
    cli_nickname_ = "1stcli";
    std::shared_ptr<CLIGroupMember> clig = cli_group_.AddCLI(cli_prog_name_, cli_desc_, cli_nickname_);
    int output = 0;
    clig->AddOption<int>("--test_opt", "-t", "help string", 23, output);
    bool bool_output = false;
    clig->AddOption("--verbose", "-v", "verbose help string", false, bool_output, true);

    cli_nickname_ = "2ndcli";
    clig = cli_group_.AddCLI(cli_prog_name_, cli_desc_, cli_nickname_);
    std::string str_output = "";
    clig->AddOption("dir_path", "dir_path help", str_output, true);
    int checksum_output = 0;
    clig->AddOption("", "-C", "checksum help string", 2345, checksum_output);

    ASSERT_TRUE(cli_group_.CheckConfiguration());

    // Expect first cli to be accepted
    const int arg_count = 1;
    int argc = arg_count;
    char a1[] = "blah";
    char* argv[arg_count] = {a1};
    cli_nickname_ = "";
    std::shared_ptr<CLIGroupMember> active_cli;
    ASSERT_FALSE(cli_group_.Parse(argc, argv, cli_nickname_, active_cli));
}