#include "parquet_comparison_main.h"

int PqCompMain(int argc, char** argv)
{
    auto t1 = Clock::now();

    CLIGroup cli_group;
    bool help_requested = false;
    std::string truth_path_str("");
    std::string test_path_str("");

    if(!ConfigurePqCompCLI(cli_group, help_requested, truth_path_str, test_path_str))
        return EX_SOFTWARE;

    std::string nickname = "";
    std::shared_ptr<CLIGroupMember> cli;
    int retcode = 0;
    if ((retcode = cli_group.Parse(argc, argv, nickname, cli)) != 0)
    {
        if (argc == 1)
            printf("%s", cli_group.MakeHelpString().c_str());
        return retcode;
    }

    if (help_requested)
    {
        printf("%s", cli_group.MakeHelpString().c_str());
        return EX_OK;
    }

    Comparator comp;
    if((retcode = comp.Initialize(ManagedPath(truth_path_str), ManagedPath(test_path_str))) != 0)
        return retcode;
    bool result = comp.CompareAll();

    auto t2 = Clock::now();
    printf("\nElapsed Time: %" PRId64 " seconds\n",
           std::chrono::duration_cast<std::chrono::seconds>(t2 - t1).count());

    if(!result)
        return 1;
    return EX_OK;
}

bool ConfigurePqCompCLI(CLIGroup& cli_group, bool& help_requested, std::string& truth_path_str, 
    std::string& test_path_str)
{
    std::string exe_name = "pqcompare";
    std::string description = "Compare a test parquet path against a truth parquet path. Input Parquet "
        "paths may either be files or directories with the suffix \".parquet\". Print \"PASS\" (exit code 0) to stdout "
        "if equivalent, \"FAIL\" (exit code 1) if not equivalent, or \"NULL\" (exit code >1) "
        "if the comparison can't be made. "
        "Column count and schema (column label and data type) will be compared first, followed by element-wise "
        "comparison of columns as arrays. All list-type columns are assumed to be arrow::Int32Type.";
    std::shared_ptr<CLIGroupMember> cli_help = cli_group.AddCLI(exe_name, 
    description, "clihelp");
    cli_help->AddOption("--help", "-h", "Show usage information", false, 
    help_requested, true);

    std::shared_ptr<CLIGroupMember> cli_full = cli_group.AddCLI(exe_name,
        description, "clifull");

    std::string truth_path_help = "Full path to TRUTH data";
    cli_full->AddOption("truth_parquet_path", truth_path_help, truth_path_str, true);

    std::string test_path_help = "Full path to TEST data";
    cli_full->AddOption("test_parquet_path", test_path_help, test_path_str, true);

    if(!cli_group.CheckConfiguration())
        return false;

    return true;
}
