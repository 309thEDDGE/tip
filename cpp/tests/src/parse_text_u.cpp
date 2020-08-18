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

TEST_F(ParseTextTest, SplitAvoidQuotedText)
{
	test_str = "ABCD,hello,\"my test, is good\"";
	return_vec = pt.Split(test_str, ',');
	EXPECT_EQ(return_vec[0], "ABCD");
	EXPECT_EQ(return_vec[1], "hello");
	EXPECT_EQ(return_vec[2], "my test, is good");
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
	test_str = "\"msg_name\",\"element_name\",\"xmit_word\",\"dest_word\","
		"\"msg_word_count\",\"bus_name\",\"xmit_lru_name\",\"xmit_lru_addr\","
		"\"dest_lru_name\",\"dest_lru_addr\",\"xmit_lru_subaddr\",\"dest_lru_subaddr\","
		"\"rate\",\"offset\",\"elem_word_count\",\"schema\",\"is_bitlevel\","
		"\"is_multiformat\",\"bitmsb\",\"bitlsb\",\"bit_count\",\"classification\","
		"\"description\",\"msb_val\",\"uom\"";
	return_vec = pt.Split(test_str, ',');
	std::vector<std::string> expected = { "msg_name","element_name","xmit_word",
		"dest_word","msg_word_count","bus_name","xmit_lru_name","xmit_lru_addr",
		"dest_lru_name","dest_lru_addr","xmit_lru_subaddr","dest_lru_subaddr",
		"rate","offset","elem_word_count","schema","is_bitlevel","is_multiformat",
		"bitmsb","bitlsb","bit_count","classification","description","msb_val","uom" };
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

