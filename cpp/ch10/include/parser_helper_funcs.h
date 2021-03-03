
#include "parse_manager.h"
#include "parser_config_params.h"
#include "managed_path.h"
#include <memory>
#include "spdlog/spdlog.h"
#include "spdlog/async.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/rotating_file_sink.h"

bool ValidateConfig(ParserConfigParams& config, std::string config_path, 
	ManagedPath& final_config_path);

bool ValidatePaths(char* arg1, char* arg2, ManagedPath& input_path, ManagedPath& output_path);

bool StartParse(ManagedPath input_path, ManagedPath output_path,
	ParserConfigParams config, double& duration);

bool SetupLogging(const ManagedPath& log_dir);