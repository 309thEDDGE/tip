#ifndef TERMINAL_H_
#define TERMINAL_H_

#include <cstdio>
#include <string>

/*
Get terminal/console row and column count in character units.

Args:
    rows            --> row count
    cols            --> column count

Return:
    False if an error occurs such as no read access or the stdout 
    descriptor is redirected to a file; true otherwise.
*/
bool GetTerminalSize(size_t& rows, size_t& cols);

#endif  // TERMINAL_H_