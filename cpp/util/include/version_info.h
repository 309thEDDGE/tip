#ifndef VERSION_INFO_H_
#define VERSION_INFO_H_

#include <cstdio>
#include <string>
#include <vector>

// Require VERSION_STRING macro
#ifndef VERSION_STRING
#error "version_info.h: VERSION_STRING macro not defined!"
#endif // #ifndef VERSION_STRING

/*
Check for version flag as first argument to main.

Args:
    argc    --> Argument count
    argv    --> Array of arguments

Return:
    True if '-v' or '--version' is first argument, false otherwise
*/
bool CheckForVersionArgument(int argc, char* argv[]);

/*
Return the version string as a std::string object.
*/
std::string GetVersionString();

#endif // #ifndef VERSION_INFO_H_