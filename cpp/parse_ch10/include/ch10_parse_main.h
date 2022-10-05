
#ifndef CH10_PARSE_MAIN_H_
#define CH10_PARSE_MAIN_H_

#include <string>
#include <cstdio>
#include <iostream>
#include "parse_manager.h"
#include "parser_config_params.h"
#include "parser_cli.h"
#include "managed_path.h"
#include "argument_validation.h"
#include "spdlog/spdlog.h"
#include "spdlog/async.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "stream_buffering.h"
#include "version_info.h"
#include "yaml_schema_validation.h"
#include "parser_metadata.h"

int Ch10ParseMain(int argc, char** argv);

bool ValidatePaths(const std::string& str_input_path, const std::string& str_output_path,
                   const std::string& str_log_dir, ManagedPath& input_path, 
                   ManagedPath& output_path, ManagedPath& log_dir, const ArgumentValidation* av);

bool StartParse(ManagedPath input_path, ManagedPath output_path,
                const ParserConfigParams& config, double& duration);

bool SetupLogging(const ManagedPath& log_dir, spdlog::level::level_enum stdout_level);  // GCOVR_EXCL_LINE

#endif  // CH10_PARSE_MAIN_H_