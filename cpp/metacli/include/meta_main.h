#ifndef META_MAIN_H_
#define META_MAIN_H_

#include <string>
#include <cstdio>
#include "sysexits.h"
#include "stream_buffering.h"
#include "version_info.h"

// Include source which includes parquet dependencies
// first to avoid the known issue of spdlog and parquet
// dependency issues with types of the same name.
#include "parquet_comparison_main.h"
#include "parquet_video_extraction_main.h"
#include "argument_validation.h"
#include "ch10_parse_main.h"
#include "meta_cli_config_params.h"
#include "meta_cli.h"
#include "translate_tabular_1553_main.h"
#include "translate_tabular_arinc429_main.h"
#include "binary_comparison_main.h"
#include "validate_yaml_main.h"


int MetaMain(int argc, char** argv);

int ExecuteMetaCLI(int argc, char** argv, CLIGroup& cli_group, 
    CLIGroup& translate_cli_group, CLIGroup& util_cli_group, 
    MetaCLIConfigParams& config);

int ExecuteTranslateCLI(int argc, char** argv, CLIGroup& translate_cli_group,
    MetaCLIConfigParams& config);

int ExecuteUtilCLI(int argc, char** argv, CLIGroup& util_cli_group,
    MetaCLIConfigParams& config);



#endif  // META_MAIN_H_