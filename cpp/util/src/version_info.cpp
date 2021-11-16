#include "version_info.h"

bool CheckForVersionArgument(int argc, char* argv[])
{
    if (argc < 2)
        return false;

    std::vector<std::string> variants({"-v", "--version"});
    std::string candidate(argv[1]);

    for (std::vector<std::string>::const_iterator it = variants.cbegin();
         it != variants.cend(); ++it)
    {
        if (candidate.compare(*it) == 0)
            return true;
    }

    return false;
}

std::string GetVersionString()
{
    std::string ver_str = VERSION_STRING;
    return ver_str;
}
