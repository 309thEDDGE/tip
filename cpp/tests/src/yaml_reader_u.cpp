#include "gtest/gtest.h"
#include "yaml_reader.h"
#include <fstream>
#include <iostream>

class YamlReaderTest : public ::testing::Test
{
   protected:
    YamlReader yr;
    int output = -1;
    std::string filename = "testfile.txt";
    std::ofstream file;

    YamlReaderTest()
    {
        std::ifstream infile(filename);
        if (infile.good())
            remove(filename.c_str());
        file.open(filename);
    }
    ~YamlReaderTest()
    {
        file.close();
        remove(filename.c_str());
    }
    void SetUp() override
    {
    }
};

TEST_F(YamlReaderTest, LinkFileReturnsInvalidFileFlag)
{
    ASSERT_FALSE(yr.LinkFile("badFilePath.txt"));
}

TEST_F(YamlReaderTest, LinkFileReturnsValidFileFlag)
{
    file << "Line1\nLine2";
    file.close();

    ASSERT_TRUE(yr.LinkFile(filename));
}

TEST_F(YamlReaderTest, IngestYamlAsStringEmptyString)
{
    std::string yaml_matter = "";
    bool status = yr.IngestYamlAsString(yaml_matter);
    EXPECT_FALSE(status);
}

TEST_F(YamlReaderTest, IngestYamlAsStringNodeLoaded)
{
    std::string yaml_matter = "data: 10";
    bool status = yr.IngestYamlAsString(yaml_matter);
    EXPECT_TRUE(status);

    int val = 0;
    status = yr.GetParams("data", val, false);
    EXPECT_TRUE(status);
    EXPECT_EQ(10, val);
}

TEST_F(YamlReaderTest, IngestYamlAsStringInvalidYaml)
{
    std::string yaml_matter = {
        "parse_chunk_bytes: 150\n"
        "parse_thread_count: 2"
        "max_chunk_read_count: 5\n"};
    bool status = yr.IngestYamlAsString(yaml_matter);
    EXPECT_FALSE(status);
}

TEST_F(YamlReaderTest, GetParamsReturnsFalseWhenParamDoesNotExist)
{
    file << "Param1 : 1\n";
    file.close();

    ASSERT_TRUE(yr.LinkFile(filename));

    bool status = yr.GetParams("InvalidParam", output, true);
    ASSERT_FALSE(status);
    ASSERT_EQ(output, -1);
}

TEST_F(YamlReaderTest, GetParamsReturnsTrueWhenParamDoesExist)
{
    file << "Param1 : 1\n";
    file.close();

    ASSERT_TRUE(yr.LinkFile(filename));

    bool status = yr.GetParams("Param1", output, true);
    ASSERT_TRUE(status);
}

TEST_F(YamlReaderTest, GetParamsCorrectParameterReturnValue)
{
    file << "Param1 : 1\n";
    file.close();

    ASSERT_TRUE(yr.LinkFile(filename));

    bool status = yr.GetParams("Param1", output, true);
    ASSERT_EQ(output, 1);
}

TEST_F(YamlReaderTest, GetParamsListReturnValue)
{
    file << "Param1 : [1,2,3]\n";
    file.close();

    ASSERT_TRUE(yr.LinkFile(filename));

    std::vector<int> test;
    std::vector<int> compare({1, 2, 3});
    bool status = yr.GetParams("Param1", test, true);
    ASSERT_EQ(test, compare);
}

TEST_F(YamlReaderTest, GetParamsInvalidDataTypeRead)
{
    file << "Param1 : a\n";
    file.close();

    ASSERT_TRUE(yr.LinkFile(filename));

    bool status = yr.GetParams("Param1", output, true);
    ASSERT_FALSE(status);
    ASSERT_EQ(output, -1);
}

TEST_F(YamlReaderTest, GetParamsUpperValidBoundary)
{
    file << "Param1 : 5\n";
    file.close();

    ASSERT_TRUE(yr.LinkFile(filename));

    bool status = yr.GetParams("Param1", output, 0, 5, true);
    EXPECT_TRUE(status);
    EXPECT_EQ(output, 5);
}

TEST_F(YamlReaderTest, GetParamsUpperInValidBoundary)
{
    file << "Param1 : 5\n";
    file.close();

    ASSERT_TRUE(yr.LinkFile(filename));

    bool status = yr.GetParams("Param1", output, 0, 4, true);
    EXPECT_FALSE(status);
}

TEST_F(YamlReaderTest, GetParamsLowerValidBoundary)
{
    file << "Param1 : 0\n";
    file.close();

    ASSERT_TRUE(yr.LinkFile(filename));

    bool status = yr.GetParams("Param1", output, 0, 5, true);
    EXPECT_TRUE(status);
    EXPECT_EQ(output, 0);
}

