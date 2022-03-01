#include <sstream>
#include <ios>
#include <fstream>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "managed_path.h"
#include "sha256_tools.h"

std::string test_data(
    "\x06\x06\x07\x06\x06\x07\x06\x05\x06\x07\x05\x05\x06"
    "\x06\x05\x06\x06\x06\x05\x06\x06\x05\x06\x06\x06\x05"
    "\x06\x06\x05\x05\x06\x06\x05\x06\x06\x05\x05\x06\x06"
    "\x05\x06\x06\x05\x05\x06\x06\x05\x05\x06\x06\x05\x06"
    "\x06\x05\x05\x06\x06\x05\x06\x06\x06\x05\x06\x06\x05"
    "\x05\x06\x05\x05\x06\x06\x05\x05\x06\x05\x05\x05\x06"
    "\x05\x05\x06\x06\x05\x06\x07\x06\x06\x07\x07\x06\x06"
    "\x07\x07\x06\x07\x07\x06\x06\x07\x07\x06\x07\x08\x07"
    "\x06\x08\x08\x06\x07\x08\x07\x06\x08\x08\x07\x07\x08"
    "\x07\x07\x07\x08\x07\x07\x08\x07\x07\x07\x08\x07\x06"
    "\x07\x07\x06\x07\x08\x07\x07\x08\x08\x07\x07\t\x08"
    "\x07\x08\t\x08\x08\t\t\x08\x08\t\x08\x07\x08"
    "\x08\x07\x07\t\x08\x07\t\t\x08\x08\t\t\x08"
    "\t\t\x08\x08\t\t\x08\x08\t\x08\x08\t\n"
    "\x08\t\x0b\t\x08\n\n\x08\x08\n\t\x08\t"
    "\n\x08\x08\t\t\x08\t\n\x08\x07\t\t\x08"
    "\x08\n\t\x08\t\n\x08\t\x0b\n\x08\n\x0b"
    "\t\t\n\n\x08\x08\n\x08\x08\t\t\x08\x08"
    "\t\x08\x07\x08\t\x07\x07\t\x08\x07\x08\t\x07"
    "\x07\x08\x08\x07\x07\x08\x07\x06\x08\x08\x07\x07\t"
    "\x08\x07\x08\t\x07\x07\t\t\x07\x08\n\x08\x07"
    "\t\t\x07\x07\x08\x07\x06\x07\x07\x06\x06\x07\x06"
    "\x06\x06\x07\x06\x06\x08\x08\x07\x08\t\x08\x07\x08"
    "\x08\x07\x07\t\x08\x07\x08\x08\x07\x07\t\x08\x07"
    "\x08\x08\x07\x07\x08\x07\x06\x07\x07\x07\x06\x07\x07"
    "\x06\x06\x07\x07\x06\x06\x07\x06\x06\x07\x06\x06\x06"
    "\x06\x06\x06\x06\x07\x06\x06\x07\x06\x06\x06\x07\x06"
    "\x06\x06\x06\x06\x06\x07\x06\x02\xff\xfd\xfc\xfc\xfb"
    "\xfa\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8"
    "\xf8\xf7\xf1\xea\xe7\xe6\xe6\xe6\xe6\xe6\xe6\xe6\xe6"
    "\xe6\xe6\xe6\xe6\xe6\xe6\xe6\xe6\xe6\xe6\xe6\xe6\xe6"
    "\xe6\xe6\xe6\xe6\xe6\xe6\xe6\xe6\xe6\xe6\xe6\xe6\xe6"
    "\xe6\xe6\xe6\xe6\xe6\xe6\xe6\xe6\xe6\xe6\xe6\xe6\xe6"
    "\xe6\xe6\xe6\xe6\xe6\xe6\xe6\xe6\xe6\xe5\xe8\xef\xf5"
    "\xf7\xf8\xf8\xf8\xf7\xf8\xfb\xf7\xf4\xfb\xfc\xf4\xf6"
    "\xfd\xf8\xf3\xf9\xfc\xf5\xf5\xfc\xf9\xf3\xf8\xfd\xf6"
    "\xf4\xfb\xfb\xf3\xf7\xfd\xf7\xf3\xfa\xfc\xf4\xf5\xfc"
    "\xf9\xf6\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf6"
    "\xf9\x06\r\x0b\x10\x13\x0e\x0e\x12\x11\r\x10\x13"
    "\x0e\x0c\x11\x11\x0c\r\x11\x0e\x0b\x0f\x10\x0b\x0c"
    "\x11\x0e\n\x0e\x10\x0c\x0b\x10\x0f\x0b\x0e\x12\x0e"
    "\x0c\x11\x12\r\x0f\x13\x10\r\x12\x13\x0e\x0f\x14"
    "\x11\r\x11\x14\x10\x0f\x14\x13\x0f\x12\x16\x12\x11"
    "\x16\x16\x11\x12\x16\x13\x11\x16\x18\x14\x15\x1b\x18"
    "\x14\x18\x1b\x15\x13\x18\x17\x11\x13\x17\x13\x10\x15"
    "\x15\x0f\x10\x15\x11\x0e\x12\x13\x0f\x0f\x14\x13\x0f"
    "\x12\x15\x10\x0f\x14\x13\x0f\x11\x15\x11\x0f\x13\x14"
    "\x0f\x10\x14\x12\x0e\x12\x14\x0f\x0f\x13\x12\x0e\x11"
    "\x13\x0f\r\x11\x10\x0c\x0e\x11\x0e\r\x11\x12\x0e"
    "\x0f\x12\x11\x0e\x11\x13\x0f\x0e\x13\x12\x0e\x10\x14"
    "\x10\x0e\x13\x13\x0e\x0f\x13\x10\r\x11\x12\x0e\x0e"
    "\x12\x10\r\x10\x13\x0f\x0e\x12\x11\r\x0f\x12\x0f"
    "\r\x10\x11\r\x0e\x11\x0e\x0c\x0f\x11\r\r\x10"
    "\x0f\x0c\x0e\x10\r\x0c\x10\x0f\x0c\r\x10\x0e\x0c"
    "\x10\x10\r\x0e\x11\x0f\x0c\x0f\x11\r\x0c\x10\x0e"
    "\x0b\r\x0f\x0c\x0b\x0e\r\n\x0c\x0e\x0c\n\x0c"
    "\r\n\n\r\x0b\t\x0b\x0c\n\t\x0b\x0b\t"
    "\n\x0b\n\t\x0b\x0b\t\t\x0b\t\x08\t\t"
    "\x08\x08\t\t\x07\t\t\x08\x07\t\t\x07\x08"
    "\n\x08\x07\t\t\x08\x08\n\t\x08\t\n\x08"
    "\x08\n\t\x08\t\n\x08\x08\t\t\x07\x08\n"
    "\x08\x07\t\t\x08\x08\t\x08\x07\x08\t\x07\x07"
    "\x08\x08\x06\x07\x08\x06\x06\x07\x07\x05\x06\x07\x06"
    "\x05\x06\x06\x05\x05\x07\x06\x05\x06\x07\x05\x06\x07"
    "\x06\x05\x06\x07\x06\x06\x07\x06\x05\x06\x07\x06\x05"
    "\x06\x06\x05\x06\x06\x06\x05\x06\x06\x05\x05\x06\x06"
    "\x05\x06\x06\x06\x05\x06\x06\x05\x06\x06\x06\x05\x06"
    "\x06\x05\x05\x06\x06\x05\x06\x06\x05\x05\x06\x06\x05"
    "\x06\x07\x06\x05\x07\x07\x06\x07\x08\x08\x07\x08\t"
    "\x07\x07\t\x08\x07\x08\x08\x07\x07\x08\x08\x06\x07"
    "\x08\x07\x06\x08\x08\x06\x07\x08\x07\x06\x07\x08\x06"
    "\x06\x08\x07\x06\x07\x08\x06\x06\x07\x07\x06\x07\x08"
    "\x07\x06\x07\x08\x07\x07\x08\x07\x07\x07\x08\x07\x07"
    "\x08\x08\x07\x08\t\x08\x08\t\t\x08\x08\n\t"
    "\t\n\n\t\t\x0b\n\x08\t\t\x08\x07\t"
    "\x08\x07\x08\t\x08\x08\t\t\x08\x08\n\t\x08"
    "\t\t\x07\x07\x08\x08\x07\x08\x08\x07\x07\x08\x08"
    "\x07\x08\t\x08\x07\t\t\x07\x08\t\x08\x08\t"
    "\t\x08\x07\t\x08\x07\x08\t\x07\x07\t\t\x08"
    "\t\n\t\x08\n\x0b\t\t\n\t\x08\t\n"
    "\x08\x08\n\t\x08\t\n\x08\x08\t\t\x07\x08"
    "\t\x08\x07\x08\t\x07\x08\t\x08\x07\x08\t\x07"
    "\x07\t\x08\x06\x07\x08\x07\x06\x08\x07\x06\x06\x07"
    "\x06\x06\x07\x07\x06\x06\x07\x07\x06\x06\x07\x06\x05"
    "\x07\x06\x05\x06\x07\x06\x06\x07\x07\x06\x06\x07\x07"
    "\x07\x08\x08\x07\x07\x08\x08\x07\x08\x08\x07\x07\x08"
    "\x08\x07\x08\t\x07\x07\x08\x08\x07\x07\x08\x07\x07"
    "\x07\x07\x06\x06\x07\x06\x06\x06\x07\x06\x06\x06\x06"
    "\x06\x06\x06\x06\x06\x06\x06\x06\x06\x06\x06\x06\x06"
    "\x06\x06\x05\x06\x06\x06\x06\x06\x06\x06\x06\x06\x06"
    "\x06\x04");

