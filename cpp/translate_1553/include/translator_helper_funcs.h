#ifndef TRANSLATOR_HELPER_FUNCS_H_
#define TRANSLATOR_HELPER_FUNCS_H_

#include "parquet_translation_manager.h"
#include "translation_master.h"
#include "yaml_reader.h"
#include "metadata.h"
#include "dts1553.h"
#include "bus_map.h"
#include "parquet_reader.h"
#include "translation_config_params.h"
#include "argument_validation.h"
#include "yaml_schema_validation.h"
#include <arrow/api.h>
#include <arrow/io/api.h>
#include <iostream>
#include <cstdlib>
#include <set>

bool ParseArgs(int argc, char* argv[], std::string& str_input_path,
	std::string& str_icd_path, std::string& str_output_dir, std::string& str_conf_dir,
	std::string& str_log_dir);

bool ValidatePaths(const std::string& str_input_path, const std::string& str_icd_path,
	const std::string& str_output_dir, const std::string& str_conf_dir, 
	const std::string& str_log_dir,	ManagedPath& input_path, ManagedPath& icd_path, 
	ManagedPath& output_dir, ManagedPath& conf_file_path, ManagedPath& conf_schema_file_path, 
	ManagedPath& icd_schema_file_path, ManagedPath& log_dir);

bool ValidateConfSchema(const ManagedPath& conf_file_path, 
	const ManagedPath& conf_schema_file_path, std::string& conf_doc);

bool ValidateDTS1553YamlSchema(const ManagedPath& icd_path, 
	const ManagedPath& icd_schema_file_path);

bool SetupLogging(const ManagedPath& log_dir);

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

#endif