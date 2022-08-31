#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "parse_text.h"
#include <yaml-cpp/yaml.h>

class ParseTextTest : public ::testing::Test
{
   protected:
    ParseText pt;
    bool res;
    std::string test_str;
    std::vector<std::string> compare_vec;
    std::vector<std::string> return_vec;
    std::map<int, std::string> quoted_sections;
    std::map<int, std::string> unquoted_sections;

    ParseTextTest() : test_str("")
    {
    }
    void SetUp() override
    {
    }
};

TEST_F(ParseTextTest, ExtractQuotedSectionsCompleteQuotes)
{
    // Complete single set of quotes.
    test_str = "A,\"B\",C";
    res = pt.ExtractQuotedSections(test_str, quoted_sections, unquoted_sections);
    EXPECT_EQ(res, true);

    // Multiple complete sets of quotes.
    test_str = "A,\"B\",C,\"D\",E,\"F\"";
    res = pt.ExtractQuotedSections(test_str, quoted_sections, unquoted_sections);
    EXPECT_EQ(res, true);

    // Unmatches quotes.
    test_str = "A,\"B\",C,\"D\",E,\"F";
    res = pt.ExtractQuotedSections(test_str, quoted_sections, unquoted_sections);
    EXPECT_EQ(res, false);
}

TEST_F(ParseTextTest, ExtractQuotedSectionsBeginQuoted)
{
    test_str = "\"B\",C,A";
    res = pt.ExtractQuotedSections(test_str, quoted_sections, unquoted_sections);
    EXPECT_EQ(res, true);
    EXPECT_EQ(quoted_sections[0], "B");
    EXPECT_EQ(unquoted_sections[1], ",C,A");

    test_str += ",\"test\",found";
    quoted_sections.clear();
    unquoted_sections.clear();
    res = pt.ExtractQuotedSections(test_str, quoted_sections, unquoted_sections);
    EXPECT_EQ(res, true);
    EXPECT_EQ(quoted_sections[0], "B");
    EXPECT_EQ(unquoted_sections[1], ",C,A,");
    EXPECT_EQ(quoted_sections[2], "test");
    EXPECT_EQ(unquoted_sections[3], ",found");
}

TEST_F(ParseTextTest, ExtractQuotedSectionsEndQuoted)
{
    test_str = "B,C,\"A\"";
    res = pt.ExtractQuotedSections(test_str, quoted_sections, unquoted_sections);
    EXPECT_EQ(res, true);
    EXPECT_EQ(unquoted_sections[0], "B,C,");
    EXPECT_EQ(quoted_sections[1], "A");

    test_str += ",test,\"found\"";
    quoted_sections.clear();
    unquoted_sections.clear();
    res = pt.ExtractQuotedSections(test_str, quoted_sections, unquoted_sections);
    EXPECT_EQ(res, true);
    EXPECT_EQ(unquoted_sections[0], "B,C,");
    EXPECT_EQ(quoted_sections[1], "A");
    EXPECT_EQ(unquoted_sections[2], ",test,");
    EXPECT_EQ(quoted_sections[3], "found");
}

TEST_F(ParseTextTest, ExtractQuotedSectionsMiddleQuoted)
{
    test_str = "B,\"C\",A";
    res = pt.ExtractQuotedSections(test_str, quoted_sections, unquoted_sections);
    EXPECT_EQ(res, true);
    EXPECT_EQ(unquoted_sections[0], "B,");
    EXPECT_EQ(quoted_sections[1], "C");
    EXPECT_EQ(unquoted_sections[2], ",A");
}

TEST_F(ParseTextTest, ExtractQuotedSectionsBeginEndQuoted)
{
    test_str = "\"B\",C,A,\"D\"";
    res = pt.ExtractQuotedSections(test_str, quoted_sections, unquoted_sections);
    EXPECT_EQ(res, true);
    EXPECT_EQ(quoted_sections[0], "B");
    EXPECT_EQ(unquoted_sections[1], ",C,A,");
    EXPECT_EQ(quoted_sections[2], "D");

    test_str += ",tried and true,\"haha\"";
    quoted_sections.clear();
    unquoted_sections.clear();
    res = pt.ExtractQuotedSections(test_str, quoted_sections, unquoted_sections);
    EXPECT_EQ(res, true);
    EXPECT_EQ(quoted_sections[0], "B");
    EXPECT_EQ(unquoted_sections[1], ",C,A,");
    EXPECT_EQ(quoted_sections[2], "D");
    EXPECT_EQ(unquoted_sections[3], ",tried and true,");
    EXPECT_EQ(quoted_sections[4], "haha");
}

TEST_F(ParseTextTest, SplitHandlesEmptyLine)
{
    test_str = "";
    return_vec = pt.Split(test_str, '|');
    ASSERT_EQ(return_vec.size(), 0);
}

TEST_F(ParseTextTest, SplitNoDelimiter)
{
    test_str = "ABCD";
    return_vec = pt.Split(test_str, '|');
    compare_vec.push_back("ABCD");
    ASSERT_TRUE(compare_vec == return_vec);
}

TEST_F(ParseTextTest, SplitTwoDelimiter)
{
    test_str = "ABCD|EFGH";
    return_vec = pt.Split(test_str, '|');
    compare_vec.push_back("ABCD");
    compare_vec.push_back("EFGH");
    ASSERT_TRUE(compare_vec == return_vec);
}

TEST_F(ParseTextTest, SplitDifferentDelimiter)
{
    test_str = "AA|BB,20";
    return_vec = pt.Split(test_str, ',');
    compare_vec.push_back("AA|BB");
    compare_vec.push_back("20");
    ASSERT_TRUE(compare_vec == return_vec);
}

