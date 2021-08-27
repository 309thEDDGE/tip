#include <cstdio>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "resource_limits.h"

#ifdef __linux__
#include <sys/resource.h>
#endif

TEST(ResourceLimitsTest, GetFileDescriptorLimitsCorrectVals)
{
    uint64_t fd_soft_limit, fd_hard_limit;
    uint64_t expected_soft_limit, expected_hard_limit;

#ifdef __linux__
    // Get file descriptor limits manually.
    struct rlimit limits;
    int ret = getrlimit(RLIMIT_NOFILE, &limits);
    ASSERT_EQ(0, ret);

    printf("file descriptor soft limit = %ju\n", limits.rlim_cur);
    printf("file descriptor hard limit = %ju\n", limits.rlim_max);

    expected_soft_limit = static_cast<uint64_t>(limits.rlim_cur);
    expected_hard_limit = static_cast<uint64_t>(limits.rlim_max);
#elif defined __WIN64
    int ret = _getmaxstdio();
    ASSERT_TRUE(ret != -1);

    expected_soft_limit = static_cast<uint64_t>(ret);
    expected_hard_limit = static_cast<uint64_t>(_SETMAXSTDIO_MAXIMUM);

    printf("file descriptor soft limit = %llu\n", expected_soft_limit);
    printf("file descriptor hard limit = %llu\n", expected_hard_limit);

#endif

    bool status = GetFileDescriptorLimits(fd_soft_limit, fd_hard_limit);
    EXPECT_TRUE(status);
    EXPECT_EQ(expected_soft_limit, fd_soft_limit);
    EXPECT_EQ(expected_hard_limit, fd_hard_limit);
}

#ifdef __linux__
TEST(ResourceLimitsTest, GetFileDescriptorLimitsFailure)
{
    uint64_t fd_soft_limit, fd_hard_limit;

    // Set bad resource value to cause getrlimit failure
    int resource = -1;

    bool status = GetFileDescriptorLimits(fd_soft_limit, fd_hard_limit,
                                          resource);    
    EXPECT_FALSE(status);
    EXPECT_EQ(0, fd_soft_limit);
    EXPECT_EQ(0, fd_hard_limit);
}
#endif // __linux__

TEST(ResourceLimitsTest, SetFileDescriptorLimitsSuccess)
{
    uint64_t fd_soft_limit, fd_hard_limit;
    
    // Get current limits.
    bool status = GetFileDescriptorLimits(fd_soft_limit, fd_hard_limit);
    ASSERT_TRUE(status);

    // Increment soft limit and attempt to set a new soft limit.
    // Gitlab runners have soft limit = hard limit, so decrement
    // by ten instead of increment.
    printf("Initial FD soft/hard limit: %llu/%llu\n", fd_soft_limit, fd_hard_limit);
    uint64_t new_fd_soft_limit = fd_soft_limit - 10;
    status = SetFileDescriptorLimits(new_fd_soft_limit);
    EXPECT_TRUE(status);

    // Get the limits and confirm that it was updated.
    status = GetFileDescriptorLimits(fd_soft_limit, fd_hard_limit);
    EXPECT_TRUE(status);
    EXPECT_EQ(new_fd_soft_limit, fd_soft_limit);
}

TEST(ResourceLimitsTest, SetFileDescriptorLimitsExceedsHardLimit)
{
    uint64_t fd_soft_limit, fd_hard_limit;
    
    // Get current limits.
    bool status = GetFileDescriptorLimits(fd_soft_limit, fd_hard_limit);
    ASSERT_TRUE(status);

    // Set soft limit to value greater than hard limit.
    uint64_t new_fd_soft_limit = fd_hard_limit + 1;
    status = SetFileDescriptorLimits(new_fd_soft_limit);

    EXPECT_FALSE(status);
}
