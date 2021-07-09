#include "spdlog_setup_helper_funcs.h"

void CreateNullLoggerWithName(std::string logger_name)
{
    auto logger = spdlog::create<spdlog::sinks::null_sink_st>(logger_name);

    // Register as default if default macros are used.
    spdlog::set_default_logger(logger);
}

void CreateStdoutLoggerWithName(std::string logger_name)
{
    spdlog::set_level(spdlog::level::debug);
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_st>();
    //console_sink->set_level(spdlog::level::info);
    console_sink->set_level(spdlog::level::debug);
    console_sink->set_pattern("%^[%T] [%n] [%l] %v%$");
    spdlog::sinks_init_list pm_sinks = {console_sink};

    // Create and register the logger for ParseManager log and console.
    //auto logger = std::make_shared<spdlog::logger>(logger_name, pm_sinks.begin(), pm_sinks.end());
    auto logger = std::make_shared<spdlog::logger>(logger_name, console_sink);
    logger->set_level(spdlog::level::debug);
    spdlog::register_logger(logger);
    spdlog::set_default_logger(logger);
}