TEST_F(ParseTextTest, SplitEmptyString)
{
    test_str = "AA||20";
    return_vec = pt.Split(test_str, '|');
    compare_vec.push_back("AA");
    compare_vec.push_back("");
    compare_vec.push_back("20");
    ASSERT_TRUE(compare_vec == return_vec);
    compare_vec.clear();

    test_str = "AA|BB|";
    return_vec = pt.Split(test_str, '|');
    compare_vec.push_back("AA");
    compare_vec.push_back("BB");
    compare_vec.push_back("");
    ASSERT_TRUE(compare_vec == return_vec);
    compare_vec.clear();

    test_str = "|AA|BB";
    return_vec = pt.Split(test_str, '|');
    compare_vec.push_back("");
    compare_vec.push_back("AA");
    compare_vec.push_back("BB");
    ASSERT_TRUE(compare_vec == return_vec);
}

TEST_F(ParseTextTest, SplitRemoveNewLineAtEnd)
{
    test_str = "AA|BB|30\n";
    return_vec = pt.Split(test_str, '|');
    compare_vec.push_back("AA");
    compare_vec.push_back("BB");
    compare_vec.push_back("30");
    ASSERT_TRUE(compare_vec == return_vec);
    compare_vec.clear();

    test_str = "AA|BB\n|30\n";
    return_vec = pt.Split(test_str, '|');
    compare_vec.push_back("AA");
    compare_vec.push_back("BB\n");
    compare_vec.push_back("30");
    ASSERT_TRUE(compare_vec == return_vec);
    compare_vec.clear();

    test_str = "AA\n";
    return_vec = pt.Split(test_str, '|');
    compare_vec.push_back("AA");
    ASSERT_TRUE(compare_vec == return_vec);
}

TEST_F(ParseTextTest, SplitRemoveCarriageReturnAtEnd)
{
    test_str = "AA|BB|30\r";
    return_vec = pt.Split(test_str, '|');
    compare_vec.push_back("AA");
    compare_vec.push_back("BB");
    compare_vec.push_back("30");
    ASSERT_TRUE(compare_vec == return_vec);
    compare_vec.clear();

    test_str = "AA|BB\r|30\r";
    return_vec = pt.Split(test_str, '|');
    compare_vec.push_back("AA");
    compare_vec.push_back("BB\r");
    compare_vec.push_back("30");
    ASSERT_TRUE(compare_vec == return_vec);
    compare_vec.clear();

    test_str = "AA\r";
    return_vec = pt.Split(test_str, '|');
    compare_vec.push_back("AA");
    ASSERT_TRUE(compare_vec == return_vec);
}

TEST_F(ParseTextTest, SplitOmitQuotedText)
{
    test_str = "ABCD,hello,\"my test, is good\"";
    return_vec = pt.Split(test_str, ',');
    EXPECT_EQ(return_vec[0], "ABCD");
    EXPECT_EQ(return_vec[1], "hello");
    EXPECT_EQ(return_vec[2], "my test, is good");
}

TEST_F(ParseTextTest, SplitRetainQuotedText)
{
    test_str = "ABCD,hello,\"my test, is good\"";
    return_vec = pt.Split(test_str, ',', true);
    EXPECT_EQ(return_vec[0], "ABCD");
    EXPECT_EQ(return_vec[1], "hello");
    EXPECT_EQ(return_vec[2], "\"my test, is good\"");
}

TEST_F(ParseTextTest, SplitOmitQuotedText2)
{
    test_str = "ABCD,hello \"my test, is good\"";
    return_vec = pt.Split(test_str, ',', true);
    ASSERT_EQ(2, return_vec.size());
    EXPECT_EQ(return_vec[0], "ABCD");
    EXPECT_EQ(return_vec[1], "hello \"my test, is good\"");
}

TEST_F(ParseTextTest, SplitOmitQuotedText3)
{
    test_str = "ABCD,hello \"my test, is good\"";
    return_vec = pt.Split(test_str, ',', false);
    ASSERT_EQ(2, return_vec.size());
    EXPECT_EQ(return_vec[0], "ABCD");
    EXPECT_EQ(return_vec[1], "hello my test, is good");
}

TEST_F(ParseTextTest, SplitOmitQuotedText4)
{
    test_str = "ABCD,hello \"my test, is good\" all right";
    return_vec = pt.Split(test_str, ',', false);
    ASSERT_EQ(2, return_vec.size());
    EXPECT_EQ(return_vec[0], "ABCD");
    EXPECT_EQ(return_vec[1], "hello my test, is good all right");
}

TEST_F(ParseTextTest, SplitIgnoreQuotedTextIfNoDelimiter)
{
    test_str = "ABCD,hello \"my test is good\"";
    return_vec = pt.Split(test_str, ',', true);
    ASSERT_EQ(2, return_vec.size());
    EXPECT_EQ(return_vec[0], "ABCD");
    EXPECT_EQ(return_vec[1], "hello \"my test is good\"");
}

TEST_F(ParseTextTest, SplitIgnoreQuotedTextIfNoDelimiter2)
{
    test_str = "ABCD,hello \"my test is good\"";
    return_vec = pt.Split(test_str, ',', false);
    ASSERT_EQ(2, return_vec.size());
    EXPECT_EQ(return_vec[0], "ABCD");
    EXPECT_EQ(return_vec[1], "hello my test is good");
}

