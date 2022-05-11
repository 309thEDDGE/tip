#include <map>
#include <unordered_map>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "cli.h"
#include "parse_text.h"

class CLITest : public ::testing::Test
{
   protected:
    
    std::string label_;
    std::string flag_;
    std::string short_flag_;
    std::string help_str_;
    std::string default_;
    std::string user_str_;
    std::string cli_description_;
    std::string prog_name_;
    std::unordered_map<std::string, int> lookup_;
    CLI cli_; 

    CLITest() : label_("test_arg"), help_str_("none"), default_(""),
        cli_description_("My Test Application"), prog_name_("mycli"), 
        cli_(prog_name_, cli_description_), 
        user_str_(""), flag_(""), short_flag_("")
    { }

};

TEST_F(CLITest, CheckConfigurationNoError)
{
    int output = 0;
    cli_.AddOption("posarg1", "help 1", output);  // positional arg

    std::string str_output;
    cli_.AddOption<std::string>("--flag", "-f", "help 2", "data_default", str_output);  // optional arg

    bool bool_output;
    cli_.AddOption("--my_bool", "", "help3", false, bool_output);  // boolean flag

    ASSERT_TRUE(cli_.CheckConfiguration());
}

TEST_F(CLITest, CheckConfigurationErrorOptionalArg)
{
    int output = 0;
    cli_.AddOption("posarg1", "help 1", output);  // positional arg

    // short flag, two "-" not permitted
    std::string str_output;
    cli_.AddOption<std::string>("--flag", "--f", "help 2", "data_default", str_output);  // optional arg

    bool bool_output;
    cli_.AddOption("--my_bool", "", "help3", false, bool_output);  // boolean flag

    ASSERT_FALSE(cli_.CheckConfiguration());
}

TEST_F(CLITest, CheckConfigurationErrorFlag)
{
    // Positional args always return true.
    int output = 0;
    cli_.AddOption("po[==sarg1", "help 1", output);  // positional arg

    std::string str_output;
    cli_.AddOption<std::string>("--flag", "-f", "help 2", "data_default", str_output);  // optional arg

    // '=' not allowed
    bool bool_output;
    cli_.AddOption("--my_=bool", "", "help3", false, bool_output);  // boolean flag

    ASSERT_FALSE(cli_.CheckConfiguration());
}

TEST_F(CLITest, SortArgs)
{
    int pos_output = 0;
    std::shared_ptr<CLIArg> a1 = CLIPositionalArg<int>::Make("pos1", "help1", pos_output);
    bool flag_output = false;
    std::shared_ptr<CLIArg> a2 = CLIFlag::Make("flag1", "sflag1", "help2", true, flag_output);
    double output = 0.0;
    std::shared_ptr<CLIArg> a3 = CLIOptionalArg<double>::Make("opt1", "sopt1", "help_opt", 31.5, output);

    ArgsVec args{a1, a2, a3};
    size_t pos_argc = 10;
    ArgsVec res = cli_.SortArgs(args, pos_argc);
    ASSERT_EQ(3, res.size());
    ASSERT_EQ(1, pos_argc);
    EXPECT_EQ(CLIArgType::POS, res.at(0)->ArgType());
    EXPECT_EQ(CLIArgType::OPT, res.at(1)->ArgType());
    EXPECT_EQ(CLIArgType::FLAG, res.at(2)->ArgType());
}

TEST_F(CLITest, ParsePositionalArgsTooMany)
{
    int pos_output = 0;
    std::shared_ptr<CLIArg> a1 = CLIPositionalArg<int>::Make("pos1", "help1", pos_output);
    bool flag_output = false;
    std::shared_ptr<CLIArg> a2 = CLIFlag::Make("flag1", "sflag1", "help2", true, flag_output);
    double output = 0.0;
    std::shared_ptr<CLIArg> a3 = CLIOptionalArg<double>::Make("opt1", "sopt1", "help_opt", 31.5, output);

    ArgsVec args{a2, a3};
    std::vector<std::string> pos_args{"data done", "entropy"};
    size_t pos_argc = 1;
    bool res = cli_.ParsePositionalArgs(pos_args, args, pos_argc);
    ASSERT_FALSE(res);
}

