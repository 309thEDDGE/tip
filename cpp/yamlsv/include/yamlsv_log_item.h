#ifndef YAMLSV_LOG_ITEM_H_
#define YAMLSV_LOG_ITEM_H_

#include <cstdio>
#include <cstdint>

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

class LogItem
{
public:
	LogLevel log_level;
	std::string message;

	LogItem() : log_level(LogLevel::TRACE), message() {}
	LogItem(LogLevel level, std::string msg) : log_level(level), message(msg) {}
};

#endif