TEST_F(ParseTextTest, SplitAvoidMultipleQuotedText)
{
    test_str = "ABCD,hello,\"my test, is good\",g8,\"end, or build\",time";
    return_vec = pt.Split(test_str, ',');
    EXPECT_EQ(return_vec[0], "ABCD");
    EXPECT_EQ(return_vec[1], "hello");
    EXPECT_EQ(return_vec[2], "my test, is good");
    EXPECT_EQ(return_vec[3], "g8");
    EXPECT_EQ(return_vec[4], "end, or build");
    EXPECT_EQ(return_vec[5], "time");
}

TEST_F(ParseTextTest, SplitAllDelimitedSectionsQuoted)
{
    test_str =
        "\"msg_name\",\"element_name\",\"xmit_word\",\"dest_word\","
        "\"msg_word_count\",\"bus_name\",\"xmit_lru_name\",\"xmit_lru_addr\","
        "\"dest_lru_name\",\"dest_lru_addr\",\"xmit_lru_subaddr\",\"dest_lru_subaddr\","
        "\"rate\",\"offset\",\"elem_word_count\",\"schema\",\"is_bitlevel\","
        "\"is_multiformat\",\"bitmsb\",\"bitlsb\",\"bit_count\",\"classification\","
        "\"description\",\"msb_val\",\"uom\"";
    return_vec = pt.Split(test_str, ',');
    std::vector<std::string> expected = {"msg_name", "element_name", "xmit_word",
                                         "dest_word", "msg_word_count", "bus_name", "xmit_lru_name", "xmit_lru_addr",
                                         "dest_lru_name", "dest_lru_addr", "xmit_lru_subaddr", "dest_lru_subaddr",
                                         "rate", "offset", "elem_word_count", "schema", "is_bitlevel", "is_multiformat",
                                         "bitmsb", "bitlsb", "bit_count", "classification", "description", "msb_val", "uom"};
    ASSERT_EQ(return_vec, expected);
}

TEST_F(ParseTextTest, ConvertIntNegative)
{
    test_str = "-20";
    int return_val;
    bool return_status = pt.ConvertInt(test_str, return_val);
    ASSERT_TRUE(return_status);
    ASSERT_EQ(return_val, -20);
}

TEST_F(ParseTextTest, ConvertIntNonNegative)
{
    test_str = "123456789";
    int return_val;
    bool return_status = pt.ConvertInt(test_str, return_val);
    ASSERT_TRUE(return_status);
    ASSERT_EQ(return_val, 123456789);
}

TEST_F(ParseTextTest, ConvertIntSingleNegative)
{
    test_str = "-";
    int return_val;
    bool return_status = pt.ConvertInt(test_str, return_val);
    ASSERT_FALSE(return_status);
    ASSERT_EQ(return_val, 0);
}

TEST_F(ParseTextTest, ConvertIntNegativeInternalPlacement)
{
    test_str = "2-";
    int return_val;
    bool return_status = pt.ConvertInt(test_str, return_val);
    ASSERT_FALSE(return_status);
    ASSERT_EQ(return_val, 0);

    test_str = "21-11";
    return_status = pt.ConvertInt(test_str, return_val);
    ASSERT_FALSE(return_status);
    ASSERT_EQ(return_val, 0);
}

TEST_F(ParseTextTest, ConvertIntEmptyString)
{
    test_str = "";
    int return_val;
    bool return_status = pt.ConvertInt(test_str, return_val);
    ASSERT_FALSE(return_status);
    ASSERT_EQ(return_val, 0);
}

TEST_F(ParseTextTest, CheckForExisitingCharAsExpected)
{
    test_str = "a jk  #";
    ASSERT_TRUE(pt.CheckForExisitingChar(test_str, '#'));

    test_str = "# ajb";
    ASSERT_TRUE(pt.CheckForExisitingChar(test_str, '#'));

    ASSERT_FALSE(pt.CheckForExisitingChar(test_str, 't'));
}

TEST_F(ParseTextTest, RemoveTrailingCharAsExpected)
{
    // Remove comma.
    test_str = "a jk  #,";
    ASSERT_EQ(pt.RemoveTrailingChar(test_str, ','), "a jk  #");

    // Remove newline.
    test_str = "# ajb\n";
    ASSERT_EQ(pt.RemoveTrailingChar(test_str, '\n'), "# ajb");

    // Don't remove non-trailing char.
    test_str = "# ajb";
    ASSERT_EQ(pt.RemoveTrailingChar(test_str, 'j'), test_str);
}

TEST_F(ParseTextTest, ConvertDoubleNegative)
{
    test_str = "-20.3";
    double return_val;
    bool return_status = pt.ConvertDouble(test_str, return_val);
    ASSERT_TRUE(return_status);
    ASSERT_EQ(return_val, double(-20.3));
}

TEST_F(ParseTextTest, ConvertDoubleNonNegative)
{
    test_str = "123456789.0";
    double return_val;
    bool return_status = pt.ConvertDouble(test_str, return_val);
    ASSERT_TRUE(return_status);
    ASSERT_EQ(return_val, double(123456789.0));
}

TEST_F(ParseTextTest, ConvertDoubleSingleNegative)
{
    test_str = "-";
    double return_val;
    bool return_status = pt.ConvertDouble(test_str, return_val);
    ASSERT_FALSE(return_status);
    ASSERT_EQ(return_val, double(0.0));
}

TEST_F(ParseTextTest, ConvertDoubleNegativeInternalPlacement)
{
    test_str = "2.345-";
    double return_val;
    bool return_status = pt.ConvertDouble(test_str, return_val);
    ASSERT_FALSE(return_status);
    ASSERT_EQ(return_val, double(0.0));

    test_str = "21-11.553322";
    return_status = pt.ConvertDouble(test_str, return_val);
    ASSERT_FALSE(return_status);
    ASSERT_EQ(return_val, double(0.0));
}

