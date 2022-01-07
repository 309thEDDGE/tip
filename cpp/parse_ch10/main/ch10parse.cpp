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

#include <map>
#include <string>
#include "parser_helper_funcs.h"
#include "stream_buffering.h"
#include "version_info.h"
#include "provenance_data.h"
#include "argument_validation.h"
#include "yaml_schema_validation.h"

int main(int argc, char* argv[])
{
    if (!SetLineBuffering(stdout))
        return 0;

    if (CheckForVersionArgument(argc, argv))
    {
        printf("%s version %s\n", CH10_PARSE_EXE_NAME, GetVersionString().c_str());
        return 0;
    }

    std::map<int, std::string> def_args = {
        {1, "input_path"}, {2, "output_path"}, {3, "conf_dir"}, {4, "log_dir"}
    };
    std::map<int, std::string> options = {
        {1, "Usage: " CH10_PARSE_EXE_NAME " <ch10 path> [output path] [config dir] [log dir]\n"
            "Needs ch10 input path."},
        {2, ""},
        {4, "If a configuration directory is specified by the user then "
            "an output log directory must also be specified"}
    };
    std::map<std::string, std::string> args;
    ArgumentValidation av;
    if(!av.TestOptionalArgCount(argc, options)) 
        return 0;
    if(!av.ParseArgs(argc, argv, def_args, args, true))
        return 0;

    ManagedPath input_path;
    ManagedPath output_path;
    ManagedPath conf_file_path;
    ManagedPath schema_file_path;
    ManagedPath log_dir;
    if (!ValidatePaths(args.at("input_path"), args.at("output_path"), 
                       args.at("conf_dir"), args.at("log_dir"),
                       input_path, output_path, conf_file_path, schema_file_path, log_dir))
        return 0;

    if (!SetupLogging(log_dir))
        return 0;

    ProvenanceData prov_data;
    if(!GetProvenanceData(input_path.absolute(), static_cast<size_t>(150e6), prov_data)) 
        return 0;

    ParserConfigParams config;
    YamlSV ysv;
    std::string config_string;
    std::string schema_string;
    if(!ysv.ValidateDocument(conf_file_path, schema_file_path, config_string, schema_string))
        return 0;

    if(!config.InitializeWithConfigString(config_string))
        return 0;

    spdlog::get("pm_logger")->info("{:s} version: {:s}", CH10_PARSE_EXE_NAME, GetVersionString());
    spdlog::get("pm_logger")->info("Ch10 file path: {:s}", input_path.absolute().RawString());
    spdlog::get("pm_logger")->info("Ch10 hash: {:s}", prov_data.hash);
    spdlog::get("pm_logger")->info("Output path: {:s}", output_path.absolute().RawString());
    spdlog::get("pm_logger")->info("Configuration file path: {:s}", conf_file_path.absolute().RawString());
    spdlog::get("pm_logger")->info("Configuration schema path: {:s}", schema_file_path.absolute().RawString());
    spdlog::get("pm_logger")->info("Log directory: {:s}", log_dir.absolute().RawString());

    double duration;
    StartParse(input_path, output_path, config, duration, prov_data);
    spdlog::get("pm_logger")->info("Duration: {:.3f} sec", duration);

    // Avoid deadlock in windows, see
    // http://stackoverflow.com/questions/10915233/stdthreadjoin-hangs-if-called-after-main-exits-when-using-vs2012-rc
    spdlog::shutdown();

    return 0;
}
