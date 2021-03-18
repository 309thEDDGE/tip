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