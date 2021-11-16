
#ifndef RESOURCE_LIMITS_H_
#define RESOURCE_LIMITS_H_

//This include first to satisfy linter
#ifdef __linux__
#include <sys/resource.h>
#endif  // __linux__

#include <cstdint>
#include <cstdio>

#ifdef __linux__

#include <cerrno>
#include <cstring>

bool GetFileDescriptorLimits(uint64_t& fd_soft_limit,
                             uint64_t& fd_hard_limit,
                             int resource = RLIMIT_NOFILE);

#elif defined __WIN64

// https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/setmaxstdio?view=msvc-160
const uint64_t _SETMAXSTDIO_MAXIMUM = 8192;

bool GetFileDescriptorLimits(uint64_t& fd_soft_limit,
                             uint64_t& fd_hard_limit);

#endif  // #ifdef __WIN64

bool SetFileDescriptorLimits(const uint64_t& new_fd_soft_limit);

#endif  // #ifndef RESOURCE_LIMITS_H_
