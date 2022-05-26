#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "cli_arg.h"


class CLIArgTest : public ::testing::Test
{
   protected:
    
    std::string temp_label_;
    std::string temp_help_str_;
    std::string default_;
    std::string expected_help_str_;
    int temp_char_width_;
    std::string fmt_str_;
    std::string user_str_;
    std::string separator_;
    CLIArg cli_arg_; 

    CLIArgTest() : temp_label_("test_arg"), temp_help_str_("none"), default_(""),
        cli_arg_(temp_label_, temp_help_str_, default_), temp_char_width_(0), fmt_str_(""),
        expected_help_str_(""), user_str_(""), separator_(" ")
    { }

};

TEST_F(CLIArgTest, FormatStringWSInsufficientWidth)
{
    temp_char_width_ = 39; // minimum 40
    size_t indent = 0;
    ASSERT_FALSE(cli_arg_.FormatString(temp_help_str_, temp_char_width_, indent, 
        fmt_str_, separator_));
    EXPECT_EQ(fmt_str_, "");
}

TEST_F(CLIArgTest, FormatStringWSSingleLine)
{
    temp_char_width_ = 40; 
    temp_help_str_ = "this is a simple single line help";   // 33
    size_t indent = 0;
    ASSERT_TRUE(cli_arg_.FormatString(temp_help_str_, temp_char_width_, 
        indent, fmt_str_, separator_));
    EXPECT_EQ(temp_help_str_, fmt_str_);
    temp_help_str_ = "this is a simple single line help string";   // 40, on the border 
    ASSERT_TRUE(cli_arg_.FormatString(temp_help_str_, temp_char_width_, 
        indent, fmt_str_, separator_));
    EXPECT_EQ(temp_help_str_, fmt_str_);
}

TEST_F(CLIArgTest, FormatStringWSSingleLineWithNewline)
{
    temp_char_width_ = 40; 
    temp_help_str_ = "this is a simple\nsingle line help";   // 33
    expected_help_str_ = "this is a simple single line help";
    size_t indent = 0;
    ASSERT_TRUE(cli_arg_.FormatString(temp_help_str_, temp_char_width_, 
        indent, fmt_str_, separator_));
    EXPECT_EQ(expected_help_str_, fmt_str_);
}

TEST_F(CLIArgTest, FormatStringWSMultiLine)
{
    temp_char_width_ = 40; 
    temp_help_str_ = "this is a simple single line help message which should span two lines";   // 69
    expected_help_str_ = "this is a simple single line help\nmessage which should span two lines";
    size_t indent = 0;
    ASSERT_TRUE(cli_arg_.FormatString(temp_help_str_, temp_char_width_, 
        indent, fmt_str_, separator_));
    EXPECT_EQ(expected_help_str_, fmt_str_);
}

TEST_F(CLIArgTest, FormatStringWSMultiLinePreExistingNewlines)
{
    temp_char_width_ = 40; 
    temp_help_str_ = "this is a simple\n single line help message which should span two lines";   // 69
    expected_help_str_ = "this is a simple single line help\nmessage which should span two lines";
    size_t indent = 0;
    ASSERT_TRUE(cli_arg_.FormatString(temp_help_str_, temp_char_width_, 
        indent, fmt_str_, separator_));
    EXPECT_EQ(expected_help_str_, fmt_str_);
}

TEST_F(CLIArgTest, FormatStringWSIndentMultLines)
{
    temp_char_width_ = 60; 
    temp_help_str_ = "this is a simple\n single line help message which should span two lines";   // 69
    expected_help_str_ = (
    "                         this is a simple single line help\n"
    "                         message which should span two\n"
    "                         lines"
    );
    size_t indent = 25;
    ASSERT_TRUE(cli_arg_.FormatString(temp_help_str_, temp_char_width_, 
        indent, fmt_str_, separator_));
    EXPECT_EQ(expected_help_str_, fmt_str_);
}

