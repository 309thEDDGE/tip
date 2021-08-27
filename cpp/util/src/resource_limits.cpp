#include "resource_limits.h"


#ifdef __linux__
bool GetFileDescriptorLimits(uint64_t& fd_soft_limit, uint64_t& fd_hard_limit,
        int resource)
{
     // Initialize outputs in case of failure
    fd_soft_limit = 0;
    fd_hard_limit = 0;

    struct rlimit limits;

    if(getrlimit(resource, &limits) != 0)
    {
        printf("GetFileDescriptorLimits(): Failed to get limits, %s\n",
               strerror(errno));
       return false; 
    }

    fd_soft_limit = static_cast<uint64_t>(limits.rlim_cur);
    fd_hard_limit = static_cast<uint64_t>(limits.rlim_max);
    return true;    
}
#elif defined __WIN64
bool GetFileDescriptorLimits(uint64_t& fd_soft_limit, uint64_t& fd_hard_limit)
{
    fd_soft_limit = 0;
    fd_hard_limit = 0;

    // See https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/getmaxstdio?view=msvc-160
    // Don't know if there is a return value which indicates error. I'll guess -1.
    if((fd_soft_limit = _getmaxstdio()) == -1)
    {
        printf("GetFileDescriptorLimits(): Failed to get limits\n");
        return false;
    }
    fd_hard_limit = _SETMAXSTDIO_MAXIMUM;
    return true;
}
#endif // #elif defined __WIN64

bool SetFileDescriptorLimits(const uint64_t& new_fd_soft_limit)
{
    uint64_t fd_soft_limit, fd_hard_limit;

    // Get current limits
    if(!GetFileDescriptorLimits(fd_soft_limit, fd_hard_limit))
        return false;

#ifdef __linux__
    // Set limits struct
    struct rlimit limits;
    limits.rlim_cur = static_cast<rlim_t>(new_fd_soft_limit);
    limits.rlim_max = static_cast<rlim_t>(fd_hard_limit);

    if(setrlimit(RLIMIT_NOFILE, &limits) != 0)
    {
        // Remove the strerror(errno) because it gives "Invalid Argument"
        // which is not useful and may be misleading.
        printf("SetFileDescriptorLimits(): Failed to set limit "
               "to %llu\n", new_fd_soft_limit);
        printf("Current hard limit = %llu\n", fd_hard_limit);
        return false;
    }
#elif defined __WIN64
    int new_max = static_cast<int>(new_fd_soft_limit);
    
    // Avoid a request that exceeds the maximum because Microsoft is bonkers
    // and doesn't just throw an error, _setmaxstdio returns -1 and invokes
    // an invalid parameter handler, which is a mess to deal with. If it's not
    // dealt with it kills the process. See 
    // https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/setmaxstdio?view=msvc-160
    // and https://docs.microsoft.com/en-us/cpp/c-runtime-library/parameter-validation?view=msvc-160
    if(new_max > _SETMAXSTDIO_MAXIMUM)
    {
        printf("SetFileDescriptorLimits(): Requested limit %llu is greater "
               "than maximum %llu\n", new_fd_soft_limit, _SETMAXSTDIO_MAXIMUM);
        return false;
    }

    int ret = _setmaxstdio(new_max);
    if(ret == -1)
    {
        printf("SetFileDescriptorLimits(): Failed to set limit "
                "to %d\n", new_max);
        printf("Current hard limit = %llu\n", _SETMAXSTDIO_MAXIMUM);
        return false;
    }
    
    if (ret != new_max)
    {
        printf("SetFileDescriptorLimits(): Failed to set correct limit "
               "%d, current limit %d\n", new_max, ret);
        printf("Current hard limit = %llu\n", _SETMAXSTDIO_MAXIMUM);
        return false;
    }
#endif
    return true;
}

