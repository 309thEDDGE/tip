#ifndef CLI_CONF_H_
#define CLI_CONF_H_

#include <string>

class CLIConf
{
    private:
        static const std::string whitespace_code;

        // Minimum user-configurable help message char width
        static const size_t minimum_help_string_char_width; 

    public:
        CLIConf() {}
        static std::string GetWhitespaceCode();
        static size_t GetMinHelpStrCharWidth();

};

#endif // CLI_CONF_H_