TEST_F(CLIArgTest, FormatStringNonCharSeparator)
{
    temp_char_width_ = 80; 
    size_t indent = 15;
    separator_ = "] ";
    temp_help_str_ = (
        "INPUT_PATH [--output_path | -o <value>] [--log_path | -l <value>]"
        " [--chunk_size | -c <value>] [--thread_count | -t <value>]"
        " [--max_read_count | -m <value>] [--worker_offset <value>] [--worker_wait <value>]"
    );
    expected_help_str_ = (
        "               INPUT_PATH [--output_path | -o <value>]\n"
        "               [--log_path | -l <value>] [--chunk_size | -c <value>]\n"
        "               [--thread_count | -t <value>] [--max_read_count | -m <value>]\n"
        "               [--worker_offset <value>] [--worker_wait <value>]"
    );

    ASSERT_TRUE(cli_arg_.FormatString(temp_help_str_, temp_char_width_, 
        indent, fmt_str_, separator_));
    EXPECT_EQ(expected_help_str_, fmt_str_);
}

TEST_F(CLIArgTest, FormatStringNonCharMultipleWS)
{
    temp_char_width_ = 80; 
    size_t indent = 15;
    separator_ = "] ";
    temp_help_str_ = (
        "INPUT_PATH [--output_path | -o <value>]   [--log_path | -l <value>]"
        "      [--chunk_size | -c <value>]   [--thread_count | -t <value>]"
        " [--max_read_count | -m <value>] [--worker_offset <value>]  [--worker_wait <value>]"
    );
    expected_help_str_ = (
        "               INPUT_PATH [--output_path | -o <value>]\n"
        "               [--log_path | -l <value>] [--chunk_size | -c <value>]\n"
        "               [--thread_count | -t <value>] [--max_read_count | -m <value>]\n"
        "               [--worker_offset <value>] [--worker_wait <value>]"
    );

    ASSERT_TRUE(cli_arg_.FormatString(temp_help_str_, temp_char_width_, 
        indent, fmt_str_, separator_));
    EXPECT_EQ(expected_help_str_, fmt_str_);
}

TEST_F(CLIArgTest, FormatStringIndentOnlySingleLine)
{
    temp_help_str_ = "this is a simple single line help";
    expected_help_str_ = "          this is a simple single line help";
    size_t indent = 10;
    ASSERT_TRUE(cli_arg_.FormatString(temp_help_str_, indent, fmt_str_));
    EXPECT_EQ(expected_help_str_, fmt_str_);
}

TEST_F(CLIArgTest, FormatStringIndentOnlyMultipleLine)
{
    temp_help_str_ = R"(this is a simple single line help
with a new line here!)";

    expected_help_str_ = R"(          this is a simple single line help
          with a new line here!)";
    size_t indent = 10;
    ASSERT_TRUE(cli_arg_.FormatString(temp_help_str_, indent, fmt_str_));
    EXPECT_EQ(expected_help_str_, fmt_str_);
}

TEST_F(CLIArgTest, FormatStringIndentOnlyWithQuotes)
{
    temp_help_str_ = R"(this is a simple "single line" help
with a new line and quotes!)";

    expected_help_str_ = R"(          this is a simple "single line" help
          with a new line and quotes!)";
    size_t indent = 10;
    ASSERT_TRUE(cli_arg_.FormatString(temp_help_str_, indent, fmt_str_));
    EXPECT_EQ(expected_help_str_, fmt_str_);
}

TEST_F(CLIArgTest, FormatStringNewlineAtMaxWidth)
{
    temp_help_str_ = R"(this is a simple "single line" help without a new line and with quotes!)";
    expected_help_str_ = R"(this is a simple "single line" help
without a new line and with quotes!)";
    size_t indent = 0;
    ASSERT_TRUE(cli_arg_.FormatString(temp_help_str_, 43, indent, fmt_str_, " "));
    EXPECT_EQ(expected_help_str_, fmt_str_);
}

TEST_F(CLIArgTest, IdentifyArgumentEmptyString)
{
    user_str_ = "";
    temp_label_ = "";
    std::string parsed = "not empty";
    ASSERT_FALSE(cli_arg_.IdentifyArgument(user_str_, temp_label_, parsed));
    EXPECT_EQ("", parsed);
}