TEST_F(ParseTextTest, ConvertDoubleEmptyString)
{
    test_str = "";
    double return_val;
    bool return_status = pt.ConvertDouble(test_str, return_val);
    ASSERT_FALSE(return_status);
    ASSERT_EQ(return_val, double(0.0));
}

TEST_F(ParseTextTest, ConvertDoubleScientificNotation)
{
    test_str = "2.36118324E+21";
    double return_val;
    bool return_status = pt.ConvertDouble(test_str, return_val);
    ASSERT_TRUE(return_status);
    ASSERT_EQ(return_val, double(2.36118324E+21));
}

TEST_F(ParseTextTest, TextIsIntegerInvalidString)
{
    // hyphen only
    test_str = "-";
    bool return_status = pt.TextIsInteger(test_str);
    ASSERT_FALSE(return_status);

    // empty string
    test_str = "";
    return_status = pt.TextIsInteger(test_str);
    ASSERT_FALSE(return_status);

    // floating point
    test_str = "543.213";
    return_status = pt.TextIsInteger(test_str);
    ASSERT_FALSE(return_status);
}

TEST_F(ParseTextTest, TextIsIntegerValidString)
{
    test_str = "32";
    bool return_status = pt.TextIsInteger(test_str);
    ASSERT_TRUE(return_status);
}

TEST_F(ParseTextTest, TextIsFloatInvalidString)
{
    // hyphen only
    test_str = "-";
    bool return_status = pt.TextIsFloat(test_str);
    ASSERT_FALSE(return_status);

    // empty string
    test_str = "";
    return_status = pt.TextIsFloat(test_str);
    ASSERT_FALSE(return_status);
}

TEST_F(ParseTextTest, TextIsFloatValidString)
{
    // Regular float
    test_str = "345.211";
    bool return_status = pt.TextIsFloat(test_str);
    ASSERT_TRUE(return_status);

    // We don't want to discriminate strings that
    // could be interpreted as integers because
    // it may be necessary to interpret them as
    // float/double.
    test_str = "3455";
    return_status = pt.TextIsFloat(test_str);
    ASSERT_TRUE(return_status);
}

TEST_F(ParseTextTest, TextIsFloatScientific)
{
    // scientific notation, E
    test_str = "345.211E+3";
    bool return_status = pt.TextIsFloat(test_str);
    EXPECT_TRUE(return_status);

    // scientific notation, e
    test_str = "34879.4412e+8";
    return_status = pt.TextIsFloat(test_str);
    EXPECT_TRUE(return_status);

    // multiple Es
    test_str = "345.211EE+3";
    return_status = pt.TextIsFloat(test_str);
    EXPECT_FALSE(return_status);

    test_str = "348E79.4412e+8";
    return_status = pt.TextIsFloat(test_str);
    EXPECT_FALSE(return_status);

    // Non-e character
    test_str = "3479.4412f+8";
    return_status = pt.TextIsFloat(test_str);
    EXPECT_FALSE(return_status);
}

TEST_F(ParseTextTest, IsASCIIEmptyString)
{
    test_str = "";
    res = pt.IsASCII(test_str);
    EXPECT_FALSE(res);
}

TEST_F(ParseTextTest, IsASCIIInvalidChar)
{
    const int n = 1;
    char vals[n] = {char(130)};  // 130 not in [0, 127]
    test_str = std::string((const char*)&vals, n);
    res = pt.IsASCII(test_str);
    EXPECT_FALSE(res);
}

TEST_F(ParseTextTest, IsASCIIInvalidSequence)
{
    const int n = 5;
    char vals[n] = {23, 56, char(144), 0, 10};  // 144 not in [0, 127]
    test_str = std::string((const char*)&vals, n);
    res = pt.IsASCII(test_str);
    EXPECT_FALSE(res);

    vals[2] = 44;
    vals[0] = -10;
    test_str = std::string((const char*)&vals, n);
    res = pt.IsASCII(test_str);
    EXPECT_FALSE(res);
}

TEST_F(ParseTextTest, IsASCIIValidSequence)
{
    const int n = 5;
    char vals[n] = {23, 56, 44, 0, 10};  // 130 not in [0, 127]
    test_str = std::string((const char*)&vals, n);
    res = pt.IsASCII(test_str);
    EXPECT_TRUE(res);
}

TEST_F(ParseTextTest, IsUTF8EmptyString)
{
    test_str = "";
    res = pt.IsUTF8(test_str);
    EXPECT_FALSE(res);
}

/*
Examples of well-formted utf-8 byte sequences, from which ill-formed
sequences are generated for testing: 
http://www.unicode.org/versions/Unicode6.2.0/ch03.pdf
*/

TEST_F(ParseTextTest, IsUTF8InvalidFirstByte)
{
    // Greater than max first byte.
    test_str = "\xF5";
    EXPECT_FALSE(pt.IsUTF8(test_str));

    // > 1B first byte and < 2B smallest
    test_str = "\x82";
    EXPECT_FALSE(pt.IsUTF8(test_str));
}

