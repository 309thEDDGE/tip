#ifndef YAMLSV_LOG_ITEM_H_
#define YAMLSV_LOG_ITEM_H_

#include <cstdio>
#include <cstdint>
#include <ostream>
#include <sstream>
#include <iomanip>
#include <map>
#include <string>

enum class LogLevel : uint8_t
{
    Trace = 0,
    Debug = 1,
    Info = 2,
    Warn = 3,
    Error = 4,
    Fatal = 5,
    Off = 6
};

const std::map<LogLevel, std::string> loglevel_to_string_map = {
    {LogLevel::Trace, "TRACE"},
    {LogLevel::Debug, "DEBUG"},
    {LogLevel::Info, "INFO"},
    {LogLevel::Warn, "WARN"},
    {LogLevel::Error, "ERROR"},
    {LogLevel::Fatal, "FATAL"},
    {LogLevel::Off, "OFF"}};

class LogItem
{
   public:
    LogLevel log_level;
    uint8_t log_value;
    std::string message;

    LogItem() : log_level(LogLevel::Trace), message(), log_value(0) {}
    LogItem(LogLevel level, std::string msg) : log_level(level),
                                               message(msg),
                                               log_value(static_cast<uint8_t>(level)) {}

    void Print() const
    {
        printf("[%-5s] %s\n", loglevel_to_string_map.at(log_level).c_str(),
               message.c_str());
    }

    void PrintToStream(std::ostream& stream) const
    {
        stream << "[" << std::setw(5) << std::left << loglevel_to_string_map.at(log_level)
               << "] " << message << std::endl;
    }
};

#endif