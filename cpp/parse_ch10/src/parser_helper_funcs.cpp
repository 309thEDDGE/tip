#include "parser_helper_funcs.h"

bool ValidatePaths(const std::string& str_input_path, const std::string& str_output_path,
                   const std::string& str_conf_dir, const std::string& str_log_dir,
                   ManagedPath& input_path, ManagedPath& output_path,
                   ManagedPath& conf_file_path, ManagedPath& schema_file_path, ManagedPath& log_dir)
{
    ArgumentValidation av;

    if (!av.CheckExtension(str_input_path, "ch10", "c10"))
    {
        printf(
            "User-defined input path (%s) does not have one of the case-insensitive "
            "extensions: ch10, c10\n", str_input_path.c_str());
        return false;
    }
    // Check utf-8 conformity and verify existence of input path
    if (!av.ValidateInputFilePath(str_input_path, input_path))
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
        if (!av.ValidateDirectoryPath(str_output_path, output_path))
        {
            printf("User-defined output path is not a directory: %s\n",
                   str_output_path.c_str());
            return false;
        }
    }

    // Create parse configuration path from base conf path
    std::string conf_file_name = "parse_conf.yaml";
    ManagedPath default_conf_base_path({"..", "conf"});
    if (!av.ValidateDefaultInputFilePath(default_conf_base_path.absolute(),
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
    if (!av.ValidateDefaultInputFilePath(default_schema_path, user_schema_path.RawString(),
                                         schema_file_name, schema_file_path))
    {
        printf("Failed to create yaml schema file path\n");
        return false;
    }

    ManagedPath default_log_dir({"..", "logs"});
    if (!av.ValidateDefaultOutputDirectory(default_log_dir, str_log_dir, log_dir, true))
        return false;

    return true;
}

bool StartParse(ManagedPath input_path, ManagedPath output_path,
                ParserConfigParams config, double& duration, 
                const ProvenanceData& prov_data)
{
    // Get start time.
    auto start_time = std::chrono::high_resolution_clock::now();

    // Configure checks configuration, prepares output paths,
    // and calculates internal quantities in preparation for
    // parsing.
    ParseManager pm;
    if (!pm.Configure(input_path, output_path, config))
        return false;

    // Begin parsing of Ch10 data by starting workers.
    if (!pm.Parse(config))
        return false;

    // Record metadata
    if (!pm.RecordMetadata(input_path, config, prov_data))
        return false;

    // Get stop time and print duration.
    auto stop_time = std::chrono::high_resolution_clock::now();
    duration = (stop_time - start_time).count() / 1.0e9;

    return true;
}

bool SetupLogging(const ManagedPath& log_dir)
{
    // Use the chart on this page for logging level reference:
    // https://www.tutorialspoint.com/log4j/log4j_logging_levels.htm

    try
    {
        // Set global logging level
        spdlog::set_level(spdlog::level::debug);

        // Setup async thread pool.
        spdlog::init_thread_pool(8192, 2);

        // Rotating logs maxima
        int max_size = 1024 * 512;  // 512 KB
        int max_files = 5;

        // Console sink
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::info);
        //console_sink->set_level(spdlog::level::debug);
        console_sink->set_pattern("%^[%T %L] %v%$");

        // ParseManager log
        ManagedPath pm_log_path = log_dir / std::string("parse_manager.log");
        auto pm_log_sink = std::make_shared<spdlog::sinks::rotating_file_sink_st>(pm_log_path.string(),
                                                                                  max_size, max_files);
        pm_log_sink->set_level(spdlog::level::debug);
        pm_log_sink->set_pattern("[%D %T %L] %v");

        // List of sinks for ParseManager, console logger
        spdlog::sinks_init_list pm_sinks = {console_sink, pm_log_sink};

        // Create and register the logger for ParseManager log and console.
        auto pm_logger = std::make_shared<spdlog::logger>("pm_logger", pm_sinks.begin(), pm_sinks.end());
        pm_logger->set_level(spdlog::level::debug);
        spdlog::register_logger(pm_logger);

        // Parser primary threaded file sink.
        max_size = 1024 * 1024 * 10;  // 10 MB
        max_files = 20;
        ManagedPath parser_log_path = log_dir / std::string("parser.log");
        auto parser_log_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(parser_log_path.string(),
                                                                                      max_size, max_files);
        parser_log_sink->set_level(spdlog::level::debug);
        parser_log_sink->set_pattern("[%D %T %L] [%@] %v");

        // List of sinks for async parser, console logger
        spdlog::sinks_init_list parser_sinks = {parser_log_sink, console_sink};

        // Create and register async parser, consoler logger
        auto parser_logger = std::make_shared<spdlog::async_logger>("parser_logger", parser_sinks,
                                                                    spdlog::thread_pool(), spdlog::async_overflow_policy::block);
        parser_logger->set_level(spdlog::level::debug);
        spdlog::register_logger(parser_logger);

        // Register as default logger to simplify calls in ParseWorker and deeper where
        // the majority of parser logging calls will be made.
        spdlog::set_default_logger(parser_logger);
    }
    catch (const spdlog::spdlog_ex& ex)
    {
        printf("SetupLogging() failed: %s\n", ex.what());
        return false;
    }
    return true;
}