TEST(SHA256ToolsTest, ComputeSHA256FailOnBadBits)
{
    std::stringstream ss;
    std::ios_base::iostate state = std::ios::failbit;
    ss.setstate(state);
    std::string hash;
    std::string null_hash("null");
    bool status = ComputeSHA256(ss, hash);
//EXPECT_FALSE(status);
    EXPECT_TRUE(status);
    EXPECT_EQ(null_hash, hash);

    state = std::ios::badbit;
    ss.setstate(state);
    hash = "";
    status = ComputeSHA256(ss, hash);
    EXPECT_FALSE(status);
    EXPECT_EQ(null_hash, hash);

    state = std::ios::eofbit;
    ss.setstate(state);
    hash = "";
    status = ComputeSHA256(ss, hash);
    EXPECT_FALSE(status);
    EXPECT_EQ(null_hash, hash);
}

TEST(SHA256ToolsTest, ComputeSHA256ByteCountGreaterThanStreamLength)
{
    std::stringstream ss;
    std::string input_chars = ";ljaefp9834tp9agphjq3490g8aqegrbhav4320";
    ss << input_chars;
    std::string hash;
    std::string null_hash("null");
    bool status = ComputeSHA256(ss, hash, input_chars.size() + 10);
    EXPECT_FALSE(status);
    EXPECT_EQ(null_hash, hash);
}