TEST_F(ParseTextTest, IsUTF8InvalidSecondByte)
{
    // Valid 1B seq first byte, second byte belongs
    // to 2B sequence.
    test_str = "\x70\x81";
    EXPECT_FALSE(pt.IsUTF8(test_str));

    // [0xC2, 0xDF] 2B sequence, 2nd byte small
    test_str = "\xC5\x2G";
    EXPECT_FALSE(pt.IsUTF8(test_str));

    // [0xC2, 0xDF] 2B sequence, 2nd byte large
    test_str = "\xC5\xD7";
    EXPECT_FALSE(pt.IsUTF8(test_str));

    // 0xE0 3B sequence, 2nd byte small
    test_str = "\xE0\x87";
    EXPECT_FALSE(pt.IsUTF8(test_str));

    // 0xE0 3B sequence, 2nd byte large
    test_str = "\xE0\xD6";
    EXPECT_FALSE(pt.IsUTF8(test_str));

    // [0xE1, 0xEC] 3B sequence, 2nd byte small
    test_str = "\xE2\x6A";
    EXPECT_FALSE(pt.IsUTF8(test_str));

    // [0xE1, 0xEC] 3B sequence, 2nd byte large
    test_str = "\xE2\xCC";
    EXPECT_FALSE(pt.IsUTF8(test_str));

    // 0xED 3B sequence, 2nd byte small
    test_str = "\xED\x4A";
    EXPECT_FALSE(pt.IsUTF8(test_str));

    // 0xED 3B sequence, 2nd byte large
    test_str = "\xED\xB2";
    EXPECT_FALSE(pt.IsUTF8(test_str));

    // [0xEE, 0xEF] 3B sequence, 2nd byte small
    test_str = "\xEE\x4A";
    EXPECT_FALSE(pt.IsUTF8(test_str));

    // [0xEE, 0xEF] 3B sequence, 2nd byte small
    test_str = "\xEF\xD0";
    EXPECT_FALSE(pt.IsUTF8(test_str));

    // 0xF0 4B sequence, 2nd byte small
    test_str = "\xF0\x7A";
    EXPECT_FALSE(pt.IsUTF8(test_str));

    // 0xF0 4B sequence, 2nd byte large
    test_str = "\xF0\xC3";
    EXPECT_FALSE(pt.IsUTF8(test_str));

    // [0xF1, 0xF3] 4B sequence, 2nd byte small
    test_str = "\xF2\x55";
    EXPECT_FALSE(pt.IsUTF8(test_str));

    // [0xF1, 0xF3] 4B sequence, 2nd byte large
    test_str = "\xF3\xF7";
    EXPECT_FALSE(pt.IsUTF8(test_str));

    // 0xF4 4B sequence, 2nd byte small
    test_str = "\xF4\x55";
    EXPECT_FALSE(pt.IsUTF8(test_str));

    // 0xF4 4B sequence, 2nd byte large
    test_str = "\xF4\x90";
    EXPECT_FALSE(pt.IsUTF8(test_str));
}

TEST_F(ParseTextTest, IsUTF8InvalidThirdByte)
{
    // valid 2B sequence with 3B third byte
    test_str = "\xC7\xA8\x87";
    EXPECT_FALSE(pt.IsUTF8(test_str));

    // 0xE0 3B sequence, 3rd byte small
    test_str = "\xE0\xA0\x77";
    EXPECT_FALSE(pt.IsUTF8(test_str));

    // 0xE0 3B sequence, 3rd byte large
    test_str = "\xE0\xA0\xC3";
    EXPECT_FALSE(pt.IsUTF8(test_str));

    // [0xE1, 0xEC] 3B sequence, 3rd byte small
    test_str = "\xE1\xA0\x77";
    EXPECT_FALSE(pt.IsUTF8(test_str));

    // [0xE1, 0xEC] 3B sequence, 3rd byte large
    test_str = "\xE1\xA0\xC0";
    EXPECT_FALSE(pt.IsUTF8(test_str));

    // 0xED 3B sequence, 3rd byte small
    test_str = "\xED\x95\x77";
    EXPECT_FALSE(pt.IsUTF8(test_str));

    // 0xED 3B sequence, 3rd byte large
    test_str = "\xED\x95\xD6";
    EXPECT_FALSE(pt.IsUTF8(test_str));

    // [0xEE, 0xEF] 3B sequence, 3rd byte small
    test_str = "\xEE\xA0\x77";
    EXPECT_FALSE(pt.IsUTF8(test_str));

    // [0xEE, 0xEF] 3B sequence, 3rd byte large
    test_str = "\xEE\xA0\xD9";
    EXPECT_FALSE(pt.IsUTF8(test_str));

    // 0xF0 4B sequence, 3rd byte small
    test_str = "\xF0\x97\x77";
    EXPECT_FALSE(pt.IsUTF8(test_str));

    // 0xF0 4B sequence, 3rd byte large
    test_str = "\xF0\x97\xEE";
    EXPECT_FALSE(pt.IsUTF8(test_str));

    // [0xF1, 0xF3] 4B sequence, 3rd byte small
    test_str = "\xF1\xA0\x77";
    EXPECT_FALSE(pt.IsUTF8(test_str));

    // [0xF1, 0xF3] 4B sequence, 3rd byte large
    test_str = "\xF1\xA0\xF2";
    EXPECT_FALSE(pt.IsUTF8(test_str));

    // 0xF4 4B sequence, 3rd byte small
    test_str = "\xF4\x97\x77";
    EXPECT_FALSE(pt.IsUTF8(test_str));

    // 0xF4 4B sequence, 3rd byte large
    test_str = "\xF4\x97\xC7";
    EXPECT_FALSE(pt.IsUTF8(test_str));
}

