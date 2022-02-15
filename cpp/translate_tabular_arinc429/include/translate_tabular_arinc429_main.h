#ifndef TRANSLATE_TABULAR_ARINC429_MAIN_H_
#define TRANSLATE_TABULAR_ARINC429_MAIN_H_

/*
Include parquet_reader.h first to avoid complications
with re-definitions by spdlog headers of arrow defs.
*/
#include "parquet_reader.h"
#include <cstdio>
#include <string>
#include <memory>
#include <chrono>
#include "spdlog/spdlog.h"
#include "spdlog/logger.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "translate_tabular.h"
#include "translate_tabular_context_arinc429.h"
#include "ch10_packet_type.h"
#include "managed_path.h"
#include "argument_validation.h"
#include "tip_md_document.h"
#include "resource_limits.h"

#include "version_info.h"
#include "stream_buffering.h"
#include "provenance_data.h"
#include "translation_config_params.h"
#include "yaml_schema_validation.h"
#include "sha256_tools.h"


int TranslateTabularARINC429Main(int argc, char** argv);

namespace transtab429
{
    bool ValidatePaths(const std::string& str_input_path, const std::string& str_icd_path,
                    const std::string& str_output_dir, const std::string& str_conf_dir,
                    const std::string& str_log_dir, ManagedPath& input_path, ManagedPath& icd_path,
                    ManagedPath& output_dir, ManagedPath& conf_file_path, ManagedPath& conf_schema_file_path,
                    ManagedPath& icd_schema_file_path, ManagedPath& log_dir);

    bool SetupLogging(const ManagedPath& log_dir);

    bool GetParsedMetadata(const ManagedPath& input_md_path, 
        TIPMDDocument& parser_md_doc);

    bool SetSystemLimits(uint8_t thread_count, size_t message_count);
}  // namespace transtab429

#endif  // TRANSLATE_TABULAR_ARINC429_MAIN_H_