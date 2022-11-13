#include "cli_conf.h"

const std::string CLIConf::whitespace_code = "--..--";
const size_t CLIConf::minimum_help_string_char_width = 40;

std::string CLIConf::GetWhitespaceCode()
{
    return whitespace_code;
}

size_t CLIConf::GetMinHelpStrCharWidth()
{
    return minimum_help_string_char_width;
}