TEST_F(ParseTextTest, IsUTF8InvalidFourthByte)
{
    // Valid 3B sequence with 4B sequence fourth byte.
    test_str = "\xE3\xA2\xA5\xB4";
    EXPECT_FALSE(pt.IsUTF8(test_str));

    // 0xF0 4B sequence, fourth byte small
    test_str = "\xF0\xAA\x89\x4F";
    EXPECT_FALSE(pt.IsUTF8(test_str));

    // 0xF0 4B sequence, fourth byte large
    test_str = "\xF0\xAA\x89\xC6";
    EXPECT_FALSE(pt.IsUTF8(test_str));

    // [0xF1, 0xF3] 4B sequence, fourth byte small
    test_str = "\xF2\x87\x89\x4F";
    EXPECT_FALSE(pt.IsUTF8(test_str));

    // [0xF1, 0xF3] 4B sequence, fourth byte large
    test_str = "\xF2\x87\x89\xD2";
    EXPECT_FALSE(pt.IsUTF8(test_str));

    // 0xF4 4B sequence, fourth byte small
    test_str = "\xF4\x88\x89\x4F";
    EXPECT_FALSE(pt.IsUTF8(test_str));

    // 0xF4 4B sequence, fourth byte large
    test_str = "\xF4\x88\x89\xCB";
    EXPECT_FALSE(pt.IsUTF8(test_str));
}

TEST_F(ParseTextTest, IsUTF8IncreasingSuccession)
{
    // Valid increasing succession.
    // 1B + 2B + 3B + 4B
    test_str = std::string("\x5D") + std::string("\xC4\xA2") +
               std::string("\xEB\x88\x9D") + std::string("\xF0\x91\x87\xBA");
    EXPECT_TRUE(pt.IsUTF8(test_str));

    // Invalid 1B seq
    test_str = std::string("\x92") + std::string("\xC4\xA2") +
               std::string("\xEB\x88\x9D") + std::string("\xF0\x91\x87\xBA");
    EXPECT_FALSE(pt.IsUTF8(test_str));

    // Invalid 2B seq
    test_str = std::string("\x5D") + std::string("\xC4\x5D") +
               std::string("\xEB\x88\x9D") + std::string("\xF0\x91\x87\xBA");
    EXPECT_FALSE(pt.IsUTF8(test_str));

    // Invalid 3B seq
    test_str = std::string("\x5D") + std::string("\xC4\xA2") +
               std::string("\xEB\xC3\x9D") + std::string("\xF0\x91\x87\xBA");
    EXPECT_FALSE(pt.IsUTF8(test_str));

    // Invalid 4B seq
    test_str = std::string("\x5D") + std::string("\xC4\xA2") +
               std::string("\xEB\x88\x9D") + std::string("\xF0\x91\x87\xCA");
    EXPECT_FALSE(pt.IsUTF8(test_str));
}

TEST_F(ParseTextTest, IsUTF8DecreasingSuccession)
{
    // Valid increasing succession.
    // 4B + 3B + 2B + 1B
    test_str = std::string("\xF0\x91\x87\xBA") + std::string("\xEB\x88\x9D") +
               std::string("\xC4\xA2") + std::string("\x5D");
    EXPECT_TRUE(pt.IsUTF8(test_str));

    // Invalid 1B seq
    test_str = std::string("\xF0\x91\x87\xBA") + std::string("\xEB\x88\x9D") +
               std::string("\xC4\xA2") + std::string("\x92");
    EXPECT_FALSE(pt.IsUTF8(test_str));

    // Invalid 2B seq
    test_str = std::string("\xF0\x91\x87\xBA") + std::string("\xEB\x88\x9D") +
               std::string("\xC4\x5D") + std::string("\x5D");
    EXPECT_FALSE(pt.IsUTF8(test_str));

    // Invalid 3B seq
    test_str = std::string("\xF0\x91\x87\xBA") + std::string("\xEB\xC3\x9D") +
               std::string("\xC4\xA2") + std::string("\x5D");
    EXPECT_FALSE(pt.IsUTF8(test_str));

    // Invalid 4B seq
    test_str = std::string("\xF0\x91\x87\xCA") + std::string("\xEB\x88\x9D") +
               std::string("\xC4\xA2") + std::string("\x5D");
    EXPECT_FALSE(pt.IsUTF8(test_str));
}

TEST_F(ParseTextTest, ToLowerEmptyString)
{
    test_str = "";
    std::string ret_str = pt.ToLower(test_str);
    EXPECT_EQ(ret_str, test_str);
}

TEST_F(ParseTextTest, ToLowerAllLower)
{
    test_str = "this is a string_ with all lower-case";
    std::string ret_str = pt.ToLower(test_str);
    EXPECT_EQ(ret_str, test_str);
}

TEST_F(ParseTextTest, ToLowerMixedCase)
{
    test_str = "This is A String_ with MIXed case";
    std::string ret_str = pt.ToLower(test_str);
    EXPECT_EQ(ret_str, "this is a string_ with mixed case");
}

TEST_F(ParseTextTest, ToLowerAllUpper)
{
    test_str = "THIS IS A STRING_ WITH ALL UPPER CASE";
    std::string ret_str = pt.ToLower(test_str);
    EXPECT_EQ(ret_str, "this is a string_ with all upper case");
}

TEST_F(ParseTextTest, ToUpperEmptyString)
{
    test_str = "";
    std::string ret_str = pt.ToUpper(test_str);
    EXPECT_EQ(ret_str, test_str);
}

TEST_F(ParseTextTest, ToUpperAllUpper)
{
    test_str = "ALL UPPER ALREADY!!";
    std::string ret_str = pt.ToUpper(test_str);
    EXPECT_EQ(ret_str, test_str);
}

TEST_F(ParseTextTest, ToUpperMixedCase)
{
    test_str = "This is A String_ with MIXed case";
    std::string ret_str = pt.ToUpper(test_str);
    EXPECT_EQ(ret_str, "THIS IS A STRING_ WITH MIXED CASE");
}

