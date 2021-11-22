#include "stream_buffering.h"

bool SetStreamBufferAndMode(FILE* stream, char* buffer, int mode, size_t size)
{
    if (stream == NULL)
    {
        printf("SetStreamBufferAndMode: stream is NULL\n");
        return false;
    }

    if (!((mode == _IOFBF) || (mode == _IOLBF) || (mode == _IONBF)))
    {
        printf("SetStreamBufferAndMode: mode must be one of [_IOFBF, _IOLBF, _IONBF]\n");
        return false;
    }

    if ((size < 2) || (size > INT_MAX))
    {
        printf("SetStreamBufferAndMode: size must be in [2, INT_MAX]\n");
        return false;
    }

    int ret = setvbuf(stream, buffer, mode, size);
    if (ret != 0)
    {
        printf("SetStreamBufferAndMode: setvbuf returned {:d}\n", ret);
        return false;
    }
    return true;
}

bool SetLineBuffering(FILE* stream)
{
    return SetStreamBufferAndMode(stream, NULL, _IOLBF, 2048);
}
