#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "version_info.h"

TEST(VersionInfoTest, CheckForVersionArgumentNoArgs)
{
    const int array_size = 1;
    char* args[array_size] = {"program"};
    int argc = array_size;

    bool ret = CheckForVersionArgument(argc, args);
    ASSERT_FALSE(ret);
}

TEST(VersionInfoTest, CheckForVersionArgumentNotCorrectString)
{
    const int array_size = 2;
    char* args[array_size] = {"program", "-vh"};
    int argc = array_size;

    bool ret = CheckForVersionArgument(argc, args);
    ASSERT_FALSE(ret);
}

TEST(VersionInfoTest, CheckForVersionArgumentOption1)
{
    const int array_size = 2;
    char* args[array_size] = {"program", "-v"};
    int argc = array_size;

    bool ret = CheckForVersionArgument(argc, args);
    ASSERT_TRUE(ret);
}

TEST(VersionInfoTest, CheckForVersionArgumentOption2)
{
    const int array_size = 2;
    char* args[array_size] = {"program", "--version"};
    int argc = array_size;

    bool ret = CheckForVersionArgument(argc, args);
    ASSERT_TRUE(ret);
}

TEST(VersionInfoTest, GetVersionString)
{
    std::string ver_str = VERSION_STRING;
    std::string result = GetVersionString();
    ASSERT_EQ(ver_str, result); 
}
