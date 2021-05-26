
#ifndef SPDLOG_SETUP_HELPER_FUNCS_H_
#define SPDLOG_SETUP_HELPER_FUNCS_H_

#include <string>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/null_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

void CreateNullLoggerWithName(std::string logger_name);
void CreateStdoutLoggerWithName(std::string logger_name);


#endif