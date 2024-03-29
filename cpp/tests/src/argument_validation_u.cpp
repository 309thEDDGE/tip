#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "argument_validation.h"
#include <map>
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

    ArgumentValidationTest() : res_(false), base_path_(""), file_name_(""), file_path_("")
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
    std::string file_data = "line1\nline2\n";
    ManagedPath doc_path;
    doc_path /= file_name_;
    ASSERT_TRUE(CreateFile(doc_path, file_data));
    std::string doc_string;
    EXPECT_TRUE(av_.ValidateDocument(doc_path, doc_string));
    EXPECT_EQ(doc_string, file_data);
    EXPECT_TRUE(doc_path.remove());
}

TEST_F(ArgumentValidationTest, ValidateDirectoryPathNotUTF8)
{
    file_path_ = "\xE3\xA2\xA5\xB4";
    ManagedPath mp_output_dir;
    EXPECT_FALSE(av_.ValidateDirectoryPath(file_path_, mp_output_dir));
    EXPECT_EQ(mp_output_dir.RawString(), "");
}

TEST_F(ArgumentValidationTest, ValidateDirectoryPathNotExist)
{
    file_path_ = "test_data";
    ManagedPath mp_output_dir;
    EXPECT_FALSE(av_.ValidateDirectoryPath(file_path_, mp_output_dir));
    EXPECT_EQ(mp_output_dir.RawString(), "");
}

TEST_F(ArgumentValidationTest, ValidateDirectoryPathCorrectPath)
{
    file_path_ = "test_data";
    ManagedPath temp_dir({file_path_});
    ASSERT_TRUE(temp_dir.create_directory());
    ManagedPath mp_output_dir;
    EXPECT_TRUE(av_.ValidateDirectoryPath(file_path_, mp_output_dir));
    EXPECT_EQ(mp_output_dir.RawString(), temp_dir.RawString());
    EXPECT_TRUE(temp_dir.remove());
}

TEST_F(ArgumentValidationTest, CheckExtensionNoExtensionOnInputPath)
{
    file_path_ = "test_data";
    EXPECT_FALSE(av_.CheckExtension(file_path_, {"data", "txt"}));
}

TEST_F(ArgumentValidationTest, CheckExtensionSingleExtension)
{
    file_path_ = "test_data.txt";
    EXPECT_FALSE(av_.CheckExtension(file_path_, {"text"}));
    EXPECT_TRUE(av_.CheckExtension(file_path_, {"txt"}));
    EXPECT_TRUE(av_.CheckExtension(file_path_, {"TXT"}));
}

TEST_F(ArgumentValidationTest, CheckExtensionMultipleExtension)
{
    file_path_ = "test_data.Ch10";
    EXPECT_FALSE(av_.CheckExtension(file_path_, {".Ch10", "bad", "txt"}));
    EXPECT_TRUE(av_.CheckExtension(file_path_, {"CCh10", "c10", "ch10"}));
    EXPECT_TRUE(av_.CheckExtension(file_path_, {"txt", "csv", "CH10"}));
}

TEST_F(ArgumentValidationTest, ValidateDefaultOutputDirectoryDefaultDirNotPresentNoCreate)
{
    base_path_ = "temp_dir";
    ManagedPath default_dir(base_path_);
    ManagedPath final_path;
    std::string user_dir = "";
    EXPECT_FALSE(av_.ValidateDefaultOutputDirectory(default_dir, user_dir, final_path, false));
    EXPECT_EQ(final_path.RawString(), "");
}

TEST_F(ArgumentValidationTest, ValidateDefaultOutputDirectoryDefaultDirNotPresentDoCreate)
{
    base_path_ = "temp_dir";
    ManagedPath default_dir({base_path_});
    ManagedPath final_path;
    std::string user_dir = "";
    ASSERT_FALSE(default_dir.is_directory());
    EXPECT_TRUE(av_.ValidateDefaultOutputDirectory(default_dir, user_dir, final_path, true));
    EXPECT_EQ(final_path.RawString(), default_dir.absolute().RawString());
    EXPECT_TRUE(final_path.remove());
}

TEST_F(ArgumentValidationTest, ValidateDefaultOutputDirectoryUserDirNotUTF8)
{
    base_path_ = "temp_dir";
    ManagedPath default_dir({base_path_});
    ManagedPath final_path;
    std::string user_dir = "\xE3\xA2\xA5\xB4";
    EXPECT_FALSE(av_.ValidateDefaultOutputDirectory(default_dir, user_dir, final_path, false));
    EXPECT_EQ(final_path.RawString(), "");
}

