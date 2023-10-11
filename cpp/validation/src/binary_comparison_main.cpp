#include "binary_comparison_main.h"

int BinCompMain(int argc, char** argv)
{
    std::streamsize read_size = static_cast<int>(1e6);

    CLIGroup cli_group;
    bool help_requested = false;
    std::string truth_path_str("");
    std::string test_path_str("");

    if(!ConfigureCLI(cli_group, help_requested, truth_path_str, test_path_str))
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

    std::ifstream ifile1;
    std::ifstream ifile2;

    // If either file can't be opened, return 1 to indicate a
    // null comparison.
    if (!OpenFile(truth_path_str.c_str(), ifile1))
        return EX_NOINPUT  ;
    if (!OpenFile(test_path_str.c_str(), ifile2))
        return EX_NOINPUT;

    std::vector<char> file1_data(read_size);
    std::vector<char> file2_data(read_size);
    std::streamsize file1_read_count = 0;
    std::streamsize file2_read_count = 0;

    bool equal_result = false;
    bool pass = false;
    int chunk_count = 0;
    while (true)
    {
        file1_read_count = ReadBytes(ifile1, read_size, file1_data.data());
        file2_read_count = ReadBytes(ifile2, read_size, file2_data.data());

        if (file1_read_count != file2_read_count)
        {
            printf("Read count not equal\n");
            printf("FAIL\n");
            break;
        }

        equal_result = std::equal(file1_data.begin(), file1_data.begin() + file1_read_count, file2_data.begin());
        if (!equal_result)
        {
            printf("Comparison of chunk %02d not equal\n", chunk_count);
            printf("FAIL\n");
            break;
        }

        if (file1_read_count != read_size)
        {
            printf("PASS\n");
            pass = true;
            break;
        }
        chunk_count++;
    }

    ifile1.close();
    ifile2.close();

    if(pass)
        return EX_OK;
    return 1;
}

bool OpenFile(const char* path, std::ifstream& ifs)
{
    ifs.open(path, std::ios::binary);
    if (!(ifs.is_open()))
    {
        printf("Error opening file: %s\n", path);
        return false;
    }
    return true;
}

std::streamsize ReadBytes(std::ifstream& ifs, std::streamsize read_count, char* data)
{
    ifs.read(data, read_count);
    return ifs.gcount();
}

bool ConfigureCLI(CLIGroup& cli_group, bool& help_requested, std::string& truth_path_str, 
    std::string& test_path_str)
{
    std::string exe_name = "bincompare";
    std::string description = "Compare a test file against a truth file, byte by byte. Print "
        "\"PASS\" (exit code 0) to stdout if equivalent, \"FAIL\" (exit code 1) "
        "if not equivalent, or NULL (exit code >1) if the comparison can't be made.";
    std::shared_ptr<CLIGroupMember> cli_help = cli_group.AddCLI(exe_name, 
    description, "clihelp");
    cli_help->AddOption("--help", "-h", "Show usage information", false, 
    help_requested, true);

    std::shared_ptr<CLIGroupMember> cli_full = cli_group.AddCLI(exe_name,
        description, "clifull");

    std::string truth_path_help = "Full path to TRUTH file";
    cli_full->AddOption("truth_file_path", truth_path_help, truth_path_str, true);

    std::string test_path_help = "Full path to TEST file";
    cli_full->AddOption("test_file_path", test_path_help, test_path_str, true);

    if(!cli_group.CheckConfiguration())
        return false;

    return true;
}