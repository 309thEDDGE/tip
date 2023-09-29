#ifndef META_CLI_HELP_STRINGS_H_
#define META_CLI_HELP_STRINGS_H_

#include <string>
namespace MetaCLIHelpStrings
{
    const std::string high_level_description = 
        R"(Entry point for Chapter 10 parser, translator, or other utilities)";

    const std::string help_request_help = 
        R"(Print usage information.)";

    const std::string version_request_help = 
        R"(Print version, either latest commit git SHA or tag.)";

    const std::string subcommand_help =
        R"(Subcommands include: parse, translate)";

    const std::string parse_help =
        R"(Parse a Chapter 10 file.)";

    const std::string translate_help =
        R"(Translate parsed intermediate files to engineering unit data.)";
}
#endif  // META_CLI_HELP_STRINGS_H_