TEST_F(ParseTextTest, ToUpperAllLower)
{
    test_str = "this is a string_ with all lower";
    std::string ret_str = pt.ToUpper(test_str);
    EXPECT_EQ(ret_str, "THIS IS A STRING_ WITH ALL LOWER");
}

TEST_F(ParseTextTest, SplitStringEmpty)
{
    test_str = "";
    return_vec = pt.Split(test_str, "] ");
    ASSERT_EQ(0, return_vec.size());
}

TEST_F(ParseTextTest, SplitStringKeepDelim)
{
    test_str = "helptada this is my string which tada now";
    return_vec = pt.Split(test_str, "tada ", true);
    compare_vec = std::vector<std::string>{"helptada", "this is my string which tada", "now"};
    ASSERT_EQ(3, return_vec.size());
    EXPECT_THAT(compare_vec, ::testing::ContainerEq(return_vec));
}

TEST_F(ParseTextTest, SplitStringWithQuotesKeepDelim)
{
    test_str = "helptada this \"is my string\" which tada now";
    return_vec = pt.Split(test_str, "tada ", true);
    compare_vec = std::vector<std::string>{"helptada", "this \"is my string\" which tada", "now"};
    ASSERT_EQ(3, return_vec.size());
    EXPECT_THAT(compare_vec, ::testing::ContainerEq(return_vec));
}

TEST_F(ParseTextTest, SplitStringWithQuotesKeepDelim2)
{
    test_str = "\"helptada this\" \"is my string\" which tada now";
    return_vec = pt.Split(test_str, "tada ", true);
    compare_vec = std::vector<std::string>{"\"helptada", "this\" \"is my string\" which tada", "now"};
    ASSERT_EQ(3, return_vec.size());
    EXPECT_THAT(compare_vec, ::testing::ContainerEq(return_vec));
}

TEST_F(ParseTextTest, SplitStringLeadingSeparatorKeepDelim1)
{
    test_str = "tada helptada this is my string which tada now";
    return_vec = pt.Split(test_str, "tada ", true);
    compare_vec = std::vector<std::string>{"tada", "helptada", "this is my string which tada", "now"};
    ASSERT_EQ(4, return_vec.size());
    EXPECT_THAT(compare_vec, ::testing::ContainerEq(return_vec));
}

TEST_F(ParseTextTest, SplitStringLeadingSeparatorKeepDelim2)
{
    test_str = "tadahelptada this is my string which tada now";
    return_vec = pt.Split(test_str, "tada ", true);
    compare_vec = std::vector<std::string>{"tadahelptada", "this is my string which tada", "now"};
    ASSERT_EQ(3, return_vec.size());
    EXPECT_THAT(compare_vec, ::testing::ContainerEq(return_vec));
}

TEST_F(ParseTextTest, SplitStringTrailingSeparatorKeepDelim1)
{
    test_str = "tada helptada this is my string which tada nowtada";
    return_vec = pt.Split(test_str, "tada ", true);
    compare_vec = std::vector<std::string>{"tada", "helptada", 
        "this is my string which tada", "nowtada"};
    ASSERT_EQ(4, return_vec.size());
    EXPECT_THAT(compare_vec, ::testing::ContainerEq(return_vec));
}

TEST_F(ParseTextTest, SplitStringTrailingSeparatorKeepDelim2)
{
    test_str = "tada helptada this is my string which tada nowtada ";
    return_vec = pt.Split(test_str, "tada ", true);
    compare_vec = std::vector<std::string>{"tada", "helptada", 
        "this is my string which tada", "nowtada"};
    ASSERT_EQ(4, return_vec.size());
    EXPECT_THAT(compare_vec, ::testing::ContainerEq(return_vec));
}

/////////////////////////////////////////

TEST_F(ParseTextTest, SplitString)
{
    test_str = "helptada this is my string which tada now";
    return_vec = pt.Split(test_str, "tada ", false);
    compare_vec = std::vector<std::string>{"help", "this is my string which ", "now"};
    ASSERT_EQ(3, return_vec.size());
    EXPECT_THAT(compare_vec, ::testing::ContainerEq(return_vec));
}

TEST_F(ParseTextTest, SplitStringWithQuotes)
{
    test_str = "helptada this \"is my string\" which tada now";
    return_vec = pt.Split(test_str, "tada ", false);
    compare_vec = std::vector<std::string>{"help", "this \"is my string\" which ", "now"};
    ASSERT_EQ(3, return_vec.size());
    EXPECT_THAT(compare_vec, ::testing::ContainerEq(return_vec));
}

TEST_F(ParseTextTest, SplitStringWithQuotes2)
{
    test_str = "\"helptada this\" \"is my string\" which tada now";
    return_vec = pt.Split(test_str, "tada ", false);
    compare_vec = std::vector<std::string>{"\"help", "this\" \"is my string\" which ", "now"};
    ASSERT_EQ(3, return_vec.size());
    EXPECT_THAT(compare_vec, ::testing::ContainerEq(return_vec));
}

TEST_F(ParseTextTest, SplitStringLeadingSeparator1)
{
    test_str = "tada helptada this is my string which tada now";
    return_vec = pt.Split(test_str, "tada ", false);
    compare_vec = std::vector<std::string>{"help", "this is my string which ", "now"};
    ASSERT_EQ(3, return_vec.size());
    EXPECT_THAT(compare_vec, ::testing::ContainerEq(return_vec));
}

TEST_F(ParseTextTest, SplitStringLeadingSeparator2)
{
    test_str = "tadahelptada this is my string which tada now";
    return_vec = pt.Split(test_str, "tada ", false);
    compare_vec = std::vector<std::string>{"tadahelp", "this is my string which ", "now"};
    ASSERT_EQ(3, return_vec.size());
    EXPECT_THAT(compare_vec, ::testing::ContainerEq(return_vec));
}

