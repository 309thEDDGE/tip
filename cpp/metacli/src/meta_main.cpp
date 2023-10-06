#include "meta_main.h"
#include "translate_tabular_1553_main.h"
#include "translate_tabular_arinc429_main.h"
#include "binary_comparison_main.h"

int MetaMain(int argc, char** argv)
{
    if (!SetLineBuffering(stdout))
        return EX_OSERR;

    CLIGroup cli_group;
    MetaCLIConfigParams config;
    if(!ConfigureMetaCLI(cli_group, config))
    {
        printf("ConfigureMetaCLI failed\n");
        return EX_SOFTWARE;
    }

    CLIGroup translate_cli_group;
    if(!ConfigureTranslateCLI(translate_cli_group, config))
    {
        printf("ConfigureTranslateCLI failed\n");
        return EX_SOFTWARE;
    }

    CLIGroup util_cli_group;
    if(!ConfigureUtilityCLI(util_cli_group, config))
    {
        printf("ConfigureUtilityCLI failed\n");
        return EX_SOFTWARE;
    }

    return ExecuteMetaCLI(argc, argv, cli_group, translate_cli_group,
        util_cli_group, config);
}

int ExecuteMetaCLI(int argc, char** argv, CLIGroup& cli_group, 
    CLIGroup& translate_cli_group, CLIGroup& util_cli_group, 
    MetaCLIConfigParams& config)
{
    std::string nickname = "";
    std::shared_ptr<CLIGroupMember> cli;
    int retcode = 0;

    // Restrict to single argument. If arg is -h, --help, -v, or
    // --version then those options would occur anyway. If the arg
    // is one of the subcommands then I want to ignore the flag, which
    // would take precedence. Example tip parse -h, don't want presence
    // of -h to trigger the 'tip' help, only the 'parse' help.
    int temp_argc = argc;
    char** temp_argv = argv;
    ArgumentValidation::ArgSelectTo(2, temp_argc, &temp_argv);
    if ((retcode = cli_group.Parse(temp_argc, temp_argv, nickname, cli)) != 0)
    {
        if (temp_argc == 1)
            printf("%s", cli_group.MakeHelpString().c_str());
        return retcode;
    }

    if (config.help_requested_ && nickname == "clihelp")
    {
        printf("%s", cli_group.MakeHelpString().c_str());
        return EX_OK;
    }

    if (config.show_version_ && nickname == "cliversion")
    {
        printf("tip version %s\n", GetVersionString().c_str());
        return EX_OK;
    }

    if (config.subcommand_ == "parse")
    {
        ArgumentValidation::ArgSelectFrom(1, argc, &argv);
        return Ch10ParseMain(argc, argv);
    }

    if (config.subcommand_ == "translate")
    {
        if((retcode = ExecuteTranslateCLI(argc, argv, 
            translate_cli_group, config)) != EX__MAX)
            return retcode;
    }

    if (config.subcommand_ == "util")
    {
        if((retcode = ExecuteUtilCLI(argc, argv, util_cli_group, 
            config)) != EX__MAX)
            return retcode;
    }

    return EX_OK;
}

int ExecuteTranslateCLI(int argc, char** argv, CLIGroup& translate_cli_group,
    MetaCLIConfigParams& config)
{   
    std::string nickname = "";
    std::shared_ptr<CLIGroupMember> cli;
    int retcode = 0;

    int temp_argc = argc;
    char** temp_argv = argv;

    ArgumentValidation::ArgSelectFrom(1, temp_argc, &temp_argv);
    ArgumentValidation::ArgSelectTo(2, temp_argc, &temp_argv);
    if((retcode = translate_cli_group.Parse(temp_argc, temp_argv, nickname, cli)) != 0)
    {
        if (temp_argc == 1)
            printf("%s", translate_cli_group.MakeHelpString().c_str());
        return retcode;
    }

    if (config.help_requested_ && nickname == "translateclihelp")
    {
        printf("%s", translate_cli_group.MakeHelpString().c_str());
        return EX_OK;
    }

    temp_argc = argc;
    temp_argv = argv;
    ArgumentValidation::ArgSelectFrom(2, temp_argc, &temp_argv);
    // ArgumentValidation::ArgSelectTo(2, temp_argc, &temp_argv);
    if (config.translate_subcommand_ == "1553")
    {
        return TranslateTabular1553Main(temp_argc, temp_argv);
    }
    else if (config.translate_subcommand_ == "arinc429")
    {
        return TranslateTabularARINC429Main(temp_argc, temp_argv);
    }

    return EX__MAX;
}

int ExecuteUtilCLI(int argc, char** argv, CLIGroup& util_cli_group,
    MetaCLIConfigParams& config)
{
    std::string nickname = "";
    std::shared_ptr<CLIGroupMember> cli;
    int retcode = 0;
    int temp_argc = argc;
    char** temp_argv = argv;

    ArgumentValidation::ArgSelectFrom(1, temp_argc, &temp_argv);
    ArgumentValidation::ArgSelectTo(2, temp_argc, &temp_argv);
    if((retcode = util_cli_group.Parse(temp_argc, temp_argv, nickname, cli)) != 0)
    {
        if (temp_argc == 1)
            printf("%s", util_cli_group.MakeHelpString().c_str());
        return retcode;
    }

    if (config.help_requested_ && nickname == "utilclihelp")
    {
        printf("%s", util_cli_group.MakeHelpString().c_str());
        return EX_OK;
    }

    temp_argc = argc;
    temp_argv = argv;
    ArgumentValidation::ArgSelectFrom(2, temp_argc, &temp_argv);
    if (config.util_subcommand_ == "bincomp")
    {
        return BinCompMain(temp_argc, temp_argv);
    }
    else if (config.util_subcommand_ == "pqcomp")
    {
        printf("pqcomp subcommand!\n");
    }
    else if (config.util_subcommand_ == "vidextract")
    {
        printf("vidextract subcommand!\n");
    }
    else if (config.util_subcommand_ == "validyaml")
    {
        printf("validyaml subcommand!\n");
    }

    return EX__MAX;
}