TEST_F(CLIArgTest, IdentifyArgumentNoLabelMatch)
{
    user_str_ = "a string --of --args";
    temp_label_ = "--arg";  // does not match "--args"
    std::string parsed = "not empty";
    ASSERT_FALSE(cli_arg_.IdentifyArgument(user_str_, temp_label_, parsed));
    EXPECT_EQ("", parsed);
}

TEST_F(CLIArgTest, IdentifyArgumentLabelMatchNoInput)
{
    user_str_ = "a string --of --args";
    temp_label_ = "--args";  // matches "--args", but no following argument
    std::string parsed = "not empty";
    ASSERT_FALSE(cli_arg_.IdentifyArgument(user_str_, temp_label_, parsed));
    EXPECT_EQ("", parsed);
}

TEST_F(CLIArgTest, IdentifyArgumentMatch1)
{
    user_str_ = "a string --of --args val";
    temp_label_ = "--args";  
    std::string parsed = "not empty";
    ASSERT_TRUE(cli_arg_.IdentifyArgument(user_str_, temp_label_, parsed));
    EXPECT_EQ("val", parsed);
}

TEST_F(CLIArgTest, IdentifyArgumentMatch2)
{
    user_str_ = "a string of --args val --data 123";
    temp_label_ = "--args";  
    std::string parsed = "not empty";
    ASSERT_TRUE(cli_arg_.IdentifyArgument(user_str_, temp_label_, parsed));
    EXPECT_EQ("val", parsed);
}

TEST_F(CLIArgTest, IdentifyArgumentMatch3)
{
    user_str_ = "a string of --args val --data 123 --arg 55";
    temp_label_ = "--arg";  
    std::string parsed = "not empty";
    ASSERT_TRUE(cli_arg_.IdentifyArgument(user_str_, temp_label_, parsed));
    EXPECT_EQ("55", parsed);
}

TEST_F(CLIArgTest, IdentifyArgumentMatch4)
{
    user_str_ = "a string of --args val --data 123 -a 55";
    temp_label_ = "-a";  
    std::string parsed = "not empty";
    ASSERT_TRUE(cli_arg_.IdentifyArgument(user_str_, temp_label_, parsed));
    EXPECT_EQ("55", parsed);
}

TEST_F(CLIArgTest, IdentifyArgumentMatch5)
{
    user_str_ = "a string of --args \"val of six\" --data 123 -a 55";
    temp_label_ = "--args";  
    std::string parsed = "not empty";
    ASSERT_TRUE(cli_arg_.IdentifyArgument(user_str_, temp_label_, parsed));
    EXPECT_EQ("val of six", parsed);
}

TEST_F(CLIArgTest, IdentifyArgumentMatch6)
{
    user_str_ = "a string of --args \"val of six\" --data \"145\" -a 55";
    temp_label_ = "--data";  
    std::string parsed = "not empty";
    ASSERT_TRUE(cli_arg_.IdentifyArgument(user_str_, temp_label_, parsed));
    EXPECT_EQ("145", parsed);
}

TEST_F(CLIArgTest, CastArgumentIntWithFloatInput)
{
    user_str_ = "23.0";
    int output = 0;
    ASSERT_FALSE(cli_arg_.CastArgument(user_str_, output));
}

TEST_F(CLIArgTest, CastArgumentInt)
{
    user_str_ = "23";
    int output = 0;
    ASSERT_TRUE(cli_arg_.CastArgument(user_str_, output));
    EXPECT_EQ(23, output);
}

TEST_F(CLIArgTest, CastArgumentString)
{
    user_str_ = "23";
    std::string output = "0";
    ASSERT_TRUE(cli_arg_.CastArgument(user_str_, output));
    EXPECT_EQ(user_str_, output);
}

TEST_F(CLIArgTest, CastArgumentDoubleWithIntInput)
{
    user_str_ = "f23";
    double output = 0.0;
    ASSERT_FALSE(cli_arg_.CastArgument(user_str_, output));
}

TEST_F(CLIArgTest, CastArgumentDouble)
{
    user_str_ = "23";
    double output = 0.0;
    ASSERT_TRUE(cli_arg_.CastArgument(user_str_, output));
    EXPECT_EQ(23.0, output);
}