TEST_F(YamlReaderTest, GetParamsLowerInValidBoundary)
{
    file << "Param1 : 0\n";
    file.close();

    ASSERT_TRUE(yr.LinkFile(filename));

    bool status = yr.GetParams("Param1", output, 1, 5, true);
    EXPECT_FALSE(status);
}

TEST(YamlReaderStaticFunctionsTest, GetMapNodeParameterCorrect)
{
        YAML::Node node = YAML::Load(
        "str: \"my string\"\n"   
        "int: 23\n"
        "flt: 51.5\n"
        );

        std::string stroutput;
        std::string parameter = "str";
        bool status;
        status = YamlReader::GetMapNodeParameter(node, parameter, stroutput);
        EXPECT_TRUE(status);
        EXPECT_EQ(stroutput, node[parameter].as<std::string>());

        int intoutput;
        parameter = "int";
        status = YamlReader::GetMapNodeParameter(node, parameter, intoutput);
        EXPECT_TRUE(status);
        EXPECT_EQ(intoutput, node[parameter].as<int>());

        float fltoutput;
        parameter = "flt";
        status = YamlReader::GetMapNodeParameter(node, parameter, fltoutput);
        EXPECT_TRUE(status);
        EXPECT_EQ(fltoutput, node[parameter].as<float>());
}

TEST(YamlReaderStaticFunctionsTest, GetMapNodeParameterNotMap)
{
        YAML::Node node = YAML::Load(
            "- 1\n"
            "- 2\n"
            "- 3\n"
        );

        bool status;
        std::string stroutput;
        std::string parameter = "1";

        status = YamlReader::GetMapNodeParameter(node, parameter, stroutput);
        EXPECT_FALSE(status);
}

TEST(YamlReaderStaticFunctionsTest, GetMapNodeParameterMissingKey)
{
        YAML::Node node = YAML::Load(
        "str: \"my string\"\n"   
        "int: 23\n"
        "flt: 51.5\n"
        );

        bool status;
        std::string stroutput;

        // Parameter "1" does not exist in map.
        std::string parameter = "1";

        status = YamlReader::GetMapNodeParameter(node, parameter, stroutput);
        EXPECT_FALSE(status);
}

TEST(YamlReaderStaticFunctionsTest, GetMapNodeParameterFailedCast)
{
        YAML::Node node = YAML::Load(
        "str: \"my string\"\n"   
        "int: 23\n"
        "flt: 51.5\n"
        );

        bool status;
        int intoutput;
        std::string parameter = "str";

        // Requesting an int cast from "my string". Ought to fail.
        status = YamlReader::GetMapNodeParameter(node, parameter, intoutput);
        EXPECT_FALSE(status);
}

TEST(YamlReaderStaticFunctionsTest, GetSequenceNodeVectorCorrect)
{
        YAML::Node node = YAML::Load(
        "str: [my string, data, 4]\n"   
        "int: [23, 0, 1000]\n"
        "flt: [51.5, 2, 10.0]\n"
        );

        bool status;
        std::vector<std::string> strvec;
        std::vector<int> intvec;
        std::vector<float> fltvec;

        std::vector<std::string> str_expected = {"my string", "data", "4"};
        status = YamlReader::GetSequenceNodeVector(node["str"], strvec);
        EXPECT_TRUE(status);
        EXPECT_EQ(str_expected, strvec);

        std::vector<int> int_expected = {23, 0, 1000};
        status = YamlReader::GetSequenceNodeVector(node["int"], intvec);
        EXPECT_TRUE(status);
        EXPECT_EQ(int_expected, intvec);

        std::vector<float> flt_expected = {51.5, 2., 10.};
        status = YamlReader::GetSequenceNodeVector(node["flt"], fltvec);
        EXPECT_TRUE(status);
        EXPECT_EQ(flt_expected, fltvec);
}

TEST(YamlReaderStaticFunctionsTest, GetSequenceNodeVectorNotSequence)
{
        YAML::Node node = YAML::Load(
        "str: data\n"   
        "int: [23, 0, 1000]\n"
        "flt: [51.5, 2, 10.0]\n"
        );

        bool status;
        std::vector<std::string> strvec;
        status = YamlReader::GetSequenceNodeVector(node["str"], strvec);
        EXPECT_FALSE(status);
}

TEST(YamlReaderStaticFunctionsTest, GetSequenceNodeVectorFailedCast)
{
        YAML::Node node = YAML::Load(
        "int: [23, zero, 1000]\n"
        "flt: [51.5, 2, 10.0]\n"
        );

        bool status;
        std::vector<int> intvec;
        status = YamlReader::GetSequenceNodeVector(node["int"], intvec);
        EXPECT_FALSE(status);
}