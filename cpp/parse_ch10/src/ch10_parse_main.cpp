#include "ch10_parse_main.h"


int Ch10ParseMain(int argc, char** argv)
{
    if (!SetLineBuffering(stdout))
        return 0;

    if (CheckForVersionArgument(argc, argv))
    {
        printf(CH10_PARSE_EXE_NAME " version %s\n", GetVersionString().c_str());
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
                       input_path, output_path, conf_file_path, schema_file_path, log_dir, &av))
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

    spdlog::get("pm_logger")->info(CH10_PARSE_EXE_NAME " {:s} version: {:s}", GetVersionString());
    spdlog::get("pm_logger")->info("Ch10 file path: {:s}", input_path.absolute().RawString());
    spdlog::get("pm_logger")->info("Ch10 hash: {:s}", prov_data.hash);
    spdlog::get("pm_logger")->info("Output path: {:s}", output_path.absolute().RawString());
    spdlog::get("pm_logger")->info("Configuration file path: {:s}", conf_file_path.absolute().RawString());
    spdlog::get("pm_logger")->info("Configuration schema path: {:s}", schema_file_path.absolute().RawString());
    spdlog::get("pm_logger")->info("Log directory: {:s}", log_dir.absolute().RawString());

    double duration;
    ParseManager pm;
    StartParse(input_path, output_path, config, duration, prov_data, &pm);
    spdlog::get("pm_logger")->info("Duration: {:.3f} sec", duration);

    // Avoid deadlock in windows, see
    // http://stackoverflow.com/questions/10915233/stdthreadjoin-hangs-if-called-after-main-exits-when-using-vs2012-rc
    spdlog::shutdown();

    return 0;

}

bool ValidatePaths(const std::string& str_input_path, const std::string& str_output_path,
                   const std::string& str_conf_dir, const std::string& str_log_dir,
                   ManagedPath& input_path, ManagedPath& output_path,
                   ManagedPath& conf_file_path, ManagedPath& schema_file_path, 
                   ManagedPath& log_dir, const ArgumentValidation* av)
{
    // std::vector<std::string> ch10_exts{"ch10", "c10"};
    if (!av->CheckExtension(str_input_path, {"ch10", "c10"}))
    {
        printf(
            "User-defined input path (%s) does not have one of the case-insensitive "
            "extensions: ch10, c10\n", str_input_path.c_str());
        return false;
    }
    // Check utf-8 conformity and verify existence of input path
    if (!av->ValidateInputFilePath(str_input_path, input_path))
    {
        printf("User-defined input path is not a file/does not exist: %s\n",
               str_input_path.c_str());
        return false;
    }

    // If no output path is specified, use the input path.
    if (str_output_path == "")
    {
        output_path = input_path.absolute().parent_path();
    }
    else
    {
        // Check if the path conforms to utf-8 and exists
        if (!av->ValidateDirectoryPath(str_output_path, output_path))
        {
            printf("User-defined output path is not a directory: %s\n",
                   str_output_path.c_str());
            return false;
        }
    }

    // Create parse configuration path from base conf path
    std::string conf_file_name = "parse_conf.yaml";
    ManagedPath default_conf_base_path({"..", "conf"});
    if (!av->ValidateDefaultInputFilePath(default_conf_base_path.absolute(),
                                         str_conf_dir, conf_file_name, conf_file_path))
    {
        printf(
            "The constructed path may be the default path, relative to CWD "
            "(\"../conf\"). Specify an explicit conf path to remedy.\n");
        return false;
    }

    // Construct the schema path. The user schema path is constructed from
    // the user conf path only if the user conf path is not an empty string.
    std::string schema_file_name = "tip_parse_conf_schema.yaml";
    ManagedPath default_schema_path({"..", "conf", "yaml_schemas"});
    ManagedPath user_schema_path(std::string(""));
    if (str_conf_dir != "")
        user_schema_path = ManagedPath({str_conf_dir, "yaml_schemas"});
    if (!av->ValidateDefaultInputFilePath(default_schema_path, user_schema_path.RawString(),
                                         schema_file_name, schema_file_path))
    {
        printf("Failed to create yaml schema file path\n");
        return false;
    }

    ManagedPath default_log_dir({"..", "logs"});
    if (!av->ValidateDefaultOutputDirectory(default_log_dir, str_log_dir, log_dir, true))
        return false;

    return true;
}

