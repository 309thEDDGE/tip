
#ifndef PARSER_HELPER_FUNCS_H_
#define PARSER_HELPER_FUNCS_H_

#include "parse_manager.h"
#include "parser_config_params.h"
#include "managed_path.h"
#include "argument_validation.h"
#include "yaml_schema_validation.h"
#include "spdlog/spdlog.h"
#include "spdlog/async.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/rotating_file_sink.h"

bool ParseArgs(int argc, char* argv[], std::string& str_input_path, std::string& str_output_path,
	std::string& str_conf_path, std::string& str_log_dir);

bool ValidatePaths(const std::string& str_input_path, const std::string& str_output_path,
	const std::string& str_conf_path, const std::string& str_log_dir, 
	ManagedPath& input_path, ManagedPath& output_path,
	ManagedPath& conf_file_path, ManagedPath& schema_file_path, ManagedPath& log_dir);

bool ValidateConfig(ParserConfigParams& config, const ManagedPath& conf_file_path,
	const ManagedPath& schema_file_path);

bool StartParse(ManagedPath input_path, ManagedPath output_path,
	ParserConfigParams config, double& duration);

bool SetupLogging(const ManagedPath& log_dir);

bool ValidateLogDir(const std::string& user_log_dir, ManagedPath& final_log_dir);

#endif