#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "argument_validation.h"
#include <cstdio>
#include <fstream>

class ArgumentValidationTest : public ::testing::Test
{
protected:

	ArgumentValidation av_;
	bool res_;
	std::string base_path_;
	std::string file_name_;
	std::string file_path_;
	ManagedPath mp_file_path_;

	ArgumentValidationTest() : res_(false), base_path_(""), file_name_(""),
		file_path_("")
	{

	}

	bool CreateFile(const ManagedPath& file_path, std::string file_data)
	{
		std::ofstream file;
		file.open(file_path.string());
		if (!file.is_open()) return false;
		file << file_data;
		file.close();
		return true;
	}

};

TEST_F(ArgumentValidationTest, ValidateInputFilePathIsNotUTF8)
{
	// See ParseText::IsUTF8 function for more information about
	// constructing invalid UTF-8 byte sequences.
	file_path_ = "\xE3\xA2\xA5\xB4";
	EXPECT_FALSE(av_.ValidateInputFilePath(file_path_, mp_file_path_));
	EXPECT_EQ(mp_file_path_.RawString(), "");
}

TEST_F(ArgumentValidationTest, ValidateInputFilePathDoesNotExist)
{
	// See ParseText::IsUTF8 function for more information about
	// constructing invalid UTF-8 byte sequences.
	file_path_ = "data.txt";
	EXPECT_FALSE(av_.ValidateInputFilePath(file_path_, mp_file_path_));
	EXPECT_EQ(mp_file_path_.RawString(), "");
}

TEST_F(ArgumentValidationTest, ValidateInputFilePathCorrectPathSet)
{
	// See ParseText::IsUTF8 function for more information about
	// constructing invalid UTF-8 byte sequences.
	file_path_ = "data.txt";
	ManagedPath temp_mp_path(file_path_);
	ASSERT_TRUE(CreateFile(temp_mp_path, "line1\nline2\n"));
	EXPECT_TRUE(av_.ValidateInputFilePath(file_path_, mp_file_path_));
	EXPECT_EQ(mp_file_path_.RawString(), file_path_);
	EXPECT_TRUE(mp_file_path_.remove());
}

TEST_F(ArgumentValidationTest, ValidateDefaultInputFilePathUserPathIsNotUTF8)
{
	// See ParseText::IsUTF8 function for more information about
	// constructing invalid UTF-8 byte sequences.
	file_path_ = "\xE3\xA2\xA5\xB4";
	file_name_ = "data.txt";
	ManagedPath default_path;
	EXPECT_FALSE(av_.ValidateDefaultInputFilePath(default_path, file_path_, 
		file_name_, mp_file_path_));
	EXPECT_EQ(mp_file_path_.RawString(), "");
}

TEST_F(ArgumentValidationTest, ValidateDefaultInputFilePathUserFileIsNotUTF8)
{
	// See ParseText::IsUTF8 function for more information about
	// constructing invalid UTF-8 byte sequences.
	file_path_ = "test_dir";
	file_name_ = "\xE3\xA2\xA5\xB4";
	ManagedPath default_path;
	EXPECT_FALSE(av_.ValidateDefaultInputFilePath(default_path, file_path_,
		file_name_, mp_file_path_));
	EXPECT_EQ(mp_file_path_.RawString(), "");
}

TEST_F(ArgumentValidationTest, ValidateDefaultInputFilePathUserPathDoesNotExist)
{
	// See ParseText::IsUTF8 function for more information about
	// constructing invalid UTF-8 byte sequences.
	file_path_ = "test_dir";
	file_name_ = "data.txt";
	ManagedPath default_path;
	EXPECT_FALSE(av_.ValidateDefaultInputFilePath(default_path, file_path_,
		file_name_, mp_file_path_));
	EXPECT_EQ(mp_file_path_.RawString(), "");
}

TEST_F(ArgumentValidationTest, ValidateDefaultInputFilePathDefaultPathDoesNotExist)
{
	// See ParseText::IsUTF8 function for more information about
	// constructing invalid UTF-8 byte sequences.
	file_path_ = "";
	file_name_ = "data.txt";
	ManagedPath default_path;
	default_path /= std::string("test_dir");
	EXPECT_FALSE(av_.ValidateDefaultInputFilePath(default_path, file_path_,
		file_name_, mp_file_path_));
	EXPECT_EQ(mp_file_path_.RawString(), "");
}

