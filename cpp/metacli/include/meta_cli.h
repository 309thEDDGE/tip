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
        high_level_description, "clihelp");
    cli_help->AddOption("--help", "-h", help_request_help, false, 
        help_requested, true);


    ////////////////////////////////////////////////////////////////////////////////
    //                                  version CLI
    ////////////////////////////////////////////////////////////////////////////////

    std::shared_ptr<CLIGroupMember> cli_version = cli_group.AddCLI("tip", 
        high_level_description, "cliversion");
    cli_version->AddOption("--version", "-v", version_request_help, 
        false, show_version, true);

    ////////////////////////////////////////////////////////////////////////////////
    //                                  subcommand
    ////////////////////////////////////////////////////////////////////////////////

    std::shared_ptr<CLIGroupMember> subcommand_cli = cli_group.AddCLI("tip",
        high_level_description, "clisubcommand");

    // Positional arg, required
    std::set<std::string> permitted_subcommands{"parse", "translate"};
    subcommand_cli->AddOption<std::string>("subcommand", subcommand_help, 
        config.subcommand_, true)->ValidatePermittedValuesAre(permitted_subcommands);

    if(!cli_group.CheckConfiguration())
        return false;
    return true;

    // // Optional args
    // cli->AddOption<std::string>("--output_path", "-o", output_path_help, "<input path directory>", 
    //     config.output_path_str_)->DefaultUseParentDirOf(config.input_path_str_);

    // cli->AddOption<std::string>("--log_path", "-l", log_dir_help, "<output path directory>",
    //     config.log_path_str_)->DefaultUseValueOf(config.output_path_str_);

    // cli->AddOption("--chunk_size", "-c", chunk_bytes_help, 200, 
    //     config.parse_chunk_bytes_)->ValidateInclusiveRangeIs(135, 1000);
    // cli->AddOption("--thread_count", "-t", thread_count_help, 1, 
    //     config.parse_thread_count_)->ValidateInclusiveRangeIs(1, 256);
    // cli->AddOption("--max_read_count", "-m", max_read_count_help, 1000, 
    //     config.max_chunk_read_count_)->ValidateInclusiveRangeIs(1, 10000);
    // cli->AddOption("--worker_offset", "", worker_offset_wait_ms_help, 200, 
    //     config.worker_offset_wait_ms_)->ValidateInclusiveRangeIs(1, 30000);
    // cli->AddOption("--worker_wait", "", worker_shift_wait_help, 200, 
    //     config.worker_shift_wait_ms_)->ValidateInclusiveRangeIs(1, 30000);

    // std::set<std::string> permitted_log_levels{"trace", "debug", "info", "warn", "error", 
    //     "critical", "off"};
    // cli->AddOption<std::string>("--stdout_log_level", "-L", stdout_log_level_help, "info", 
    //     config.stdout_log_level_)->ValidatePermittedValuesAre(permitted_log_levels);

    // // Flags
    // cli->AddOption("--disable_1553f1", "", disable_1553f1_help, false, config.disable_1553f1_);
    // cli->AddOption("--disable_videof0", "", disable_videof0_help, false, config.disable_videof0_);
    // cli->AddOption("--disable_eth0", "", disable_eth0_help, false, config.disable_eth0_);
    // cli->AddOption("--disable_arinc0", "", disable_arinc0_help, false, config.disable_arinc0_);

    // if(!cli_group.CheckConfiguration())
    //     return false;
    // return true;
}


#endif  // META_CLI_H_