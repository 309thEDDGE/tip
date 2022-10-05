#include <string>
#include <memory>

#include "spdlog/spdlog.h"
#include "spdlog/sinks/null_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

struct PersistentLogger
{
    std::string name;
    std::shared_ptr<spdlog::logger> logger;

    template <typename T>
    PersistentLogger(const std::string &name_, T creator) : name(name_),
                                                            logger(creator(name_))
    {
    }
};

static std::shared_ptr<spdlog::logger> CreateNullLogger(const std::string &name)
{
    auto logger = spdlog::create<spdlog::sinks::null_sink_st>(name);
    // for some reason this shouldn't be called here
    //      spdlog::register_logger(logger);
    return logger;
}

static std::shared_ptr<spdlog::logger> CreateStdoutLogger(const std::string &name)
{
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_st>();

    console_sink->set_level(spdlog::level::debug);
    console_sink->set_pattern("%^[%T] [%n] [%l] %v%$");

    auto logger = std::make_shared<spdlog::logger>(name, console_sink);
    logger->set_level(spdlog::level::debug);

    spdlog::register_logger(logger);
    return logger;
}

void EnsureLoggersAreAvailable()
{
    static PersistentLogger ensure_loggers[] = {
        {"pm_logger", CreateNullLogger},
        {"ethf0_logger", CreateStdoutLogger},
        {"npp_logger", CreateStdoutLogger}};

    assert(spdlog::get("pm_logger"));
    assert(spdlog::get("ethf0_logger"));
    assert(spdlog::get("npp_logger"));
}
