#ifndef VALIDATE_YAML_MAIN_H_
#define VALIDATE_YAML_MAIN_H_

#include "sysexits.h"
#include "yaml_schema_validation.h"
#include "file_reader.h"
#include "managed_path.h"
#include "cli_group.h"

int ValidateYamlMain(int argc, char** argv);

bool ConfigureValidateYamlCLI(CLIGroup& cli_group, bool& help_requested, std::string& yaml_path_str,
    std::string& schema_path_str);

#endif  // VALIDATE_YAML_MAIN_H_
