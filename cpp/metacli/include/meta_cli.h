#ifndef META_CLI_H_
#define META_CLI_H_

#include <string>
#include <set>
#include "cli_group.h"
#include "meta_cli_help_strings.h"
#include "meta_cli_config_params.h"

inline bool ConfigureMetaCLI(CLIGroup& cli_group, 
    MetaCLIConfigParams& config, bool& help_requested, 
    bool& show_version)
{

    ////////////////////////////////////////////////////////////////////////////////
    //                                  help CLI
    ////////////////////////////////////////////////////////////////////////////////

    std::shared_ptr<CLIGroupMember> cli_help = cli_group.AddCLI("tip", 
        MetaCLIHelpStrings::high_level_description, "clihelp");
    cli_help->AddOption("--help", "-h", MetaCLIHelpStrings::help_request_help, false, 
        help_requested, true);


    ////////////////////////////////////////////////////////////////////////////////
    //                                  version CLI
    ////////////////////////////////////////////////////////////////////////////////

    std::shared_ptr<CLIGroupMember> cli_version = cli_group.AddCLI("tip", 
        MetaCLIHelpStrings::high_level_description, "cliversion");
    cli_version->AddOption("--version", "-v", MetaCLIHelpStrings::version_request_help, 
        false, show_version, true);

    ////////////////////////////////////////////////////////////////////////////////
    //                                  subcommand
    ////////////////////////////////////////////////////////////////////////////////

    std::shared_ptr<CLIGroupMember> subcommand_cli = cli_group.AddCLI("tip",
        MetaCLIHelpStrings::high_level_description, "clisubcommand");

    // Positional arg, required
    std::set<std::string> permitted_subcommands{"parse", "translate"};
    subcommand_cli->AddOption<std::string>("subcommand", MetaCLIHelpStrings::subcommand_help, 
        config.subcommand_, true)->ValidatePermittedValuesAre(permitted_subcommands);

    if(!cli_group.CheckConfiguration())
        return false;
    return true;
}

#endif  // META_CLI_H_