#include "terminal.h"

#ifdef __WIN64

#include <windows.h>

bool GetTerminalSize(size_t& rows, size_t& cols)
{
    rows = 0;
    cols = 0;
    CONSOLE_SCREEN_BUFFER_INFO csbi;

    HANDLE hdl = GetStdHandle(STD_OUTPUT_HANDLE);
    if(hdl == INVALID_HANDLE_VALUE)
    {
        printf("GetTerminalSize(): Failed to get stdout handle\n");
        return false;
    }

    int result = GetConsoleScreenBufferInfo(hdl, &csbi);
    if(result == 0)
    {
        printf("GetTerminalSize(): Failed to get console buffer info\n");
        return false;
    }

    cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1; 

    return true;
}

#elif defined __linux__

#include <sys/ioctl.h>
#include <unistd.h>

bool GetTerminalSize(size_t& rows, size_t& cols)
{
    rows = 0;
    cols = 0;

    struct winsize w;
    int result = ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    if(result != 0)
    {
        printf("GetTerminalSize(): Call to ioctl failed\n");
        return false;
    }

    rows = static_cast<size_t>(w.ws_row);
    cols = static_cast<size_t>(w.ws_col);
    return true;
}

#endif
