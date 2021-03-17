#include "gtest/gtest.h"
#include "file_reader.h"
#include <fstream>
#include <iostream>

class FileReaderTest : public ::testing::Test
{
protected:

	FileReader fr;

	FileReaderTest()
	{

	}
	void SetUp() override
	{

	}


};


TEST_F(FileReaderTest, ReadFileReturnsInvalidFileFlag)
{
	std::string filename = "badFilePath.txt";
	int returnVal = fr.ReadFile(filename);
	ASSERT_EQ(returnVal, 1);
}

TEST_F(FileReaderTest, ReadFileReturnsValidFileFlag)
{
	std::string filename = "testfile.txt";
	std::ofstream file;
	file.open(filename);
	file << "Line1\nLine2";
	file.close();

	int returnVal = fr.ReadFile(filename);
	ASSERT_EQ(returnVal, 0);
	remove(filename.c_str());
}

TEST_F(FileReaderTest, ReadFileAddOneLineToVector)
{
	std::string filename = "testfile.txt";
	std::ofstream file;
	file.open(filename);
	file << "Line1";
	file.close();

	std::vector<std::string> compareVec;
	compareVec.push_back("Line1");

	fr.ReadFile(filename);
	ASSERT_TRUE(compareVec == fr.GetLines());
	remove(filename.c_str());
}


TEST_F(FileReaderTest, ReadFileAddMultipleLinesToVector)
{
	std::string filename = "testfile.txt";
	std::ofstream file;
	file.open(filename);
	file << "Line1\nLine2";
	file.close();

	std::vector<std::string> compareVec;
	compareVec.push_back("Line1");
	compareVec.push_back("Line2");

	fr.ReadFile(filename);
	ASSERT_TRUE(compareVec == fr.GetLines());
	remove(filename.c_str());
}

TEST_F(FileReaderTest, GetDocumentAsStringNoReadFile)
{
	std::string doc = fr.GetDocumentAsString();
	ASSERT_TRUE(doc.size() == 0);
}

TEST_F(FileReaderTest, GetDocumentAsString)
{
	std::string filename = "testfile.txt";
	std::ofstream file;
	std::string data = "Line1\nLine2\nhere is a line which is [x, y, z]\n";
	file.open(filename);
	file << data;
	file.close();

	EXPECT_EQ(fr.ReadFile(filename), 0);
	std::string doc = fr.GetDocumentAsString();
	EXPECT_TRUE(doc.size() > 0);
	EXPECT_EQ(doc, data);
}

TEST_F(FileReaderTest, ReadFileReadNewFile)
{
	std::string filename = "testfile.txt";
	std::ofstream file;
	file.open(filename);
	file << "Line1\nLine2\n";
	file.close();

	EXPECT_EQ(fr.ReadFile(filename), 0);
	EXPECT_EQ(fr.GetLines().size(), 2);
	remove(filename.c_str());

	file.open(filename);
	file << "newline1\nnewline2\n";
	file.close();

	EXPECT_EQ(fr.ReadFile(filename), 0);
	EXPECT_EQ(fr.GetLines().size(), 2);
	remove(filename.c_str());
}