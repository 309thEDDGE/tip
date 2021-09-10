#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "uri_percent_encoding.h"

class URIPercentEncodingTest : public ::testing::Test
{
    protected:
        URIPercentEncoding uripe_;
        std::stringstream enc_stream_;
        char temp_char_;
        std::string temp_string_;
        std::string expected_string_;
        bool ret_;
        std::string output_string_;
        URIPercentEncodingTest() : uripe_(), enc_stream_(), temp_char_('a'), 
            expected_string_(""), temp_string_(""), ret_(false), output_string_("")
        {
        }
};

TEST_F(URIPercentEncodingTest, EncodeCharPadded)
{
    temp_char_ = '\n'; // 0x0a
    uripe_.EncodeChar(temp_char_, enc_stream_);
    expected_string_ = "%0a";
    ASSERT_EQ(expected_string_, enc_stream_.str());
}

TEST_F(URIPercentEncodingTest, EncodeCharNonASCII)
{
    temp_char_ = static_cast<char>(0x8c); 
    uripe_.EncodeChar(temp_char_, enc_stream_);
    expected_string_ = "%8c";
    ASSERT_EQ(expected_string_, enc_stream_.str());
}

TEST_F(URIPercentEncodingTest, AppendChar)
{
    temp_char_ = '-'; 
    enc_stream_ << "this ";
    uripe_.AppendChar(temp_char_, enc_stream_);
    expected_string_ = "this -";
    ASSERT_EQ(expected_string_, enc_stream_.str());
}

TEST_F(URIPercentEncodingTest, PercentEncodeReservedASCIICharsInputNotASCII)
{
    temp_string_ = "\xF2\xB3";
    output_string_ = "null";
    ret_ = uripe_.PercentEncodeReservedASCIIChars(temp_string_, output_string_);
    expected_string_ = "";
    EXPECT_FALSE(ret_);
    EXPECT_EQ(expected_string_, output_string_);
}

TEST_F(URIPercentEncodingTest, PercentEncodeReservedASCIICharsAllUnReserved)
{
    temp_string_ = "trying05OnlyUnReserved_78Chars~right-now.";
    output_string_ = "null";
    ret_ = uripe_.PercentEncodeReservedASCIIChars(temp_string_, output_string_);
    expected_string_ = temp_string_;
    EXPECT_TRUE(ret_);
    EXPECT_EQ(expected_string_, output_string_);
}

TEST_F(URIPercentEncodingTest, PercentEncodeReservedASCIICharsSomeReserved)
{
    temp_string_ = "This has some #vals ! that/are/re+served?";
    output_string_ = "null";
    ret_ = uripe_.PercentEncodeReservedASCIIChars(temp_string_, output_string_);
    expected_string_ = "This%20has%20some%20%23vals%20%21%20that%2fare%2fre%2bserved%3f";
    EXPECT_TRUE(ret_);
    EXPECT_EQ(expected_string_, output_string_);
}

TEST_F(URIPercentEncodingTest, PercentEncodeReservedASCIICharsClearStream)
{
    temp_string_ = "This has some #vals ! that/are/re+served?";
    output_string_ = "null";
    ret_ = uripe_.PercentEncodeReservedASCIIChars(temp_string_, output_string_);
    expected_string_ = "This%20has%20some%20%23vals%20%21%20that%2fare%2fre%2bserved%3f";
    EXPECT_TRUE(ret_);
    EXPECT_EQ(expected_string_, output_string_);

    temp_string_ = "another string <= old";
    output_string_ = "null";
    expected_string_ = "another%20string%20%3c%3d%20old";
    ret_ = uripe_.PercentEncodeReservedASCIIChars(temp_string_, output_string_);
    EXPECT_TRUE(ret_);
    EXPECT_EQ(expected_string_, output_string_);
}