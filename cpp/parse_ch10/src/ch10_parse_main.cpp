#include "ch10_parse_main.h"


int Ch10ParseMain(int argc, char** argv)
{
    if (!SetLineBuffering(stdout))
        return 71;

    CLIGroup cli_group;
    ParserConfigParams config;
    bool help_requested = false;
    bool show_version = false;
    if(!ConfigureParserCLI(cli_group, config, help_requested, show_version))
    {
        printf("ConfigureParserCLI failed\n");
        return 70;
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
        return 0;
    }

    if (show_version && nickname == "cliversion")
    {
        printf(CH10_PARSE_EXE_NAME " version %s\n", GetVersionString().c_str());
        return 0;
    }
    config.MakeCh10PacketEnabledMap();

    ManagedPath input_path;
    ManagedPath output_path;
    ManagedPath log_dir;
    ArgumentValidation av;
    if ((retcode = ValidatePaths(config.input_path_str_, config.output_path_str_, 
                       config.log_path_str_, input_path, output_path, log_dir, &av)) != 0)
        return retcode;

    if ((retcode = SetupLogging(log_dir, spdlog::level::from_str(config.stdout_log_level_))) != 0)
        return retcode;

    spdlog::get("pm_logger")->info(CH10_PARSE_EXE_NAME " version: {:s}", GetVersionString());
    spdlog::get("pm_logger")->info("Ch10 file path: {:s}", input_path.absolute().RawString());
    spdlog::get("pm_logger")->info("Output path: {:s}", output_path.absolute().RawString());
    spdlog::get("pm_logger")->info("Log directory: {:s}", log_dir.absolute().RawString());


    double duration = 0.0;
    if((retcode = StartParse(input_path, output_path, config, duration)) != 0)
        return retcode;
    spdlog::get("pm_logger")->info("Duration: {:.3f} sec", duration);

    // Avoid deadlock in windows, see
    // http://stackoverflow.com/questions/10915233/stdthreadjoin-hangs-if-called-after-main-exits-when-using-vs2012-rc
    spdlog::shutdown();

    return 0;

}

int ValidatePaths(const std::string& str_input_path, const std::string& str_output_path,
                   const std::string& str_log_dir, ManagedPath& input_path, 
                   ManagedPath& output_path, ManagedPath& log_dir, const ArgumentValidation* av)
{
    if (!av->CheckExtension(str_input_path, {"ch10", "c10"}))
    {
        printf(
            "User-defined input path (%s) does not have one of the case-insensitive "
            "extensions: ch10, c10\n", str_input_path.c_str());
        return 65;
    }
    // Check utf-8 conformity and verify existence of input path
    if (!av->ValidateInputFilePath(str_input_path, input_path))
    {
        printf("User-defined input path is not a file/does not exist: %s\n",
               str_input_path.c_str());
        return 66;
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
            printf("Output path is not a directory: %s\n",
                   str_output_path.c_str());
            return 69;
        }
    }

    if (!av->ValidateDirectoryPath(str_log_dir, log_dir))
    {
        printf("Log path is not a directory: %s\n", str_log_dir.c_str());
        return 69;
    }

    return 0;
}

int StartParse(ManagedPath input_path, ManagedPath output_path,
                const ParserConfigParams& config, double& duration)
{
    // Get start time.
    auto start_time = std::chrono::high_resolution_clock::now();

    int retcode = 0;
    if((retcode = Parse(input_path, output_path, config)) != 0)
        return retcode;

    // Get stop time and print duration.
    auto stop_time = std::chrono::high_resolution_clock::now();
    duration = (stop_time - start_time).count() / 1.0e9;

    return 0;
}

int SetupLogging(const ManagedPath& log_dir, spdlog::level::level_enum stdout_level)  // GCOVR_EXCL_LINE
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
        console_sink->set_level(stdout_level);  // GCOVR_EXCL_LINE
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
        return 70;  // GCOVR_EXCL_LINE
    }
    return 0;  // GCOVR_EXCL_LINE
}