TEST_F(CLITest, ParsePositionalArgsInsufficient)
{
    int pos_output = 0;
    std::shared_ptr<CLIArg> a1 = CLIPositionalArg<int>::Make("pos1", "help1", pos_output);

    double pos2_output = 0.0;
    std::shared_ptr<CLIArg> a1a = CLIPositionalArg<double>::Make("pos2", "help2", pos2_output);

    bool flag_output = false;
    std::shared_ptr<CLIArg> a2 = CLIFlag::Make("flag1", "sflag1", "help2", true, flag_output);
    double output = 0.0;
    std::shared_ptr<CLIArg> a3 = CLIOptionalArg<double>::Make("opt1", "sopt1", "help_opt", 31.5, output);

    ArgsVec args{a1, a1a, a2, a3};
    std::vector<std::string> pos_args{"entropy"};
    size_t pos_argc = 2;
    bool res = cli_.ParsePositionalArgs(pos_args, args, pos_argc);
    ASSERT_FALSE(res);
}

TEST_F(CLITest, ParsePositionalArgsFailToCast)
{
    int pos_output = 0;
    std::shared_ptr<CLIArg> a1 = CLIPositionalArg<int>::Make("pos1", "help1", pos_output);

    double pos2_output = 0.0;
    std::shared_ptr<CLIArg> a1a = CLIPositionalArg<double>::Make("pos2", "help2", pos2_output);

    bool flag_output = false;
    std::shared_ptr<CLIArg> a2 = CLIFlag::Make("flag1", "sflag1", "help2", true, flag_output);
    double output = 0.0;
    std::shared_ptr<CLIArg> a3 = CLIOptionalArg<double>::Make("opt1", "sopt1", "help_opt", 31.5, output);

    ArgsVec args{a1, a1a, a2, a3};
    std::vector<std::string> pos_args{"153", "data"};  // can't cast "data" to double
    size_t pos_argc = 2;
    bool res = cli_.ParsePositionalArgs(pos_args, args, pos_argc);
    ASSERT_FALSE(res);
}

TEST_F(CLITest, ParsePositionalArgsPass)
{
    int pos_output = 0;
    std::shared_ptr<CLIArg> a1 = CLIPositionalArg<int>::Make("pos1", "help1", pos_output);

    double pos2_output = 0.0;
    std::shared_ptr<CLIArg> a1a = CLIPositionalArg<double>::Make("pos2", "help2", pos2_output);

    bool flag_output = false;
    std::shared_ptr<CLIArg> a2 = CLIFlag::Make("flag1", "sflag1", "help2", true, flag_output);
    double output = 0.0;
    std::shared_ptr<CLIArg> a3 = CLIOptionalArg<double>::Make("opt1", "sopt1", "help_opt", 31.5, output);

    ArgsVec args{a1, a1a, a2, a3};
    std::vector<std::string> pos_args{"153", "90.5"};  // can't cast "data" to double
    size_t pos_argc = 2;
    bool res = cli_.ParsePositionalArgs(pos_args, args, pos_argc);
    ASSERT_TRUE(res);
    EXPECT_EQ(153, pos_output);
    EXPECT_EQ(90.5, pos2_output);
}

TEST_F(CLITest, ConcatenateUserArgsWhitespaceEncoded)
{
    int pos_output = 0;
    std::shared_ptr<CLIArg> a1 = CLIPositionalArg<int>::Make("pos1", "help1", pos_output);

    double output = 0.0;
    std::shared_ptr<CLIArg> a1a = CLIOptionalArg<double>::Make("--value", "-v", "help double opt", 
        9.3, output);

    bool flag_output = false;
    std::shared_ptr<CLIArg> a2 = CLIFlag::Make("flag1", "sflag1", "help2", true, flag_output);
    double output2 = 0.0;
    std::shared_ptr<CLIArg> a3 = CLIOptionalArg<double>::Make("--opt", "-o", "help_opt", 31.5, output2);

    ArgsVec args{a1, a1a, a2, a3};

    const int arg_count = 7;
    int argc = arg_count;
    char arg1[] = "myprogram";  
    char arg2[] = "1053.65";   
    char arg3[] = "--opt";
    char arg4[] = "blahblah";
    char arg5[] = "-v";
    char arg6[] = "23 is a good value.";
    char arg7[] = "--no-verbose";
    char* argv[arg_count] = {arg1, arg2, arg3, arg4, arg5, arg6, arg7}; 
    std::string expected = "1053.65 --opt blahblah -v 23--..--is--..--a--..--good--..--value. --no-verbose";
    std::string concat_str = cli_.ConcatenateUserArgs(argc, argv, args);
    EXPECT_EQ(expected, concat_str);
}

