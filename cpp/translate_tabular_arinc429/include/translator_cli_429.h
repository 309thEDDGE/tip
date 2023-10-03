#ifndef TRANSLATOR_CLI_429_H_
#define TRANSLATOR_CLI_429_H_

#include <memory>
#include <string>
#include <set>
#include <vector>
#include <map>
#include "cli_group.h"
#include "translator_cli_help_strings_429.h"
#include "translation_config_params.h"

inline bool ConfigureTranslatorCLI429(CLIGroup& cli_group, TranslationConfigParams& config, 
    bool& help_requested, bool& show_version, bool& show_dts_info, 
    const std::string& high_level_description)
{
    ////////////////////////////////////////////////////////////////////////////////
    //                                  help CLI
    ////////////////////////////////////////////////////////////////////////////////

    std::shared_ptr<CLIGroupMember> cli_help = cli_group.AddCLI(TRANSLATE_429_EXE_NAME, 
        high_level_description, "clihelp");
    cli_help->AddOption("--help", "-h", 
        TranslateARINC429CLIHelpStrings::help_request_help, false, 
        help_requested, true);


    ////////////////////////////////////////////////////////////////////////////////
    //                                  version CLI
    ////////////////////////////////////////////////////////////////////////////////

    std::shared_ptr<CLIGroupMember> cli_version = cli_group.AddCLI(TRANSLATE_429_EXE_NAME, 
        high_level_description, "cliversion");
    cli_version->AddOption("--version", "-v", 
        TranslateARINC429CLIHelpStrings::version_request_help, 
        false, show_version, true);

    
    ////////////////////////////////////////////////////////////////////////////////
    //                                  DTS 429 help CLI
    ////////////////////////////////////////////////////////////////////////////////

    std::shared_ptr<CLIGroupMember> cli_dts = cli_group.AddCLI(TRANSLATE_429_EXE_NAME, 
        high_level_description, "clidts");
    cli_dts->AddOption("--dts_help", "", 
        TranslateARINC429CLIHelpStrings::dts_request_help, false, show_dts_info, true);


    ////////////////////////////////////////////////////////////////////////////////
    //                                  full CLI
    ////////////////////////////////////////////////////////////////////////////////

    std::shared_ptr<CLIGroupMember> cli = cli_group.AddCLI(TRANSLATE_429_EXE_NAME,
        high_level_description, "clifull");

    // Positional args, required
    cli->AddOption("input_data", 
        TranslateARINC429CLIHelpStrings::input_parsed_data_help, config.input_data_path_str_, true);
    cli->AddOption("input_dts", 
        TranslateARINC429CLIHelpStrings::input_dts429_help, config.input_dts_path_str_, true);
    
    // Optional args
    cli->AddOption<std::string>("--output_path", "-o", 
        TranslateARINC429CLIHelpStrings::output_path_help, "<input path directory>", 
        config.output_path_str_)->DefaultUseParentDirOf(config.input_data_path_str_);

    cli->AddOption<std::string>("--log_path", "-l", 
        TranslateARINC429CLIHelpStrings::log_dir_help, "<output path directory>",
        config.log_path_str_)->DefaultUseValueOf(config.output_path_str_);

    cli->AddOption("--thread_count", "-t", 
        TranslateARINC429CLIHelpStrings::translate_thread_count_help, 1,
        config.translate_thread_count_)->ValidateInclusiveRangeIs(1, 128);

    std::set<std::string> permitted_log_levels{"trace", "debug", "info", "warn", "error", 
        "critical", "off"};
    cli->AddOption<std::string>("--log_level", "-L", 
        TranslateARINC429CLIHelpStrings::stdout_log_level_help, "info", 
        config.stdout_log_level_)->ValidatePermittedValuesAre(permitted_log_levels)->SetSimpleHelpFormat();

    cli->AddOption("--disable_sys_limits", "", 
        TranslateARINC429CLIHelpStrings::auto_sys_limits_help, true, 
        config.auto_sys_limits_);

    if(!cli_group.CheckConfiguration())
        return false;
    return true;
}

#endif  // TRANSLATOR_CLI_429_H_