#include "parquet_translation_manager.h"
#include "translation_master.h"
#include "yaml_reader.h"
#include "metadata.h"
#include "file_reader.h"
#include "dts1553.h"
#include "bus_map.h"
#include "parquet_reader.h"
#include "translation_config_params.h"
#include "managed_path.h"
#include "parse_text.h"
#include <arrow/api.h>
#include <arrow/io/api.h>
#include <iostream>
#include <cstdlib>
#include <set>

bool GetArguments(int argc, char* argv[], ManagedPath& input_path,
	ManagedPath& icd_path);
bool InitializeConfig(std::string conf_path, TranslationConfigParams& tcp);
bool PrepareICDAndBusMap(DTS1553& dts1553, const ManagedPath& input_path,
	const ManagedPath& dts_path, bool stop_after_bus_map, bool prompt_user,
	uint64_t vote_threshold, bool vote_method_checks_tmats,
	std::vector<std::string> bus_exclusions,
	std::map<std::string, std::string>& tmats_bus_name_corrections,
	bool use_tmats_busmap, std::map<uint64_t, std::string>& chanid_to_bus_name_map,
	std::set<uint64_t>& excluded_channel_ids);
bool SynthesizeBusMap(DTS1553& dts1553, const ManagedPath& input_path, bool prompt_user,
	uint64_t vote_threshold, bool vote_method_checks_tmats,
	std::vector<std::string> bus_exclusions,
	std::map<std::string, std::string>& tmats_bus_name_corrections,
	bool use_tmats_busmap, std::map<uint64_t, std::string>& chanid_to_bus_name_map,
	std::set<uint64_t>& excluded_channel_ids);
bool MTTranslate(TranslationConfigParams config, const ManagedPath& input_path,
	ICDData icd, const ManagedPath& dts_path,
	std::map<uint64_t, std::string>& chanid_to_bus_name_map,
	const std::set<uint64_t>& excluded_channel_ids, double& duration);
bool Translate(TranslationConfigParams config, const ManagedPath& input_path,
	ICDData icd, const ManagedPath& dts_path,
	std::map<uint64_t, std::string>& chanid_to_bus_name_map,
	const std::set<uint64_t>& excluded_channel_ids, double& duration);
bool RecordMetadata(TranslationConfigParams config, const ManagedPath& translated_data_dir,
	const ManagedPath& dts_path, std::map<uint64_t, std::string>& chanid_to_bus_name_map,
	const std::set<uint64_t>& excluded_channel_ids, const ManagedPath& input_path,
	const std::set<std::string>& translated_messages);