TEST_F(CLITest, ConcatenateUserArgs)
{
    const int arg_count = 7;
    int argc = arg_count;
    char arg1[] = "myprogram";  
    char arg2[] = "1053.65";   
    char arg3[] = "--entropy";
    char arg4[] = "blahblah";
    char arg5[] = "-B";
    char arg6[] = "23";
    char arg7[] = "--no-verbose";
    char* argv[arg_count] = {arg1, arg2, arg3, arg4, arg5, arg6, arg7}; 
    int pos_argc = 1;
    std::string expected = "--entropy blahblah -B 23 --no-verbose";
    std::string concat_str = cli_.ConcatenateUserArgs(argc, argv, pos_argc);
    EXPECT_EQ(expected, concat_str);
}

TEST_F(CLITest, ParseOptionalArgsNotPresent)
{
    int pos_output = 0;
    std::shared_ptr<CLIArg> a1 = CLIPositionalArg<int>::Make("pos1", "help1", pos_output);
    double pos2_output = 0.0;
    std::shared_ptr<CLIArg> a1a = CLIPositionalArg<double>::Make("pos2", "help2", pos2_output);

    bool flag_output = false;
    std::shared_ptr<CLIArg> a2 = CLIFlag::Make("--flag1", "-f", "help2", true, flag_output);
    double output = 0.0;
    std::shared_ptr<CLIArg> a3 = CLIOptionalArg<double>::Make("--opt1", "-o", "help_opt", 31.5, output);

    ArgsVec args{a1, a1a, a2, a3};
    std::string user_str = "--nintendo five --opt 23.0";
    ASSERT_TRUE(cli_.ParseOptionalArgs(user_str, args));
}

TEST_F(CLITest, ParseOptionalArgsCastFail)
{
    int pos_output = 0;
    std::shared_ptr<CLIArg> a1 = CLIPositionalArg<int>::Make("pos1", "help1", pos_output);
    double pos2_output = 0.0;
    std::shared_ptr<CLIArg> a1a = CLIPositionalArg<double>::Make("pos2", "help2", pos2_output);

    bool flag_output = false;
    std::shared_ptr<CLIArg> a2 = CLIFlag::Make("--flag1", "-f", "help2", true, flag_output);
    double output = 0.0;
    std::shared_ptr<CLIArg> a3 = CLIOptionalArg<double>::Make("--opt1", "-o", "help_opt", 31.5, output);

    ArgsVec args{a1, a1a, a2, a3};
    // "thirty" can't be casted to double
    std::string user_str = "--nintendo five --opt1 thirty";
    ASSERT_FALSE(cli_.ParseOptionalArgs(user_str, args));
}

TEST_F(CLITest, ParseOptionalArgs)
{
    int pos_output = 0;
    std::shared_ptr<CLIArg> a1 = CLIPositionalArg<int>::Make("pos1", "help1", pos_output);
    double pos2_output = 0.0;
    std::shared_ptr<CLIArg> a1a = CLIPositionalArg<double>::Make("pos2", "help2", pos2_output);

    bool flag_output = false;
    std::shared_ptr<CLIArg> a2 = CLIFlag::Make("--flag1", "-f", "help2", true, flag_output);
    double output = 0.0;
    std::shared_ptr<CLIArg> a3 = CLIOptionalArg<double>::Make("--opt1", "-o", "help_opt", 31.5, output);

    ArgsVec args{a1, a1a, a2, a3};
    // "thirty" can't be casted to double
    std::string user_str = "--nintendo five --opt1 30";
    ASSERT_TRUE(cli_.ParseOptionalArgs(user_str, args));
    EXPECT_EQ(30.0, output);
}

TEST_F(CLITest, ParseFlagsNotPresent)
{
    int pos_output = 0;
    std::shared_ptr<CLIArg> a1 = CLIPositionalArg<int>::Make("pos1", "help1", pos_output);
    double pos2_output = 0.0;
    std::shared_ptr<CLIArg> a1a = CLIPositionalArg<double>::Make("pos2", "help2", pos2_output);

    bool flag_output = false;
    std::shared_ptr<CLIArg> a2 = CLIFlag::Make("--flag1", "-f", "help2", true, flag_output);
    double output = 0.0;
    std::shared_ptr<CLIArg> a3 = CLIOptionalArg<double>::Make("--opt1", "-o", "help_opt", 31.5, output);

    ArgsVec args{a1, a1a, a2, a3};
    std::string user_str = "--nintendo five --opt1 30 --flag";
    ASSERT_TRUE(cli_.ParseFlags(user_str, args));
    EXPECT_EQ(true, flag_output);
}

