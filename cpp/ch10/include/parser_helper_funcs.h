
#ifndef PARSER_HELPER_FUNCS_H_
#define PARSER_HELPER_FUNCS_H_

#include "parse_manager.h"
#include "parser_config_params.h"
#include "managed_path.h"
#include "argument_validation.h"
#include "yaml_schema_validation.h"

bool ValidateConfig(ParserConfigParams& config, std::string config_path,
	std::string config_schema_path);

bool ValidatePaths(char* arg1, char* arg2, ManagedPath& input_path, ManagedPath& output_path);

bool StartParse(ManagedPath input_path, ManagedPath output_path,
	ParserConfigParams config, double& duration);

#endif