TEST(SHA256ToolsTest, ComputeSHA256TotalStream)
{
    std::stringstream ss;
    std::string input_chars = ";ljaefp9834tp9agphjq3490g8aqegrbhav4320";
    ss << input_chars;
    std::string hash;
    std::string expected_hash = "138566517af4eba8f202cad0a674723e6d689ebbea3b8fb3288c915e54dc7b07";
    bool status = ComputeSHA256(ss, hash);
    EXPECT_TRUE(status);
    EXPECT_EQ(expected_hash, hash);
}

TEST(SHA256ToolsTest, ComputeSHA256PartialStream)
{
    std::stringstream ss;
    std::string input_chars = ";ljaefp9834tp9agphjq3490g8aqegrbhav4320";
    ss << input_chars;
    std::string hash;
    std::string expected_hash = "57366d809ddeaa63605a185ee80414a63e50cafd6d02cce710ada8ce402a0a67";
    bool status = ComputeSHA256(ss, hash, input_chars.size() - 10);
    EXPECT_TRUE(status);
    EXPECT_EQ(expected_hash, hash);
}

TEST(SHA256ToolsTest, ComputeSHA256NonASCII)
{
    std::stringstream ss;
    std::string input_chars(
        "\xe6\xe6\xe6\xe6\xe6\xe6\xe6\xe6\xe6\xe6"
        "\xe6\xe6\xe6\xe6\xe6\xe6\xe6\xe6\xe6\xe6"
        "\xe6\xe6\xe6\xe6\xe6\xe6\xe6\xe6\xe6\xe6"
        "\xe6\xe6\xe6\xe6\xe6\xe6\xe6\xe6\xe6\xe6"
        "\xe6\xe6\xe6\xe6\xe5\xe6\xeb\xf2\xf6\xf7"
        "\xf8\xf8\xf8\xf9\xf6\xf6\xfc\xf9\xf3\xf8"
        "\xfd\xf6\xf4\xfc\xfa\xf3\xf7\xfd\xf7\xf3"
        "\xfb\xfb\xf4\xf6\xfd\xf8\xf3\xf9\xfc\xf5"
        "\xf5\xfc\xf9\xf3\xf8\xfd\xf6\xf5\xf9\xf9"
        "\xf7\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8"
        "\xf8\xfd\x08\x11\x10\x0c\x0f\x12\x0f\x0c"
        "\x10\x11\x0c\r\x11\x0e\x0b\x0f\x11\x0c\x0c"
        "\x10\x0f\x0b\x0e\x11\r\x0c\x11\x11\x0c\x0e"
        "\x12\x0f\x0c\x11\x12\r\x0e\x13\x11\r\x12\x15"
        "\x10\x10\x15\x14\x10\x14\x18\x13\x11\x18\x18"
        "\x12\x15\x1a\x16\x14\x1a\x1b\x16\x18\x1d\x1a"
        "\x16\x1a\x1b\x16\x15\x19\x17\x12\x16\x1a\x16"
        "\x14\x1a\x1a\x14\x16\x1b\x16\x12\x17\x18\x11"
        "\x11\x16\x13\x0e");
    ss << input_chars;
    std::string hash;
    std::string expected_hash = "45cbf918c2193304db9433b8c1c7a2c20c85042593b3b8c9ffe4b9f7a9e611f7";
    bool status = ComputeSHA256(ss, hash, input_chars.size());
    EXPECT_TRUE(status);
    EXPECT_EQ(expected_hash, hash);
}