TEST_F(CLITest, ParseFlagsPresent)
{
    int pos_output = 0;
    std::shared_ptr<CLIArg> a1 = CLIPositionalArg<int>::Make("pos1", "help1", pos_output);
    double pos2_output = 0.0;
    std::shared_ptr<CLIArg> a1a = CLIPositionalArg<double>::Make("pos2", "help2", pos2_output);

    bool flag_output = false;
    std::shared_ptr<CLIArg> a2 = CLIFlag::Make("--flag1", "-f", "help2", true, flag_output);
    double output = 0.0;
    std::shared_ptr<CLIArg> a3 = CLIOptionalArg<double>::Make("--opt1", "-o", "help_opt", 31.5, output);

    ArgsVec args{a1, a1a, a2, a3};
    std::string user_str = "--nintendo five --opt1 30 --flag1";
    ASSERT_TRUE(cli_.ParseFlags(user_str, args));
    EXPECT_EQ(false, flag_output);

    user_str = "--nintendo five --opt1 30 -f";
    flag_output = true;
    ASSERT_TRUE(cli_.ParseFlags(user_str, args));
    EXPECT_EQ(false, flag_output);
}

TEST_F(CLITest, ParseNoPosArgs)
{
    std::string output = "";
    cli_.AddOption<std::string>("--opt1", "-o", "help info", "default value", output);
    const int arg_count = 4;
    int argc = arg_count;
    char a0[] = "myprogname";
    char a1[] = "-o";
    char a2[] = "val";
    char a3[] = "data";
    char* argv[arg_count] = {a0, a1, a2, a3};
    ASSERT_TRUE(cli_.Parse(argc, argv));
    EXPECT_EQ(output, "val");
}

TEST_F(CLITest, ParsePosArgFail)
{
    int output = 0;
    cli_.AddOption("position 1", "help1", output);
    const int arg_count = 3;
    int argc = arg_count;
    char a0[] = "progname";
    char a1[] = "--something_else";  // unrecognized
    char a2[] = "data";  
    char* argv[] = {a0, a1, a2};
    ASSERT_FALSE(cli_.Parse(argc, argv));
}

TEST_F(CLITest, MakeHelpString)
{
    std::string opt_output;
    cli_.AddOption<std::string>("--opt_input", "-o", "Optional input", "inputData", opt_output);
    bool flag_output;
    cli_.AddOption("--testflag", "", "Test Flag", false, flag_output);
    int output = 0;
    cli_.AddOption("position1", "help1", output);
    std::string expected(
        "Usage: mycli <positional arguments> [options / flags]\n"
        "       mycli [--opt_input | -o <value>] [--testflag] POSITION1\n\n"
        "My Test Application\n\n"
        "--opt_input, -o Optional input\n"
        "                default: inputData\n\n"
        "--testflag      Test Flag\n"
        "                default: false\n\n"
        "POSITION1       help1\n"
        "                default: none\n\n"
    );
    std::string help_str = cli_.MakeHelpString();
    // printf("observed help string:\n\n%s\n", help_str.c_str());
    ASSERT_EQ(expected, help_str);
}

