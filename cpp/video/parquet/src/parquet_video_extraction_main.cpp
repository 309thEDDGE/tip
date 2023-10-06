#include "parquet_video_extraction_main.h"

int PqVidExtractMain(int argc, char** argv)
{
    auto t1 = Clock::now();

    CLIGroup cli_group;
    bool help_requested = false;
    std::string input_path_str("");
    std::string output_path_str("");

    if(!ConfigurePqVidExtractCLI(cli_group, help_requested, input_path_str, output_path_str))
        return EX_SOFTWARE;

    std::string nickname = "";
    std::shared_ptr<CLIGroupMember> cli;
    int retcode = 0;
    if ((retcode = cli_group.Parse(argc, argv, nickname, cli)) != 0)
    {
        return retcode;
    }

    if (help_requested)
    {
        printf("%s", cli_group.MakeHelpString().c_str());
        return EX_OK;
    }

    ManagedPath input_path(input_path_str);
    ManagedPath output_path(output_path_str);
    ParquetVideoExtraction pe;
    if((retcode = pe.Initialize(input_path, output_path)) != 0)
    {
        return retcode;
    }

    retcode = pe.ExtractTS();

    auto t2 = Clock::now();
    printf("\nElapsed Time: %" PRId64 " seconds\n", std::chrono::duration_cast<std::chrono::seconds>(t2 - t1).count());

    return retcode;
}

bool ConfigurePqVidExtractCLI(CLIGroup& cli_group, bool& help_requested, std::string& input_path_str, 
    std::string& output_path_str)
{
    std::string exe_name = "parquet_video_extractor";
    std::string description = "Create Transport Stream (TS) files from extracted video "
        "format 0 data in Parquet format. Output TS files will be labled \"video_channel_id_XX.ts\", "
        "in which XX is the channel ID given in the video packet header and to which all video data "
        "in a specific video file will be relevant.";
    std::shared_ptr<CLIGroupMember> cli_help = cli_group.AddCLI(exe_name, 
    description, "clihelp");
    cli_help->AddOption("--help", "-h", "Show usage information", false, 
    help_requested, true);

    std::shared_ptr<CLIGroupMember> cli_full = cli_group.AddCLI(exe_name,
        description, "clifull");

    std::string input_pq_dir_help = "Path to directory of video format 0 data in Parquet format. "
        "These data are the output of tip_parse if video format 0 packets are present in the "
        "Chapter 10 file. A correct input directory will be of the form: \"<ch10 name>_VIDEO_DATA_F0.parquet\"";
    cli_full->AddOption("input_parquet_dir", input_pq_dir_help, input_path_str, true);

    std::string output_dir_help = "Path in which to create output directory and write TS files.";
    cli_full->AddOption<std::string>("--output_dir", "-o", output_dir_help, "<INPUT_PARQUET_DIR parent dir>", 
        output_path_str)->DefaultUseParentDirOf(input_path_str);

    if(!cli_group.CheckConfiguration())
        return false;

    return true;
}