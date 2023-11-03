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
#include <set>
#include <chrono>
#include <vector>
#include <unordered_map>
#include <map>
#include <iostream>
#include <fstream>
#include "sysexits.h"
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
#include "arinc429_data.h"
#include "dts429.h"
#include "organize_429_icd.h"
#include "yaml-cpp/yaml.h"
#include "version_info.h"
#include "stream_buffering.h"
#include "provenance_data.h"
#include "translation_config_params.h"
#include "yaml_schema_validation.h"
#include "sha256_tools.h"
#include "translator_cli_429.h"
#include "dts_arinc429_schema.h"


int TranslateTabularARINC429Main(int argc, char** argv);

namespace transtab429
{
    int ValidatePaths(const std::string& str_input_path, const std::string& str_icd_path,
                    const std::string& str_output_dir, const std::string& str_log_dir,
                    ManagedPath& input_path, ManagedPath& icd_path,
                    ManagedPath& output_dir, ManagedPath& log_dir, ArgumentValidation* av);

    bool SetupLogging(const ManagedPath& log_dir, 
        spdlog::level::level_enum stdout_log_level,
        spdlog::level::level_enum file_log_level);

    bool IngestICD(DTS429* dts429, Organize429ICD* org429, ARINC429Data& data429,
                    const std::vector<std::string>& icd_lines, size_t& arinc_message_count,
                    YAML::Node& parser_md_node,
                    std::vector<std::string>& subchan_name_lookup_misses);

    int GetParsedMetadata(const ManagedPath& input_md_path,
        TIPMDDocument& parser_md_doc);

    bool SetSystemLimits(uint8_t thread_count, size_t message_count);

    bool GetFileContents(std::string file_name, std::vector<std::string>& file_contents);

    int Translate(size_t thread_count, const ManagedPath& input_path,
                const ManagedPath& output_dir, const ARINC429Data& icd,
                const ManagedPath& translated_data_dir,
                const ManagedPath& output_base_name,
                std::set<std::string>& translated_msg_names,
                std::map<uint32_t, std::map<uint32_t, std::set<uint16_t>>>& chanid_busnum_labels_map);

    bool RecordMetadata(const TranslationConfigParams& config,
                    const ManagedPath& translated_data_dir,
                    const ManagedPath& dts_path,
                    const ManagedPath& input_path,
                    const std::set<std::string>& translated_messages,
                    const ProvenanceData& prov_data, const TIPMDDocument& parser_md_doc,
                    const std::map<uint32_t, std::map<uint32_t, std::set<uint16_t>>>& chanid_busnum_labels);
}  // namespace transtab429

#endif  // TRANSLATE_TABULAR_ARINC429_MAIN_H_