TEST_F(CLITest, MakeLabelLookupMap)
{
    std::string pos_output = "";
    std::shared_ptr<CLIArg> a1 = CLIPositionalArg<std::string>::Make("str_data", "help1", pos_output);
    bool flag_output = false;
    std::shared_ptr<CLIArg> a2 = CLIFlag::Make("--flag", "-F", "help2", true, flag_output);
    double output = 0.0;
    std::shared_ptr<CLIArg> a3 = CLIOptionalArg<double>::Make("--opt", "-o", "help_opt", 31.5, output);
    double output2 = 0.0;
    std::shared_ptr<CLIArg> a4 = CLIOptionalArg<double>::Make("--test", "", "help_opt", 
        1.7, output2);
    bool flag_output2 = false;
    std::shared_ptr<CLIArg> a5 = CLIFlag::Make("", "-g", "help2", true, flag_output2);
    double output3 = 0.0;
    std::shared_ptr<CLIArg> a6 = CLIOptionalArg<double>::Make("", "-m", "help_opt", 
        1.7, output3);

    bool flag_output3 = false;
    std::shared_ptr<CLIArg> a7 = CLIFlag::Make("--verbose", "", "help2", true, flag_output3);
    int pos_output2 = 0;
    std::shared_ptr<CLIArg> a8 = CLIPositionalArg<int>::Make("int_data", "help_int", pos_output2);
    
    ArgsVec av{a1, a2, a3, a4, a5, a6, a7, a8};
    lookup_ = cli_.MakeLabelLookupMap(av);
    ASSERT_EQ(8, lookup_.size());
    ASSERT_EQ(1, lookup_.count("--flag"));
    ASSERT_EQ(1, lookup_.count("-F"));
    ASSERT_EQ(1, lookup_.count("--opt"));
    ASSERT_EQ(1, lookup_.count("-o"));
    ASSERT_EQ(1, lookup_.count("--test"));
    ASSERT_EQ(1, lookup_.count("-g"));
    ASSERT_EQ(1, lookup_.count("-m"));
    ASSERT_EQ(1, lookup_.count("--verbose"));
    EXPECT_EQ(0, lookup_.at("--flag"));
    EXPECT_EQ(0, lookup_.at("-F"));
    EXPECT_EQ(0, lookup_.at("-g"));
    EXPECT_EQ(0, lookup_.at("--verbose"));
    EXPECT_EQ(1, lookup_.at("--opt"));
    EXPECT_EQ(1, lookup_.at("-o"));
    EXPECT_EQ(1, lookup_.at("-m"));
    EXPECT_EQ(1, lookup_.at("--test"));
}

TEST_F(CLITest, VectorizeUserArgs)
{    
    const int arg_count = 5;
    int argc = arg_count;
    char a0[] = "myprogname";
    char a1[] = "-o";
    char a2[] = "val";
    char a3[] = "data";
    char a4[] = "--test";
    char* argv[arg_count] = {a0, a1, a2, a3, a4};
    std::vector<std::string> expected{"-o", "val", "data", "--test"};
    ASSERT_THAT(expected, ::testing::ContainerEq(cli_.VectorizeUserArgs(argc, argv)));
}

TEST_F(CLITest, ValidateUserInputUnrecogOptArg1)
{
    bool flag_output = false;
    std::shared_ptr<CLIArg> arg2 = CLIFlag::Make("--flag1", "-f", "help2", true, flag_output);
    double output = 0.0;
    std::shared_ptr<CLIArg> arg3 = CLIOptionalArg<double>::Make("--opt1", "-o", "help_opt", 31.5, output);

    ArgsVec args{arg2, arg3};
    const int arg_count = 5;
    int argc = arg_count;
    char a0[] = "programname";
    char a1[] = "five";
    char a2[] = "--opt";
    char a3[] = "30";
    char a4[] = "--flag1";
    char* argv[arg_count] = {a0, a1, a2, a3, a4};

    std::vector<std::string> pos_args;
    ASSERT_FALSE(cli_.ValidateUserInput(argc, argv, args, pos_args));
    EXPECT_EQ(0, pos_args.size());
}

TEST_F(CLITest, ValidateUserInputUnrecogOptArg2)
{
    bool flag_output = false;
    std::shared_ptr<CLIArg> arg2 = CLIFlag::Make("--flag1", "-f", "help2", true, flag_output);
    double output = 0.0;
    std::shared_ptr<CLIArg> arg3 = CLIOptionalArg<double>::Make("--opt1", "-o", "help_opt", 31.5, output);

    ArgsVec args{arg2, arg3};
    const int arg_count = 5;
    int argc = arg_count;
    char a0[] = "programname";
    char a1[] = "five";
    char a2[] = "-O";
    char a3[] = "30";
    char a4[] = "--flag1";
    char* argv[arg_count] = {a0, a1, a2, a3, a4};

    std::vector<std::string> pos_args;
    ASSERT_FALSE(cli_.ValidateUserInput(argc, argv, args, pos_args));
    EXPECT_EQ(0, pos_args.size());
}

TEST_F(CLITest, ValidateUserInputUnrecogFlag1)
{
    bool flag_output = false;
    std::shared_ptr<CLIArg> arg2 = CLIFlag::Make("--flag1", "-f", "help2", true, flag_output);
    bool output = false;
    std::shared_ptr<CLIArg> arg3 = CLIFlag::Make("--opt1", "-o", "help_opt", false, output);

    ArgsVec args{arg2, arg3};
    const int arg_count = 4;
    int argc = arg_count;
    char a0[] = "programname";
    char a1[] = "five";
    char a2[] = "--opt";
    char a4[] = "--flag1";
    char* argv[arg_count] = {a0, a1, a2, a4};

    std::vector<std::string> pos_args;
    ASSERT_FALSE(cli_.ValidateUserInput(argc, argv, args, pos_args));
    EXPECT_EQ(0, pos_args.size());
}

