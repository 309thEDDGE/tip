#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "version_info.h"

TEST(VersionInfoTest, CheckForVersionArgumentNoArgs)
{
    const int array_size = 1;
    char a1[] = "program";
    char* args[array_size] = {a1};
    int argc = array_size;

    bool ret = CheckForVersionArgument(argc, args);
    ASSERT_FALSE(ret);
}

TEST(VersionInfoTest, CheckForVersionArgumentNotCorrectString)
{
    const int array_size = 2;
    char a1[] = "program";
    char a2[] = "-vh";
    char* args[array_size] = {a1, a2};
    int argc = array_size;

    bool ret = CheckForVersionArgument(argc, args);
    ASSERT_FALSE(ret);
}

TEST(VersionInfoTest, CheckForVersionArgumentOption1)
{
    const int array_size = 2;
    char a1[] = "program";
    char a2[] = "-v";
    char* args[array_size] = {a1, a2};
    int argc = array_size;

    bool ret = CheckForVersionArgument(argc, args);
    ASSERT_TRUE(ret);
}

TEST(VersionInfoTest, CheckForVersionArgumentOption2)
{
    const int array_size = 2;
    char a1[] = "program";
    char a2[] = "--version";
    char* args[array_size] = {a1, a2};
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
