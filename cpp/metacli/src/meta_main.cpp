#include "meta_main.h"

int MetaMain(int argc, char** argv)
{
    if (!SetLineBuffering(stdout))
        return EX_OSERR;

    CLIGroup cli_group;
    MetaCLIConfigParams config;
    bool help_requested = false;
    bool show_version = false;
    if(!ConfigureMetaCLI(cli_group, config, help_requested, show_version))
    {
        printf("ConfigureMetaCLI failed\n");
        return EX_SOFTWARE;
    }

    std::string nickname = "";
    std::shared_ptr<CLIGroupMember> cli;
    int retcode = 0;
    if ((retcode = cli_group.Parse(argc, argv, nickname, cli)) != 0)
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
        printf("TIP version %s\n", GetVersionString().c_str());
        return EX_OK;
    }

    if (config.subcommand_ == "parse")
    {
        printf("parse subcommand!\n");
    }

    if (config.subcommand_ == "translate")
    {
        printf("translate subcommand!\n");
    }


    return EX_OK;
}