TEST_F(CLITest, ValidateUserInputUnrecogFlag2)
{
    bool flag_output = false;
    std::shared_ptr<CLIArg> arg2 = CLIFlag::Make("--flag1", "-f", "help2", true, flag_output);
    bool output = false;
    std::shared_ptr<CLIArg> arg3 = CLIFlag::Make("--opt1", "-o", "help_opt", false, output);

    ArgsVec args{arg2, arg3};
    const int arg_count = 4;
    int argc = arg_count;
    char a0[] = "programname";
    char a1[] = "five";
    char a2[] = "-O";
    char a4[] = "--flag1";
    char* argv[arg_count] = {a0, a1, a2, a4};

    std::vector<std::string> pos_args;
    ASSERT_FALSE(cli_.ValidateUserInput(argc, argv, args, pos_args));
    EXPECT_EQ(0, pos_args.size());
}

TEST_F(CLITest, ValidateUserInputPosArgsAtBeginning)
{
    bool flag_output = false;
    std::shared_ptr<CLIArg> arg1 = CLIFlag::Make("--flag1", "-f", "help2", true, flag_output);
    std::string opt_output1 = "";
    std::shared_ptr<CLIArg> arg2 = CLIOptionalArg<std::string>::Make("--data", "", "opthelp1",
        "none", opt_output1);
    bool output = false;
    std::shared_ptr<CLIArg> arg3 = CLIFlag::Make("--opt1", "-o", "help_opt", false, output);
    int opt_output2 = 0;
    std::shared_ptr<CLIArg> arg4 = CLIOptionalArg<int>::Make("--int_val", "-I", "opthelp2",
        1, opt_output2);

    ArgsVec args{arg1, arg2, arg3, arg4};
    const int arg_count = 6;
    int argc = arg_count;
    char a0[] = "programname";
    char a1[] = "five";
    char a2[] = "/path/to/data";
    char a3[] = "--flag1";
    char a4[] = "--int_val";
    char a5[] = "333";
    char* argv[arg_count] = {a0, a1, a2, a3, a4, a5};

    std::vector<std::string> pos_args;
    ASSERT_TRUE(cli_.ValidateUserInput(argc, argv, args, pos_args));
    ASSERT_EQ(2, pos_args.size());
    EXPECT_EQ("five", pos_args.at(0));
    EXPECT_EQ("/path/to/data", pos_args.at(1));
}

TEST_F(CLITest, ValidateUserInputPosArgsAtEnd)
{
    bool flag_output = false;
    std::shared_ptr<CLIArg> arg1 = CLIFlag::Make("--flag1", "-f", "help2", true, flag_output);
    std::string opt_output1 = "";
    std::shared_ptr<CLIArg> arg2 = CLIOptionalArg<std::string>::Make("--data", "", "opthelp1",
        "none", opt_output1);
    bool output = false;
    std::shared_ptr<CLIArg> arg3 = CLIFlag::Make("--opt1", "-o", "help_opt", false, output);
    int opt_output2 = 0;
    std::shared_ptr<CLIArg> arg4 = CLIOptionalArg<int>::Make("--int_val", "-I", "opthelp2",
        1, opt_output2);

    ArgsVec args{arg1, arg2, arg3, arg4};
    const int arg_count = 6;
    int argc = arg_count;
    char a0[] = "programname";
    char a1[] = "five";
    char a2[] = "--flag1";
    char a3[] = "--int_val";
    char a4[] = "333";
    char a5[] = "/path/to/data";
    char* argv[arg_count] = {a0, a1, a2, a3, a4, a5};

    std::vector<std::string> pos_args;
    ASSERT_TRUE(cli_.ValidateUserInput(argc, argv, args, pos_args));
    ASSERT_EQ(2, pos_args.size());
    EXPECT_EQ("five", pos_args.at(0));
    EXPECT_EQ("/path/to/data", pos_args.at(1));
}

