#ifndef META_CLI_HELP_STRINGS_H_
#define META_CLI_HELP_STRINGS_H_

#include <string>
namespace MetaCLIHelpStrings
{
    const std::string high_level_description = 
        R"(Entry point for Chapter 10 parser, translator, and other utilities)";

    const std::string help_request_help = 
        R"(Print usage information)";

    const std::string version_request_help = 
        R"(Print version, either latest commit git SHA or tag)";

    const std::string subcommand_help =
        R"(Commands include: parse, translate, util)";

    const std::string parse_help =
        R"(Parse a Chapter 10 file.)";

    const std::string translate_hl_desc = 
        R"(Entry point for translating tip parse origin-specific
        intermediate products into outputs with engineering units)";

    const std::string translate_help =
        R"(Translate parsed intermediate files to engineering unit data)";

    const std::string translate_subcommand_help = 
        R"(Subcommands include: 1553, arinc429)"; 

    const std::string util_hl_desc = 
        R"(Entry point for utilities)";

    const std::string util_help = 
        R"(Utilities for automated end-to-end testing and extraction)";

    const std::string util_subcommand_help = 
        R"(Subcommands include: bincomp, pqcomp, vidextract, 
        validyaml)";
}
#endif  // META_CLI_HELP_STRINGS_H_