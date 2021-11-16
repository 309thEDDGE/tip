// ch10parse.h

#ifndef ARROW_STATIC
#error arrow_static must be defined in conjunction with parquet
#endif

#ifndef PARQUET_STATIC
#error parquet_static must be defined in conjunction with parquet
#endif

#ifndef CH10_PARSE_EXE_NAME
#error "ch10parse.cpp: CH10_PARSE_EXE_NAME must be defined"
#endif

#include "parser_helper_funcs.h"
#include "stream_buffering.h"
#include "version_info.h"

int main(int argc, char* argv[])
{
    if(!SetLineBuffering(stdout))
        return 0;

    if (CheckForVersionArgument(argc, argv))
    {
        printf("%s version %s\n", CH10_PARSE_EXE_NAME, GetVersionString().c_str());
        return 0;
    }

    std::string str_input_path;
    std::string str_output_path;
    std::string str_conf_dir;
    std::string str_log_dir;
    if (!ParseArgs(argc, argv, str_input_path, str_output_path, str_conf_dir,
                   str_log_dir))
        return 0;

    ManagedPath input_path;
    ManagedPath output_path;
    ManagedPath conf_file_path;
    ManagedPath schema_file_path;
    ManagedPath log_dir;
    if (!ValidatePaths(str_input_path, str_output_path, str_conf_dir, str_log_dir,
                       input_path, output_path, conf_file_path, schema_file_path, log_dir))
        return 0;

    ParserConfigParams config;
    std::string config_schema_path = "";
    ManagedPath final_config_path;
    ManagedPath final_schema_path;
    if (!ValidateConfig(config, conf_file_path, schema_file_path))
        return 0;

    if (!SetupLogging(log_dir))
        return 0;

    spdlog::get("pm_logger")->info("{:s} version: {:s}", CH10_PARSE_EXE_NAME, GetVersionString());
    spdlog::get("pm_logger")->info("Ch10 file path: {:s}", input_path.absolute().RawString());
    spdlog::get("pm_logger")->info("Output path: {:s}", output_path.absolute().RawString());
    spdlog::get("pm_logger")->info("Configuration file path: {:s}", conf_file_path.absolute().RawString());
    spdlog::get("pm_logger")->info("Configuration schema path: {:s}", schema_file_path.absolute().RawString());
    spdlog::get("pm_logger")->info("Log directory: {:s}", log_dir.absolute().RawString());

    double duration;
    StartParse(input_path, output_path, config, duration);
    spdlog::get("pm_logger")->info("Duration: {:.3f} sec", duration);

    // Avoid deadlock in windows, see
    // http://stackoverflow.com/questions/10915233/stdthreadjoin-hangs-if-called-after-main-exits-when-using-vs2012-rc
    spdlog::shutdown();

    return 0;
}