TEST_F(CLITest, ValidateUserInputPosArgsMixed)
{
    bool flag_output = false;
    std::shared_ptr<CLIArg> arg1 = CLIFlag::Make("--flag1", "-f", "help2", true, flag_output);
    std::string opt_output1 = "";
    std::shared_ptr<CLIArg> arg2 = CLIOptionalArg<std::string>::Make("--data", "", "opthelp1",
        "none", opt_output1);
    bool output = false;
    std::shared_ptr<CLIArg> arg3 = CLIFlag::Make("--opt1", "-o", "help_opt", false, output);
    int opt_output2 = 0;
    std::shared_ptr<CLIArg> arg4 = CLIOptionalArg<int>::Make("--int_val", "-I", "opthelp2",
        1, opt_output2);

    ArgsVec args{arg1, arg2, arg3, arg4};
    const int arg_count = 6;
    int argc = arg_count;
    char a0[] = "programname";
    char a1[] = "--flag1";
    char a2[] = "five";
    char a3[] = "--int_val";
    char a4[] = "333";
    char a5[] = "/path/to/data";
    char* argv[arg_count] = {a0, a1, a2, a3, a4, a5};

    std::vector<std::string> pos_args;
    ASSERT_TRUE(cli_.ValidateUserInput(argc, argv, args, pos_args));
    ASSERT_EQ(2, pos_args.size());
    EXPECT_EQ("five", pos_args.at(0));
    EXPECT_EQ("/path/to/data", pos_args.at(1));
}

TEST_F(CLITest, ValidateUserInputPosArgsAdditionalUnknown)
{
    bool flag_output = false;
    std::shared_ptr<CLIArg> arg1 = CLIFlag::Make("--flag1", "-f", "help2", true, flag_output);
    std::string opt_output1 = "";
    std::shared_ptr<CLIArg> arg2 = CLIOptionalArg<std::string>::Make("--data", "", "opthelp1",
        "none", opt_output1);
    bool output = false;
    std::shared_ptr<CLIArg> arg3 = CLIFlag::Make("--opt1", "-o", "help_opt", false, output);
    int opt_output2 = 0;
    std::shared_ptr<CLIArg> arg4 = CLIOptionalArg<int>::Make("--int_val", "-I", "opthelp2",
        1, opt_output2);

    ArgsVec args{arg1, arg2, arg3, arg4};
    const int arg_count = 7;
    int argc = arg_count;
    char a0[] = "programname";
    char a1[] = "-bad_flag";
    char a2[] = "--flag1";
    char a3[] = "five";
    char a4[] = "--int_val";
    char a5[] = "333";
    char a6[] = "/path/to/data";
    char* argv[arg_count] = {a0, a1, a2, a3, a4, a5, a6};

    std::vector<std::string> pos_args;
    ASSERT_TRUE(cli_.ValidateUserInput(argc, argv, args, pos_args));
    ASSERT_EQ(3, pos_args.size());
    EXPECT_EQ("-bad_flag", pos_args.at(0));
    EXPECT_EQ("five", pos_args.at(1));
    EXPECT_EQ("/path/to/data", pos_args.at(2));
}

TEST_F(CLITest, ValidateUserInputPosArgsWhiteSpace)
{
    bool flag_output = false;
    std::shared_ptr<CLIArg> arg1 = CLIFlag::Make("--flag1", "-f", "help2", true, flag_output);
    std::string opt_output1 = "";
    std::shared_ptr<CLIArg> arg2 = CLIOptionalArg<std::string>::Make("--data", "", "opthelp1",
        "none", opt_output1);
    bool output = false;
    std::shared_ptr<CLIArg> arg3 = CLIFlag::Make("--opt1", "-o", "help_opt", false, output);
    int opt_output2 = 0;
    std::shared_ptr<CLIArg> arg4 = CLIOptionalArg<int>::Make("--int_val", "-I", "opthelp2",
        1, opt_output2);

    ArgsVec args{arg1, arg2, arg3, arg4};
    const int arg_count = 7;
    int argc = arg_count;
    char a0[] = "programname";
    char a1[] = "-o";
    char a2[] = "--flag1";
    char a3[] = "five";
    char a4[] = "--int_val";
    char a5[] = "333";
    char a6[] = "/path/to/my data";
    char* argv[arg_count] = {a0, a1, a2, a3, a4, a5, a6};

    std::vector<std::string> pos_args;
    ASSERT_TRUE(cli_.ValidateUserInput(argc, argv, args, pos_args));
    ASSERT_EQ(2, pos_args.size());
    EXPECT_EQ("five", pos_args.at(0));
    EXPECT_EQ("/path/to/my data", pos_args.at(1));
}