TEST_F(ArgumentValidationTest, ValidateDefaultOutputDirectoryUserDirNotPresentNoCreate)
{
    base_path_ = "temp_dir";
    ManagedPath default_dir({base_path_});
    ManagedPath final_path;
    std::string user_dir = "temp_user_dir";
    EXPECT_FALSE(av_.ValidateDefaultOutputDirectory(default_dir, user_dir, final_path, false));
    EXPECT_EQ(final_path.RawString(), "");
}

TEST_F(ArgumentValidationTest, ValidateDefaultOutputDirectoryUserDirNotPresentDoCreate)
{
    base_path_ = "temp_dir";
    ManagedPath default_dir({base_path_});
    ManagedPath final_path;
    std::string user_dir = "temp_user_dir";
    ManagedPath user_full_path({user_dir});
    EXPECT_TRUE(av_.ValidateDefaultOutputDirectory(default_dir, user_dir, final_path, true));
    EXPECT_EQ(final_path.RawString(), user_full_path.RawString());
    EXPECT_TRUE(final_path.remove());
}

TEST_F(ArgumentValidationTest, ValidateDefaultOutputDirectoryDefaultDirPresent)
{
    base_path_ = "temp_dir";
    ManagedPath default_dir({base_path_});
    ASSERT_TRUE(default_dir.create_directory());
    ManagedPath final_path;
    std::string user_dir = "";
    EXPECT_TRUE(av_.ValidateDefaultOutputDirectory(default_dir, user_dir, final_path, false));
    EXPECT_EQ(final_path.RawString(), default_dir.RawString());
    EXPECT_TRUE(default_dir.remove());
}

TEST_F(ArgumentValidationTest, ValidateDefaultOutputDirectoryUserDirPresent)
{
    base_path_ = "temp_dir";
    ManagedPath default_dir({base_path_});
    ManagedPath final_path;
    std::string user_dir = "temp_user_dir";
    ManagedPath user_full_path({user_dir});
    ASSERT_TRUE(user_full_path.create_directory());
    EXPECT_TRUE(av_.ValidateDefaultOutputDirectory(default_dir, user_dir, final_path, false));
    EXPECT_EQ(final_path.RawString(), user_full_path.RawString());
    EXPECT_TRUE(final_path.remove());
}

TEST_F(ArgumentValidationTest, ParseArgsInsufficientArgCount)
{
    const int arg_count = 3;
    int argc = arg_count;
    char a1[] = "a";
    char a2[] = "b";
    char a3[] = "see";
    char* argv[arg_count] = {a1, a2, a3};

    std::map<int, std::string> def_args = {
        {1, "A"},
        {2, "B"},
        {3, "C"},
        {4, "D"}
    };
    std::map<std::string, std::string> out_args;
    ASSERT_FALSE(av_.ParseArgs(argc, argv, def_args, out_args));
}

TEST_F(ArgumentValidationTest, ParseArgs)
{
    const int arg_count = 4;
    int argc = arg_count;
    char a1[] = "a";
    char a2[] = "b";
    char a3[] = "see";
    char a4[] = "dee";
    char* argv[arg_count] = {a1, a2, a3, a4};

    std::map<int, std::string> def_args = {
        {1, "A"},
        {2, "B"},
        {3, "C"}
    };
    std::map<std::string, std::string> out_args;
    ASSERT_TRUE(av_.ParseArgs(argc, argv, def_args, out_args));
    ASSERT_TRUE(out_args.size() == (arg_count-1));
    EXPECT_EQ("b", out_args.at("A"));
    EXPECT_EQ("see", out_args.at("B"));
    EXPECT_EQ("dee", out_args.at("C"));
}

TEST_F(ArgumentValidationTest, ParseArgsInsufficientArgCountAllowFewer)
{
    const int arg_count = 3;
    int argc = arg_count;
    char a1[] = "a";
    char a2[] = "b";
    char a3[] = "see";
    char* argv[arg_count] = {a1, a2, a3};

    std::map<int, std::string> def_args = {
        {1, "A"},
        {2, "B"},
        {3, "C"},
        {4, "D"}
    };
    std::map<std::string, std::string> out_args;
    ASSERT_TRUE(av_.ParseArgs(argc, argv, def_args, out_args, true));
    EXPECT_EQ("b", out_args.at("A"));
    EXPECT_EQ("see", out_args.at("B"));
    EXPECT_EQ("", out_args.at("C"));
    EXPECT_EQ("", out_args.at("D"));
}

TEST_F(ArgumentValidationTest, TestOptionalArgCountGreaterThanMax)
{
    const int arg_count = 6;

    std::map<int, std::string> req_args = {
        {1, "print statement 1"},
        {3, "print 2"},
        {4, "print this"}
    };
    ASSERT_FALSE(av_.TestOptionalArgCount(arg_count, req_args));

    std::map<int, std::string> req_args2 = {
        {1, "print statement 1"},
    };
    ASSERT_FALSE(av_.TestOptionalArgCount(arg_count, req_args2));
}

