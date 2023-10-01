#include "meta_main.h"

int MetaMain(int argc, char** argv)
{
    if (!SetLineBuffering(stdout))
        return EX_OSERR;

    CLIGroup cli_group;
    CLIGroup translate_cli_group;
    MetaCLIConfigParams config;
    bool help_requested = false;
    bool show_version = false;
    if(!ConfigureMetaCLI(cli_group, translate_cli_group, config, 
        help_requested, show_version))
    {
        printf("ConfigureMetaCLI failed\n");
        return EX_SOFTWARE;
    }

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
        return retcode;
    }

    if (help_requested && nickname == "clihelp")
    {
        printf("%s", cli_group.MakeHelpString().c_str());
        return EX_OK;
    }

    if (show_version && nickname == "cliversion")
    {
        printf("tip version %s\n", GetVersionString().c_str());
        return EX_OK;
    }

    if (config.subcommand_ == "parse")
    {
        printf("parse subcommand!\n");
        ArgumentValidation::ArgSelectFrom(1, argc, &argv);
        return Ch10ParseMain(argc, argv);
    }

    if (config.subcommand_ == "translate")
    {
        printf("translate subcommand!\n");

        temp_argc = argc;
        temp_argv = argv;
        ArgumentValidation::ArgSelectFrom(1, temp_argc, &temp_argv);
        ArgumentValidation::ArgSelectTo(2, temp_argc, &temp_argv);
        if((retcode = translate_cli_group.Parse(temp_argc, temp_argv, nickname, cli)) != 0)
        {
            printf("failed translate cli, first arg %s\n", temp_argv[0]);
            return retcode;
        }

        if (help_requested && nickname == "clihelp")
        {
            printf("%s", translate_cli_group.MakeHelpString().c_str());
            return EX_OK;
        }

        if (show_version && nickname == "cliversion")
        {
            printf("tip translate version %s\n", GetVersionString().c_str());
            return EX_OK;
        }

        temp_argc = argc;
        temp_argv = argv;
        ArgumentValidation::ArgSelectFrom(2, temp_argc, &temp_argv);
        if (config.translate_subcommand_ == "1553")
        {
            printf("translate 1553 subcommand!\n");
        }

        if (config.translate_subcommand_ == "arinc429")
        {
            printf("translate arinc429 subcommand!\n");
        }
    }


    return EX_OK;
}