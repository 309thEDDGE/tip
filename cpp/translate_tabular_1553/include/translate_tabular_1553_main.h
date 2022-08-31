#ifndef TRANSLATE_TABULAR_1553_MAIN_H_
#define TRANSLATE_TABULAR_1553_MAIN_H_

/*
Include parquet_reader.h first to avoid complications
with re-definitions by spdlog headers of arrow defs.
*/
#include "parquet_reader.h"
#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <set>
#include <vector>
#include <map>
#include <memory>
#include <string>
#include "spdlog/spdlog.h"
#include "spdlog/logger.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "file_reader.h"
#include "sha256_tools.h"
#include "translate_tabular.h"
#include "translate_tabular_context_1553.h"
#include "ch10_packet_type.h"
#include "yaml_reader.h"
#include "dts1553.h"
#include "bus_map.h"
#include "translation_config_params.h"
#include "argument_validation.h"
#include "resource_limits.h"
#include "provenance_data.h"
#include "tip_md_document.h"
#include "version_info.h"
#include "stream_buffering.h"
#include "yaml_schema_validation.h"
#include "translator_cli_1553.h"

int TranslateTabular1553Main(int argc, char** argv);

namespace transtab1553
{
    bool ValidatePaths(const std::string& str_input_path, const std::string& str_icd_path,
                    const std::string& str_output_dir, const std::string& str_log_dir, 
                    ManagedPath& input_path, ManagedPath& icd_path, ManagedPath& output_dir, 
                    ManagedPath& log_dir, ArgumentValidation* av);

    bool SetupLogging(const ManagedPath& log_dir, spdlog::level::level_enum stdout_level);

    bool IngestICD(DTS1553* dts1553, const ManagedPath& dts_path,
                std::map<std::string, std::string>& msg_name_substitutions,
                std::map<std::string, std::string>& elem_name_substitutions,
                FileReader* fr);

    bool PrepareBusMap(const ManagedPath& input_path, DTS1553& dts1553, 
                    const TIPMDDocument& parser_md_doc, 
                    const TranslationConfigParams& config_params,
                    BusMap* bus_map);

    bool SynthesizeBusMap(DTS1553& dts1553, const TIPMDDocument& parser_md_doc,
                        const TranslationConfigParams& config_params,
                        BusMap* bus_map);

    bool SetSystemLimits(uint8_t thread_count, size_t message_count);

    bool Translate(size_t thread_count, const ManagedPath& input_path,
                const ManagedPath& output_dir, ICDData icd,
                const ManagedPath& translated_data_dir,
                const ManagedPath& output_base_name,
                const std::vector<std::string>& selected_msg_names,
                std::set<std::string>& translated_msg_names);

    bool RecordMetadata(const TranslationConfigParams& config, 
                        const ManagedPath& translated_data_dir,
                        const ManagedPath& dts_path, 
                        const ManagedPath& input_path,
                        const std::set<std::string>& translated_messages,
                        const std::map<std::string, std::string>& msg_name_substitutions,
                        const std::map<std::string, std::string>& elem_name_substitutions,
                        const ProvenanceData& prov_data, const TIPMDDocument& parser_md_doc,
                        const BusMap* bus_map);

    bool GetParsed1553Metadata(const ManagedPath* input_md_path, 
        TIPMDDocument* parser_md_doc, FileReader* fr);
}  // namespace transtab1553
#endif  // TRANSLATE_TABULAR_1553_MAIN_H_