TEST_F(CLITest, ValidateUserInputRepeatArgs)
{
    bool flag_output = false;
    std::shared_ptr<CLIArg> arg1 = CLIFlag::Make("--flag1", "-f", "help2", true, flag_output);
    std::string opt_output1 = "";
    std::shared_ptr<CLIArg> arg2 = CLIOptionalArg<std::string>::Make("--data", "", "opthelp1",
        "none", opt_output1);
    bool output = false;
    std::shared_ptr<CLIArg> arg3 = CLIFlag::Make("--opt1", "-o", "help_opt", false, output);
    int opt_output2 = 0;
    std::shared_ptr<CLIArg> arg4 = CLIOptionalArg<int>::Make("--int_val", "-I", "opthelp2",
        1, opt_output2);

    ArgsVec args{arg1, arg2, arg3, arg4};
    const int arg_count = 11;
    int argc = arg_count;
    char a0[] = "programname";
    char a1[] = "-f";
    char a2[] = "--data";
    char a3[] = "d1";
    char a4[] = "--flag1";
    char a5[] = "five";
    char a6[] = "--int_val";
    char a7[] = "333";
    char a8[] = "--data";
    char a9[] = "d2";
    char a10[] = "/path/to/data";
    char* argv[arg_count] = {a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10};

    std::vector<std::string> pos_args;
    ASSERT_TRUE(cli_.ValidateUserInput(argc, argv, args, pos_args));
    ASSERT_EQ(2, pos_args.size());
    EXPECT_EQ("five", pos_args.at(0));
    EXPECT_EQ("/path/to/data", pos_args.at(1));
}

TEST_F(CLITest, IsFlagOrOptArgCandidate)
{
    std::string input = "-x";
    EXPECT_TRUE(cli_.IsFlagOrOptArgCandidate(input));

    input = "-xx";
    EXPECT_FALSE(cli_.IsFlagOrOptArgCandidate(input));

    input = "--xx";
    EXPECT_TRUE(cli_.IsFlagOrOptArgCandidate(input));

    input = "--x";
    EXPECT_FALSE(cli_.IsFlagOrOptArgCandidate(input));
}

TEST_F(CLITest, MakeHelpStringWithSimpleFormat)
{
    std::string opt_output;
    cli_.AddOption<std::string>("--opt_input", "-o", "Optional input", "inputData", opt_output);
    bool flag_output;
    std::string flag_help = R"(This is a special formatted:
  - test1
  - test2
more data  to   come!)";
    cli_.AddOption("--testflag", "", flag_help, false, flag_output)->SetSimpleHelpFormat();
    int output = 0;
    cli_.AddOption("position1", "help1", output);
    std::string expected(
        "Usage: mycli <positional arguments> [options / flags]\n"
        "       mycli [--opt_input | -o <value>] [--testflag] POSITION1\n\n"
        "My Test Application\n\n"
        "--opt_input, -o Optional input\n"
        "                default: inputData\n\n"
        "--testflag      This is a special formatted:\n"
        "                  - test1\n"
        "                  - test2\n"
        "                more data  to   come!\n"
        "                default: false\n\n"
        "POSITION1       help1\n"
        "                default: none\n\n"
    );
    std::string help_str = cli_.MakeHelpString();
    ASSERT_EQ(expected, help_str);
}

TEST_F(CLITest, ParseMultipleVecInputs)
{
    bool flag_output = false;
    cli_.AddOption("--flag1", "-f", "help2", true, flag_output);
    std::vector<int> output;
    std::vector<int> default_val{1, 10};
    cli_.AddOption<std::vector<int>>("--opt1", "-o", "help_opt", default_val, output);
    ASSERT_TRUE(cli_.CheckConfiguration());

    const int arg_count = 7;
    int argc = arg_count;
    char a0[] = "programname";
    char a1[] = "-f";
    char a2[] = "-o";
    char a3[] = "30";
    char a4[] = "posarg";
    char a5[] = "--opt1";
    char a6[] = "45";
    char* argv[arg_count] = {a0, a1, a2, a3, a4, a5, a6};

    ASSERT_TRUE(cli_.Parse(argc, argv));
    EXPECT_EQ(2, output.size());
    EXPECT_EQ(30, output.at(0));
    EXPECT_EQ(45, output.at(1));
}