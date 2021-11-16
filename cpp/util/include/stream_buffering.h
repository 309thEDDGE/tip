#ifndef STREAM_BUFFERING_H_
#define STREAM_BUFFERING_H_

#include <cstdio>
#include <climits>

/*
Wrapper for setvbuf. Input checking to avoid errors and Windows
deadly "invalid parameter handler." See 
https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/setvbuf?view=msvc-170
or 
https://linux.die.net/man/3/setvbuf

Return:
    True if an invalid input argument is used or the return value
    from setvbuf is 0; otherwise false.
*/
bool SetStreamBufferAndMode(FILE* stream, char* buffer, int mode, size_t size);



/*
Set line buffering on a stream. Sets the automatically allocated
buffer to size 2048.

Args:
    stream  --> I/O stream for which buffering will be set
                to line buffering

Return:
    True if no errors; false otherwise
*/
bool SetLineBuffering(FILE* stream);

#endif  // STREAM_BUFFERING_H_