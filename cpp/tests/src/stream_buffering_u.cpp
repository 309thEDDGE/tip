#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "stream_buffering.h"

TEST(StreamBufferingTest, SetStreamBufferAndModeNullStream)
{
    FILE* stream = NULL;
    char buff[1024];
    int mode = _IOFBF;
    size_t size = sizeof(buff);
    bool result = SetStreamBufferAndMode(stream, buff, mode, size);
    EXPECT_FALSE(result);
}

TEST(StreamBufferingTest, SetStreamBufferAndModeValidMode)
{
    FILE* stream = stdout;
    char buff[1024];
    int mode = 10;  // invalid
    size_t size = sizeof(buff);
    bool result = SetStreamBufferAndMode(stream, buff, mode, size);
    EXPECT_FALSE(result);

    mode = _IOFBF;
    result = SetStreamBufferAndMode(stream, buff, mode, size);
    EXPECT_TRUE(result);

    mode = _IOLBF;
    result = SetStreamBufferAndMode(stream, buff, mode, size);
    EXPECT_TRUE(result);

    mode = _IONBF;
    result = SetStreamBufferAndMode(stream, buff, mode, size);
    EXPECT_TRUE(result);
}

TEST(StreamBufferingTest, SetStreamBufferAndModeInvalidSize)
{
    FILE* stream = stdout;
    char buff[1024];
    int mode = _IOFBF;
    size_t size = 1;  // must be >= 2
    bool result = SetStreamBufferAndMode(stream, buff, mode, size);
    EXPECT_FALSE(result);

    size = INT_MAX + 1;  // must be <= INT_MAX
    result = SetStreamBufferAndMode(stream, buff, mode, size);
    EXPECT_FALSE(result);
}