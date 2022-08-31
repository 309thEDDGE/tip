#ifndef TRANSLATOR_CLI_1553_H_
#define TRANSLATOR_CLI_1553_H_

#include <memory>
#include <string>
#include <set>
#include <vector>
#include <map>
#include "cli_group.h"
#include "translator_cli_help_strings_1553.h"
#include "translation_config_params.h"

inline bool ConfigureTranslatorCLI(CLIGroup& cli_group, TranslationConfigParams& config, 
    bool& help_requested, bool& show_version, bool& show_dts_info, 
    const std::string& high_level_description)
{
    ////////////////////////////////////////////////////////////////////////////////
    //                                  help CLI
    ////////////////////////////////////////////////////////////////////////////////

    std::shared_ptr<CLIGroupMember> cli_help = cli_group.AddCLI(TRANSLATE_1553_EXE_NAME, 
        high_level_description, "clihelp");
    cli_help->AddOption("--help", "-h", help_request_help, false, 
        help_requested, true);


    ////////////////////////////////////////////////////////////////////////////////
    //                                  version CLI
    ////////////////////////////////////////////////////////////////////////////////

    std::shared_ptr<CLIGroupMember> cli_version = cli_group.AddCLI(TRANSLATE_1553_EXE_NAME, 
        high_level_description, "cliversion");
    cli_version->AddOption("--version", "-v", version_request_help, 
        false, show_version, true);

    
    ////////////////////////////////////////////////////////////////////////////////
    //                                  DTS 1553 help CLI
    ////////////////////////////////////////////////////////////////////////////////

    std::shared_ptr<CLIGroupMember> cli_dts = cli_group.AddCLI(TRANSLATE_1553_EXE_NAME, 
        high_level_description, "clidts");
    cli_dts->AddOption("--dts_help", "", dts_request_help, false, show_dts_info, true);


    ////////////////////////////////////////////////////////////////////////////////
    //                                  full CLI
    ////////////////////////////////////////////////////////////////////////////////

    std::shared_ptr<CLIGroupMember> cli = cli_group.AddCLI(TRANSLATE_1553_EXE_NAME,
        high_level_description, "clifull");

    // Positional args, required
    cli->AddOption("input_data", input_parsed_1553_help, config.input_data_path_str_, true);
    cli->AddOption("input_dts", input_dts1553_help, config.input_dts_path_str_, true);
    
    // Optional args
    cli->AddOption<std::string>("--output_path", "-o", output_path_help, "<input path directory>", 
        config.output_path_str_)->DefaultUseParentDirOf(config.input_data_path_str_);

    cli->AddOption<std::string>("--log_path", "-l", log_dir_help, "<output path directory>",
        config.log_path_str_)->DefaultUseValueOf(config.output_path_str_);

    cli->AddOption("--thread_count", "-t", translate_thread_count_help, 1,
        config.translate_thread_count_)->ValidateInclusiveRangeIs(1, 128);

    std::map<std::string, std::string> tmats_busname_corrections_default{};
    cli->AddOption<std::map<std::string, std::string>>("--tmats_busname_corrections", 
        "-T", tmats_busname_corrections_help, tmats_busname_corrections_default, 
        config.tmats_busname_corrections_);

    cli->AddOption("--vote_threshold", "-V", vote_threshold_help, 1, 
        config.vote_threshold_)->ValidateInclusiveRangeIs(1, INT_MAX); 

    std::vector<std::string> busname_exclusions_default{};
    cli->AddOption<std::vector<std::string>>("--busname_exclusion", "-b", bus_name_exclusions_help,
        busname_exclusions_default, config.bus_name_exclusions_); 

    std::vector<std::string> select_spec_msgs_default{};
    cli->AddOption<std::vector<std::string>>("--select_msg", "-m", select_specific_messages_help,
        select_spec_msgs_default, config.select_specific_messages_);

    std::set<std::string> permitted_log_levels{"trace", "debug", "info", "warn", "error", 
        "critical", "off"};
    cli->AddOption<std::string>("--log_level", "-L", stdout_log_level_help, "info", 
        config.stdout_log_level_)->ValidatePermittedValuesAre(permitted_log_levels);

    // Flags
    cli->AddOption("--tmats", "", use_tmats_busmap_help, false, config.use_tmats_busmap_);
    cli->AddOption("--prompt_user", "-p", prompt_user_help, false, config.prompt_user_);
    cli->AddOption("--stop_after_busmap", "-s", stop_after_bus_map_help, false, 
        config.stop_after_bus_map_);
    cli->AddOption("--check_tmats", "", vote_method_checks_tmats_help, false, 
        config.vote_method_checks_tmats_);
    // cli->AddOption("--halt", "", halt_after_table_creation_help, false, 
    //     config.exit_after_table_creation_);
    cli->AddOption("--disable_sys_limits", "", auto_sys_limits_help, true, config.auto_sys_limits_);

    std::string disable_validation_help = "Do not process input DTS1553 yaml file with "
        "schema validator. Primarily for testing purposes. Use at your own risk.";
    cli->AddOption("--disable_dts_validation", "", disable_validation_help, false, 
        config.disable_dts_schema_validation_);


    if(!cli_group.CheckConfiguration())
        return false;
    return true;
}

#endif  // TRANSLATOR_CLI_1553_H_