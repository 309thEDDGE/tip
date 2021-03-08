
#ifndef SPDLOG_SETUP_HELPER_FUNCS_H_
#define SPDLOG_SETUP_HELPER_FUNCS_H_

#include <string>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/null_sink.h"

void CreateNullLoggerWithName(std::string logger_name)
{
	auto logger = spdlog::create<spdlog::sinks::null_sink_st>(logger_name);

	// Register as default if default macros are used.
	spdlog::set_default_logger(logger);
}

#endif