TEST_F(ParseTextTest, SplitStringTrailingSeparator1)
{
    test_str = "tada helptada this is my string which tada nowtada";
    return_vec = pt.Split(test_str, "tada ", false);
    compare_vec = std::vector<std::string>{"help", "this is my string which ", "nowtada"};
    ASSERT_EQ(3, return_vec.size());
    EXPECT_THAT(compare_vec, ::testing::ContainerEq(return_vec));
}

TEST_F(ParseTextTest, SplitStringTrailingSeparator2)
{
    test_str = "tada helptada this is my string which tada nowtada ";
    return_vec = pt.Split(test_str, "tada ", false);
    compare_vec = std::vector<std::string>{"help", "this is my string which ", "now"};
    ASSERT_EQ(3, return_vec.size());
    EXPECT_THAT(compare_vec, ::testing::ContainerEq(return_vec));
}


////////////////////////////////
TEST_F(ParseTextTest, SplitMultipleWhitespace1)
{
    test_str = "hello, this    is   a line with various       whitespace  characters.";
    compare_vec = std::vector<std::string>{"hello,", "this", "is", "a", 
        "line", "with", "various", "whitespace", "characters."};
    return_vec = pt.Split(test_str);
    ASSERT_EQ(9, return_vec.size());
    EXPECT_THAT(compare_vec, ::testing::ContainerEq(return_vec));
}

TEST_F(ParseTextTest, SplitMultipleWhitespace2)
{
    test_str = "   hello, this    is   a line with various       whitespace  characters.";
    compare_vec = std::vector<std::string>{"hello,", "this", "is", "a", 
        "line", "with", "various", "whitespace", "characters."};
    return_vec = pt.Split(test_str);
    ASSERT_EQ(9, return_vec.size());
    EXPECT_THAT(compare_vec, ::testing::ContainerEq(return_vec));
}

TEST_F(ParseTextTest, SplitMultipleWhitespace3)
{
    test_str = "hello, this    is   a line with various       whitespace  characters.  ";
    compare_vec = std::vector<std::string>{"hello,", "this", "is", "a", 
        "line", "with", "various", "whitespace", "characters."};
    return_vec = pt.Split(test_str);
    ASSERT_EQ(9, return_vec.size());
    EXPECT_THAT(compare_vec, ::testing::ContainerEq(return_vec));
}

TEST_F(ParseTextTest, SplitMultipleWhitespace4)
{
    test_str = " hello, this    is   a line with various       whitespace  characters.  ";
    compare_vec = std::vector<std::string>{"hello,", "this", "is", "a", 
        "line", "with", "various", "whitespace", "characters."};
    return_vec = pt.Split(test_str);
    ASSERT_EQ(9, return_vec.size());
    EXPECT_THAT(compare_vec, ::testing::ContainerEq(return_vec));
}

TEST_F(ParseTextTest, SplitMultipleWhitespaceEmptyString)
{
    test_str = "";
    compare_vec = std::vector<std::string>();
    return_vec = pt.Split(test_str);
    ASSERT_EQ(0, return_vec.size());
}

TEST_F(ParseTextTest, SplitMultipleWhitespaceNoSpaces)
{
    test_str = "this_isAstring_with-no-spaces";
    compare_vec = std::vector<std::string>{"this_isAstring_with-no-spaces"};
    return_vec = pt.Split(test_str);
    ASSERT_EQ(1, return_vec.size());
    EXPECT_THAT(compare_vec, ::testing::ContainerEq(return_vec));
}

TEST_F(ParseTextTest, JoinEmptyVec)
{
    std::vector<std::string> input_vec;
    test_str = pt.Join(input_vec);
    ASSERT_EQ("", test_str);
}

TEST_F(ParseTextTest, Join)
{
    std::vector<std::string> input_vec{"all", "about", "the", "data"};
    test_str = pt.Join(input_vec);
    std::string expected = "all about the data";
    ASSERT_EQ(expected, test_str);
}

TEST_F(ParseTextTest, ReplaceSubNotPresent)
{
    test_str = "my tale of woe";
    std::string find_str = "data";
    std::string repl_str = "BIG";
    std::string expected = test_str;
    ASSERT_EQ(expected, pt.Replace(test_str, find_str, repl_str));
}

TEST_F(ParseTextTest, ReplaceEmptyString)
{
    test_str = "";
    std::string find_str = "data";
    std::string repl_str = "BIG";
    std::string expected = test_str;
    ASSERT_EQ(expected, pt.Replace(test_str, find_str, repl_str));
}

TEST_F(ParseTextTest, ReplaceWord)
{
    test_str = "my tale of data";
    std::string find_str = "data";
    std::string repl_str = "BIG";
    std::string expected = "my tale of BIG";
    ASSERT_EQ(expected, pt.Replace(test_str, find_str, repl_str));
}

TEST_F(ParseTextTest, ReplaceSub)
{
    test_str = "my tale of data";
    std::string find_str = "le";
    std::string repl_str = "boo";
    std::string expected = "my taboo of data";
    ASSERT_EQ(expected, pt.Replace(test_str, find_str, repl_str));
}

TEST_F(ParseTextTest, ReplaceMultiple)
{
    test_str = "my tale of data at level for le";
    std::string find_str = "le";
    std::string repl_str = "bee";
    std::string expected = "my tabee of data at beevel for bee";
    ASSERT_EQ(expected, pt.Replace(test_str, find_str, repl_str));
}