bool StartParse(ManagedPath input_path, ManagedPath output_path,
                const ParserConfigParams& config, double& duration, 
                const ProvenanceData& prov_data, ParseManager* pm)
{
    // Get start time.
    auto start_time = std::chrono::high_resolution_clock::now();

    // Configure checks configuration, prepares output paths,
    // and calculates internal quantities in preparation for
    // parsing.
    if (!pm->Configure(input_path, output_path, config))
        return false;

    // Begin parsing of Ch10 data by starting workers.
    if (!pm->Parse(config))
        return false;

    // Record metadata
    if (!pm->RecordMetadata(input_path, config, prov_data))
        return false;

    // Get stop time and print duration.
    auto stop_time = std::chrono::high_resolution_clock::now();
    duration = (stop_time - start_time).count() / 1.0e9;

    return true;
}

bool SetupLogging(const ManagedPath& log_dir)  // GCOVR_EXCL_LINE
{
    // Use the chart on this page for logging level reference:
    // https://www.tutorialspoint.com/log4j/log4j_logging_levels.htm

    try
    {
        // Set global logging level
        spdlog::set_level(spdlog::level::debug);  // GCOVR_EXCL_LINE

        // Setup async thread pool.
        spdlog::init_thread_pool(8192, 2);  // GCOVR_EXCL_LINE

        // Rotating logs maxima
        int max_size = 1024 * 512;  // GCOVR_EXCL_LINE
        int max_files = 5;  // GCOVR_EXCL_LINE

        // Console sink
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();  // GCOVR_EXCL_LINE
        console_sink->set_level(spdlog::level::info);  // GCOVR_EXCL_LINE
        //console_sink->set_level(spdlog::level::debug);  // GCOVR_EXCL_LINE
        console_sink->set_pattern("%^[%T %L] %v%$");  // GCOVR_EXCL_LINE

        // ParseManager log
        ManagedPath pm_log_path = log_dir / std::string("parse_manager.log");  // GCOVR_EXCL_LINE
        auto pm_log_sink = std::make_shared<spdlog::sinks::rotating_file_sink_st>(pm_log_path.string(),  // GCOVR_EXCL_LINE
                                                                                  max_size, max_files);  // GCOVR_EXCL_LINE
        pm_log_sink->set_level(spdlog::level::debug);  // GCOVR_EXCL_LINE
        pm_log_sink->set_pattern("[%D %T %L] %v");  // GCOVR_EXCL_LINE

        // List of sinks for ParseManager, console logger
        spdlog::sinks_init_list pm_sinks = {console_sink, pm_log_sink};  // GCOVR_EXCL_LINE

        // Create and register the logger for ParseManager log and console.
        auto pm_logger = std::make_shared<spdlog::logger>("pm_logger", pm_sinks.begin(), pm_sinks.end());  // GCOVR_EXCL_LINE
        pm_logger->set_level(spdlog::level::debug);  // GCOVR_EXCL_LINE
        spdlog::register_logger(pm_logger);  // GCOVR_EXCL_LINE

        // Parser primary threaded file sink.
        max_size = 1024 * 1024 * 10;  // GCOVR_EXCL_LINE
        max_files = 20;  // GCOVR_EXCL_LINE
        ManagedPath parser_log_path = log_dir / std::string("parser.log");  // GCOVR_EXCL_LINE
        auto parser_log_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(parser_log_path.string(),  // GCOVR_EXCL_LINE
                                                                                      max_size, max_files);  // GCOVR_EXCL_LINE
        parser_log_sink->set_level(spdlog::level::debug);  // GCOVR_EXCL_LINE
        parser_log_sink->set_pattern("[%D %T %L] [%@] %v");  // GCOVR_EXCL_LINE

        // List of sinks for async parser, console logger
        spdlog::sinks_init_list parser_sinks = {parser_log_sink, console_sink};  // GCOVR_EXCL_LINE

        // Create and register async parser, consoler logger
        auto parser_logger = std::make_shared<spdlog::async_logger>("parser_logger", parser_sinks,
                                                                    spdlog::thread_pool(), spdlog::async_overflow_policy::block);  // GCOVR_EXCL_LINE
        parser_logger->set_level(spdlog::level::debug);  // GCOVR_EXCL_LINE
        spdlog::register_logger(parser_logger);  // GCOVR_EXCL_LINE

        // Register as default logger to simplify calls in ParseWorker and deeper where
        // the majority of parser logging calls will be made.
        spdlog::set_default_logger(parser_logger);  // GCOVR_EXCL_LINE
    }
    catch (const spdlog::spdlog_ex& ex)  // GCOVR_EXCL_LINE
    {
        printf("SetupLogging() failed: %s\n", ex.what());  // GCOVR_EXCL_LINE
        return false;  // GCOVR_EXCL_LINE
    }
    return true;  // GCOVR_EXCL_LINE
}