TEST(SHA256ToolsTest, ComputeSHA256FromIStreamTotal)
{
    ManagedPath temp_path;
    temp_path /= "sha256_test.bin";
    std::ofstream outstream(temp_path.string(), std::ofstream::binary);
    ASSERT_TRUE(outstream.is_open());
    outstream << test_data;
    outstream.close();

    ASSERT_TRUE(temp_path.is_regular_file());
    std::ifstream instream(temp_path.string(), std::ifstream::binary);
    ASSERT_TRUE(instream.is_open());

    std::string hash;
    std::string expected_hash = "bbff0ed06d7f7e5e9769936dfd3f42ccd9b9dbbec016015691c616e4f2eba00f";
    bool status = ComputeSHA256(instream, hash);
    EXPECT_TRUE(status);
    EXPECT_EQ(expected_hash, hash);

    instream.close();
    EXPECT_TRUE(temp_path.remove());
}

TEST(SHA256ToolsTest, ComputeSHA256FromIStreamPartial)
{
    ManagedPath temp_path;
    temp_path /= "sha256_test.bin";
    std::ofstream outstream(temp_path.string(), std::ofstream::binary);
    ASSERT_TRUE(outstream.is_open());
    outstream << test_data;
    outstream.close();

    ASSERT_TRUE(temp_path.is_regular_file());
    std::ifstream instream(temp_path.string(), std::ifstream::binary);
    ASSERT_TRUE(instream.is_open());

    std::string hash;
    std::string expected_hash = "63cb2cb1dd6197b3b24575cd02accad390baf88e28de15fdc26043e884e720ec";
    bool status = ComputeSHA256(instream, hash, 426);
    EXPECT_TRUE(status);
    EXPECT_EQ(expected_hash, hash);

    instream.close();
    EXPECT_TRUE(temp_path.remove());
}

TEST(SHA256ToolsTest, ComputeFileSHA256FileNotExist)
{
    ManagedPath temp_path;
    temp_path /= "sha256_test.bin";
    ASSERT_FALSE(temp_path.is_regular_file());

    std::string hash;
    bool status = ComputeFileSHA256(temp_path, hash);
    EXPECT_FALSE(status);
}

TEST(SHA256ToolsTest, ComputeFileSHA256)
{
    ManagedPath temp_path;
    temp_path /= "sha256_test.bin";
    std::ofstream outstream(temp_path.string(), std::ofstream::binary);
    ASSERT_TRUE(outstream.is_open());
    outstream << test_data;
    outstream.close();
    ASSERT_TRUE(temp_path.is_regular_file());

    std::string hash;
    std::string expected_hash = "bbff0ed06d7f7e5e9769936dfd3f42ccd9b9dbbec016015691c616e4f2eba00f";
    bool status = ComputeFileSHA256(temp_path, hash);
    EXPECT_TRUE(status);
    EXPECT_EQ(expected_hash, hash);
    EXPECT_TRUE(temp_path.remove());
}

TEST(SHA256ToolsTest, ComputeFileSHA256ByteCountGreaterThanSize)
{
    ManagedPath temp_path;
    temp_path /= "sha256_test.bin";
    std::ofstream outstream(temp_path.string(), std::ofstream::binary);
    ASSERT_TRUE(outstream.is_open());
    outstream << test_data;
    outstream.close();
    ASSERT_TRUE(temp_path.is_regular_file());

    std::string hash;
    std::string expected_hash = "bbff0ed06d7f7e5e9769936dfd3f42ccd9b9dbbec016015691c616e4f2eba00f";
    size_t hash_byte_count = 100e6;  // Much greater than total size of file
    bool status = ComputeFileSHA256(temp_path, hash, hash_byte_count);
    EXPECT_TRUE(status);
    EXPECT_EQ(expected_hash, hash);
    EXPECT_TRUE(temp_path.remove());
}
