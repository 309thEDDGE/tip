#ifndef YAMLSV_LOG_ITEM_H_
#define YAMLSV_LOG_ITEM_H_

#include <cstdio>
#include <cstdint>
#include <map>

enum class LogLevel : uint8_t
{
	TRACE = 0,
	DDEBUG = 1,
	INFO = 2,
	WARN = 3,
	ERROR = 4,
	FATAL = 5,
	OFF = 6
};

const std::map<LogLevel, std::string> loglevel_to_string_map = {
	{LogLevel::TRACE, "TRACE"},
	{LogLevel::DDEBUG, "DEBUG"},
	{LogLevel::INFO, "INFO"},
	{LogLevel::WARN, "WARN"},
	{LogLevel::ERROR, "ERROR"},
	{LogLevel::FATAL, "FATAL"},
	{LogLevel::OFF, "OFF"}
};

class LogItem
{
public:
	LogLevel log_level;
	std::string message;

	LogItem() : log_level(LogLevel::TRACE), message() {}
	LogItem(LogLevel level, std::string msg) : log_level(level), message(msg) {}

	void Print() const
	{
		printf("[%-5s] %s\n", loglevel_to_string_map.at(log_level).c_str(),
			message.c_str());
	}
};

#endif