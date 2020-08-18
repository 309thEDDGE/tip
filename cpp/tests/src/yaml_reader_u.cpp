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
		if(infile.good())
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
	std::vector<int> compare({ 1,2,3 });
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