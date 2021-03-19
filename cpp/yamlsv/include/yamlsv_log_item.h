#ifndef YAMLSV_LOG_ITEM_H_
#define YAMLSV_LOG_ITEM_H_

#include <cstdio>
#include <cstdint>
#include <map>

enum class LogLevel : uint8_t
{
	LLTRACE = 0,
	LLDEBUG = 1,
	LLINFO = 2,
	LLWARN = 3,
	LLERROR = 4,
	LLFATAL = 5,
	LLOFF = 6,
};

const std::map<LogLevel, std::string> loglevel_to_string_map = {
	{LogLevel::LLTRACE, "TRACE"},
	{LogLevel::LLDEBUG, "DEBUG"},
	{LogLevel::LLINFO, "INFO"},
	{LogLevel::LLWARN, "WARN"},
	{LogLevel::LLERROR, "ERROR"},
	{LogLevel::LLFATAL, "FATAL"},
	{LogLevel::LLOFF, "OFF"}
};

class LogItem
{
public:
	LogLevel log_level;
	uint8_t log_value;
	std::string message;

	LogItem() : log_level(LogLevel::LLTRACE), message(), log_value(0) {}
	LogItem(LogLevel level, std::string msg) : log_level(level), 
		message(msg), log_value(static_cast<uint8_t>(level)) {}

	void Print() const
	{
		printf("[%-5s] %s\n", loglevel_to_string_map.at(log_level).c_str(),
			message.c_str());
	}
};

#endif