TEST_F(CLIArgTest, IsAllowedCharTrue)
{
    char c = 'A';
    ASSERT_TRUE(cli_arg_.IsAllowedChar(c));

    c = 'r';
    ASSERT_TRUE(cli_arg_.IsAllowedChar(c));

    c = '1';
    ASSERT_TRUE(cli_arg_.IsAllowedChar(c));

    c = '_';
    ASSERT_TRUE(cli_arg_.IsAllowedChar(c));
}

TEST_F(CLIArgTest, IsAllowedCharFalse)
{
    char c = '-';
    ASSERT_FALSE(cli_arg_.IsAllowedChar(c));

    c = '=';
    ASSERT_FALSE(cli_arg_.IsAllowedChar(c));

    c = '/';
    ASSERT_FALSE(cli_arg_.IsAllowedChar(c));

    c = '\\';
    ASSERT_FALSE(cli_arg_.IsAllowedChar(c));

    c = '[';
    ASSERT_FALSE(cli_arg_.IsAllowedChar(c));
}

TEST_F(CLIArgTest, CheckLabelLeadingHyphens)
{
    temp_label_ = "data";
    ASSERT_FALSE(cli_arg_.CheckLabel(temp_label_));

    temp_label_ = "-data";  // need two hyphen
    ASSERT_FALSE(cli_arg_.CheckLabel(temp_label_));

    temp_label_ = "--data";  
    ASSERT_TRUE(cli_arg_.CheckLabel(temp_label_));
}

TEST_F(CLIArgTest, CheckLabelNoInternalHyphens)
{
    temp_label_ = "--data-set";
    ASSERT_FALSE(cli_arg_.CheckLabel(temp_label_));

    temp_label_ = "--data_set";  
    ASSERT_TRUE(cli_arg_.CheckLabel(temp_label_));
}

TEST_F(CLIArgTest, CheckLabelNoQuotesOrSpaces)
{
    temp_label_ = "--data set";
    ASSERT_FALSE(cli_arg_.CheckLabel(temp_label_));

    temp_label_ = "--\"data_set\"";  
    ASSERT_FALSE(cli_arg_.CheckLabel(temp_label_));
}

TEST_F(CLIArgTest, CheckShortLabel)
{
    temp_label_ = "--t";
    ASSERT_FALSE(cli_arg_.CheckShortLabel(temp_label_));

    temp_label_ = "-to";  
    ASSERT_FALSE(cli_arg_.CheckShortLabel(temp_label_));

    temp_label_ = "to";  
    ASSERT_FALSE(cli_arg_.CheckShortLabel(temp_label_));

    temp_label_ = "-j";  
    ASSERT_TRUE(cli_arg_.CheckShortLabel(temp_label_));

    temp_label_ = "-]";  
    ASSERT_FALSE(cli_arg_.CheckShortLabel(temp_label_));

    temp_label_ = "-_";  
    ASSERT_FALSE(cli_arg_.CheckShortLabel(temp_label_));
}

TEST_F(CLIArgTest, FindEscapedComponentsNoEscapeChars)
{
    temp_help_str_ = "lorem ipsum dolor sit amet, eu dicunt animal tamquam vis.";
    std::map<int, std::string> nonescaped;
    std::map<int, std::string> escaped;
    ASSERT_FALSE(cli_arg_.FindEscapedComponents(temp_help_str_, nonescaped, escaped));
    ASSERT_EQ(0, nonescaped.size());
    ASSERT_EQ(0, escaped.size());
}

TEST_F(CLIArgTest, FindEscapedComponentsNotInPairs)
{
    // three sets of escape sequence, NOT 4
    temp_help_str_ = "lorem **||ipsum dolor sit amet,**|| eu dicunt animal **||tamquam vis.**|*";
    std::map<int, std::string> nonescaped;
    std::map<int, std::string> escaped;
    ASSERT_FALSE(cli_arg_.FindEscapedComponents(temp_help_str_, nonescaped, escaped));
    ASSERT_EQ(0, nonescaped.size());
    ASSERT_EQ(0, escaped.size());
}