TEST_F(ArgumentValidationTest, ValidateDefaultInputFilePathCorrectUserPath)
{
	// See ParseText::IsUTF8 function for more information about
	// constructing invalid UTF-8 byte sequences.
	file_path_ = "test_dir";
	file_name_ = "data.txt";
	ManagedPath default_path;
	ManagedPath user_path;
	user_path = user_path / file_path_;
	ASSERT_TRUE(user_path.create_directory());
	user_path /= file_name_;
	ASSERT_TRUE(CreateFile(user_path, "line1\nline2\n"));
	EXPECT_TRUE(av_.ValidateDefaultInputFilePath(default_path, file_path_,
		file_name_, mp_file_path_));
	EXPECT_EQ(mp_file_path_.RawString(), user_path.RawString());
	EXPECT_TRUE(mp_file_path_.remove());
	EXPECT_TRUE(mp_file_path_.parent_path().remove());
}

TEST_F(ArgumentValidationTest, ValidateDefaultInputFilePathCorrectDefaultPath)
{
	// See ParseText::IsUTF8 function for more information about
	// constructing invalid UTF-8 byte sequences.
	file_path_ = "";
	file_name_ = "data.txt";
	ManagedPath default_path;
	default_path /= std::string("test_dir");
	ASSERT_TRUE(default_path.create_directory());
	default_path /= file_name_;
	ASSERT_TRUE(CreateFile(default_path, "line1\nline2\n"));
	EXPECT_TRUE(av_.ValidateDefaultInputFilePath(default_path.parent_path(), file_path_,
		file_name_, mp_file_path_));
	EXPECT_EQ(mp_file_path_.RawString(), default_path.RawString());
	EXPECT_TRUE(mp_file_path_.remove());
	EXPECT_TRUE(mp_file_path_.parent_path().remove());
}

TEST_F(ArgumentValidationTest, ValidateDocumentBadPath)
{
	// See ParseText::IsUTF8 function for more information about
	// constructing invalid UTF-8 byte sequences.
	file_path_ = "";
	file_name_ = "data.txt";
	ManagedPath doc_path;
	doc_path /= file_name_;
	std::string doc_string;
	EXPECT_FALSE(av_.ValidateDocument(doc_path, doc_string));
	EXPECT_EQ(doc_string, "");
}

TEST_F(ArgumentValidationTest, ValidateDocumentNotUTF8)
{
	// See ParseText::IsUTF8 function for more information about
	// constructing invalid UTF-8 byte sequences.
	file_path_ = "";
	file_name_ = "data.txt";
	ManagedPath doc_path;
	doc_path /= file_name_;
	ASSERT_TRUE(CreateFile(doc_path, "line1\n\xE3\xA2\xA5\xB4\n"));
	std::string doc_string;
	EXPECT_FALSE(av_.ValidateDocument(doc_path, doc_string));
	EXPECT_EQ(doc_string, "");
	EXPECT_TRUE(doc_path.remove());
}

TEST_F(ArgumentValidationTest, ValidateDocumentCorrectDocString)
{
	// See ParseText::IsUTF8 function for more information about
	// constructing invalid UTF-8 byte sequences.
	file_path_ = "";
	file_name_ = "data.txt";
	std::string file_data = "line1\n\line2\n";
	ManagedPath doc_path;
	doc_path /= file_name_;
	ASSERT_TRUE(CreateFile(doc_path, file_data));
	std::string doc_string;
	EXPECT_TRUE(av_.ValidateDocument(doc_path, doc_string));
	EXPECT_EQ(doc_string, file_data);
	EXPECT_TRUE(doc_path.remove());
}

TEST_F(ArgumentValidationTest, ValidateOutputDirPathNotUTF8)
{
	file_path_ = "\xE3\xA2\xA5\xB4";
	ManagedPath mp_output_dir;
	EXPECT_FALSE(av_.ValidateOutputDirPath(file_path_, mp_output_dir));
	EXPECT_EQ(mp_output_dir.RawString(), "");
}

TEST_F(ArgumentValidationTest, ValidateOutputDirPathNotExist)
{
	file_path_ = "test_data";
	ManagedPath mp_output_dir;
	EXPECT_FALSE(av_.ValidateOutputDirPath(file_path_, mp_output_dir));
	EXPECT_EQ(mp_output_dir.RawString(), "");
}

TEST_F(ArgumentValidationTest, ValidateOutputDirCorrectPath)
{
	file_path_ = "test_data";
	ManagedPath temp_dir({ file_path_ });
	ASSERT_TRUE(temp_dir.create_directory());
	ManagedPath mp_output_dir;
	EXPECT_TRUE(av_.ValidateOutputDirPath(file_path_, mp_output_dir));
	EXPECT_EQ(mp_output_dir.RawString(), temp_dir.RawString());
	EXPECT_TRUE(temp_dir.remove());
}