TEST_F(ArgumentValidationTest, TestOptionalArgCountZeroMapSize)
{
    const int arg_count = 5;
    std::map<int, std::string> req_args;
    ASSERT_TRUE(av_.TestOptionalArgCount(arg_count, req_args));
}

TEST_F(ArgumentValidationTest, TestOptionalArgCountLessThanMin)
{
    const int arg_count = 1;

    std::map<int, std::string> req_args = {
        {2, "print statement 1"},
        {3, "print 2"},
        {4, "print this"}
    };
    ASSERT_FALSE(av_.TestOptionalArgCount(arg_count, req_args));
}

TEST_F(ArgumentValidationTest, TestOptionalArgCountNotOneOf)
{
    const int arg_count = 3;
    std::map<int, std::string> req_args = {
        {1, "print statement 1"},
        {3, "print 2"}
    };
    ASSERT_FALSE(av_.TestOptionalArgCount(arg_count, req_args));
}

TEST_F(ArgumentValidationTest, TestOptionalArgCount)
{
    const int arg_count = 2;
    std::map<int, std::string> req_args = {
        {1, "print statement 1"},
        {2, "print 2"}
    };
    ASSERT_TRUE(av_.TestOptionalArgCount(arg_count, req_args));
}

TEST_F(ArgumentValidationTest, ArgSelectFromZeroArgs)
{
    const int arg_count = 0;
    int argc = arg_count;
    char** arg_vec = nullptr;
    char** argv = arg_vec;
    av_.ArgSelectFrom(2, argc, &argv);
    ASSERT_EQ(arg_count, argc);
    ASSERT_EQ(arg_vec, argv);
}

TEST_F(ArgumentValidationTest, ArgSelectFromLast)
{
    const int arg_count = 3;
    int argc = arg_count;
    char a1[] = "one";
    char a2[] = "two";
    char a3[] = "three";
    char* arg_vec[arg_count] = {a1, a2, a3};
    char** argv = arg_vec;
    char** expected = argv+2;
    av_.ArgSelectFrom(2, argc, &argv);
    ASSERT_EQ(1, argc);
    ASSERT_EQ(expected, argv);
    ASSERT_EQ(a3, argv[0]);
}

TEST_F(ArgumentValidationTest, ArgSelectFrom)
{
    const int arg_count = 4;
    int argc = arg_count;
    char a1[] = "one";
    char a2[] = "two";
    char a3[] = "three";
    char a4[] = "four";
    char* arg_vec[arg_count] = {a1, a2, a3, a4};
    char** argv = arg_vec;
    char** expected = argv+1;
    av_.ArgSelectFrom(1, argc, &argv);
    ASSERT_EQ(3, argc);
    ASSERT_EQ(expected, argv);
    ASSERT_EQ(a2, argv[0]);
    ASSERT_EQ(a3, argv[1]);
    ASSERT_EQ(a4, argv[2]);
}

TEST_F(ArgumentValidationTest, ArgSelectToZeroArgs)
{
    const int arg_count = 0;
    int argc = arg_count;
    char** arg_vec = nullptr;
    char** argv = arg_vec;
    av_.ArgSelectTo(2, argc, &argv);
    ASSERT_EQ(arg_count, argc);
    ASSERT_EQ(arg_vec, argv);
}

TEST_F(ArgumentValidationTest, ArgSelectToLast)
{
    const int arg_count = 3;
    int argc = arg_count;
    char a1[] = "one";
    char a2[] = "two";
    char a3[] = "three";
    char* arg_vec[arg_count] = {a1, a2, a3};
    char** argv = arg_vec;
    char** expected = argv;
    av_.ArgSelectTo(2, argc, &argv);
    ASSERT_EQ(2, argc);
    ASSERT_EQ(expected, argv);
    ASSERT_EQ(a1, argv[0]);
    ASSERT_EQ(a2, argv[1]);
}

TEST_F(ArgumentValidationTest, ArgSelectTo)
{
    const int arg_count = 4;
    int argc = arg_count;
    char a1[] = "one";
    char a2[] = "two";
    char a3[] = "three";
    char a4[] = "four";
    char* arg_vec[arg_count] = {a1, a2, a3, a4};
    char** argv = arg_vec;
    char** expected = argv;
    av_.ArgSelectTo(1, argc, &argv);
    ASSERT_EQ(1, argc);
    ASSERT_EQ(expected, argv);
    ASSERT_EQ(a1, argv[0]);
}