TEST_F(CLIArgTest, FindEscapedComponentsBeginsWith)
{
    temp_help_str_ = "**||lorem **||ipsum dolor sit amet,**|| eu dicunt animal **||tamquam vis.";
    std::map<int, std::string> nonescaped;
    std::map<int, std::string> escaped;
    ASSERT_TRUE(cli_arg_.FindEscapedComponents(temp_help_str_, nonescaped, escaped));
    ASSERT_EQ(2, nonescaped.size());
    ASSERT_EQ(2, escaped.size());

    std::map<int, std::string> expected_nonescaped{
        {1, "ipsum dolor sit amet,"},
        {3, "tamquam vis."}
    };
    std::map<int, std::string> expected_escaped{
        {0, "lorem "},
        {2, " eu dicunt animal "}
    };
    EXPECT_THAT(expected_nonescaped, ::testing::ContainerEq(nonescaped));
    EXPECT_THAT(expected_escaped, ::testing::ContainerEq(escaped));
}

TEST_F(CLIArgTest, FindEscapedComponentsEndsWith)
{
    temp_help_str_ = "lorem **||ipsum dolor sit amet,**|| eu dicunt animal **||tamquam vis.**||";
    std::map<int, std::string> nonescaped;
    std::map<int, std::string> escaped;
    ASSERT_TRUE(cli_arg_.FindEscapedComponents(temp_help_str_, nonescaped, escaped));
    ASSERT_EQ(2, nonescaped.size());
    ASSERT_EQ(2, escaped.size());

    std::map<int, std::string> expected_nonescaped{
        {0, "lorem "},
        {2, " eu dicunt animal "}
    };
    std::map<int, std::string> expected_escaped{
        {1, "ipsum dolor sit amet,"},
        {3, "tamquam vis."}
    };
    EXPECT_THAT(expected_nonescaped, ::testing::ContainerEq(nonescaped));
    EXPECT_THAT(expected_escaped, ::testing::ContainerEq(escaped));
}

TEST_F(CLIArgTest, FindEscapedComponentsBeginsAndEndsWith)
{
    temp_help_str_ = "**||lorem **||ipsum **||dolor sit amet,**|| eu dicunt animal **||tamquam vis.**||";
    std::map<int, std::string> nonescaped;
    std::map<int, std::string> escaped;
    ASSERT_TRUE(cli_arg_.FindEscapedComponents(temp_help_str_, nonescaped, escaped));
    ASSERT_EQ(2, nonescaped.size());
    ASSERT_EQ(3, escaped.size());

    std::map<int, std::string> expected_nonescaped{
        {1, "ipsum "},
        {3, " eu dicunt animal "}
    };
    std::map<int, std::string> expected_escaped{
        {0, "lorem "},
        {2, "dolor sit amet,"},
        {4, "tamquam vis."}
    };
    EXPECT_THAT(expected_nonescaped, ::testing::ContainerEq(nonescaped));
    EXPECT_THAT(expected_escaped, ::testing::ContainerEq(escaped));
}

TEST_F(CLIArgTest, GetHelpStringSpecialEscapeChars)
{
    temp_char_width_ = 50;
    temp_help_str_ = R"(lorem ipsum dolor sit amet, eu dicunt animal tamquam vis.
et saperet sensibus mea, ei dicam praesent pro.**|| Veri sonet dicant quo ei, tamquam numquam salutandi
ea vim.**|| Possim definitionem quo no. Idque aperiam mea id.)";
    expected_help_str_ = (
        "          lorem ipsum dolor sit amet, eu dicunt\n"
        "          animal tamquam vis. et saperet sensibus\n"
        "          mea, ei dicam praesent pro.\n"
        "           Veri sonet dicant quo ei, tamquam numquam salutandi\n"
        "          ea vim.\n"
        "          Possim definitionem quo no. Idque\n"
        "          aperiam mea id."
    );
    size_t indent = 10;
    CLIArg cliarg(temp_label_, temp_help_str_, default_);
    ASSERT_TRUE(cliarg.GetHelpString(temp_char_width_, indent, fmt_str_));
    EXPECT_EQ(expected_help_str_, fmt_str_);
}

