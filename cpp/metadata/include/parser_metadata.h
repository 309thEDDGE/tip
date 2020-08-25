#ifndef PARSER_METADATA_H
#define PARSER_METADATA_H

#include <map>
#include <set>
#include <cstdint>
#include <string>
#include <filesystem>
#include <unordered_map>
#include <fstream>
#include "parse_text.h"
#include "parquet_milstd1553f1.h"
#include "yaml_reader.h"
#include "yaml-cpp/yaml.h"

class ParserMetadata
{
private:
	ParseText parse_text_;
	std::string chanid_to_lruaddrs_metadata_string_;
	std::string tmats_chanid_to_source_metadata_string_;
	std::string tmats_chanid_to_type_metadata_string_;
	std::map<uint64_t, std::set<uint64_t>> chanid_to_lruaddrs_map_;
	std::map<uint64_t, std::string> tmats_chanid_to_source_map_;
	std::map<uint64_t, std::string> tmats_chanid_to_type_map_;
	std::string chanid_to_lruaddrs_map_key_;
	std::string tmats_chanid_to_source_map_key_;
	std::string tmats_chanid_to_type_map_key_;
	std::string output_path_;
	void print_chanid_to_lruaddrs_map();
	void print_tmats_chanid_to_source_map();
	void print_tmats_chanid_to_type_map();
	void create_output_parquet_path(std::filesystem::path parquet_dir);
	void create_output_yaml_path(std::filesystem::path parquet_dir);
	void recover_tmats_chanid_to_source_map();
	void recover_chanid_to_lruaddrs_map();
	void recover_tmats_chanid_to_type_map();

	template<typename Key, typename Val>
	void emit_simple_map(YAML::Emitter& e, 
		const std::map<Key, Val>& input_map, const std::string& key);

	template<typename Key, typename Val>
	void emit_compound_map(YAML::Emitter& e, 
		const std::map<Key, std::set<Val>>& input_map, const std::string& key);

	std::vector<size_t> positions_of(std::string str, std::string search_str);
	//std::vector<std::string> split(std::string in_str, std::string split_str);

public:
	ParserMetadata();
	void create_chanid_to_lruaddrs_metadata_strings(const std::map<uint32_t, std::set<uint16_t>>& map1,
		const std::map<uint32_t, std::set<uint16_t>>& map2);
	void create_tmats_channel_source_metadata_string(const std::map<uint32_t, std::string>& tmats_map);
	void create_tmats_channel_type_metadata_string(const std::map<uint32_t, std::string>& tmats_map);
	std::string get_chanid_to_lruaddrs_metadata_string();
	void set_chanid_to_lruaddrs_metadata_string(std::string& md_str);

	std::map<uint64_t, std::set<uint64_t>> get_chanid_to_lruaddrs_map();
	std::map<uint64_t, std::string> get_tmats_chanid_to_source_map();
	std::map<uint64_t, std::string> get_tmats_chanid_to_type_map();

	//void write_metadata_to_parquet(std::filesystem::path parquet_dir);
	void write_metadata_to_yaml(std::filesystem::path parquet_dir);
	//bool read_parquet_metadata(const std::string& parquet_dir);
	bool read_yaml_metadata(const std::string& parquet_dir);
};

template <typename Key, typename Val>
void ParserMetadata::emit_simple_map(YAML::Emitter& e, 
	const std::map<Key, Val>& input_map, const std::string& key)
{
	e << YAML::BeginMap;
	e << YAML::Key << key;
	e << YAML::Value << input_map;
	e << YAML::EndMap;
	e << YAML::Newline;
}

template<typename Key, typename Val>
void ParserMetadata::emit_compound_map(YAML::Emitter& e, 
	const std::map<Key, std::set<Val>>& input_map, const std::string& key)
{
	e << YAML::BeginMap;
	e << YAML::Key << key;
	e << YAML::Value << YAML::BeginMap;
	typename std::map<Key, std::set<Val>>::const_iterator it;
	for (it = input_map.cbegin(); it != input_map.cend(); ++it)
	{
		std::vector<Val> intermediate(it->second.begin(), it->second.end());
		e << YAML::Key << it->first << YAML::Value << YAML::Flow << intermediate;
	}
	e << YAML::EndMap;
	e << YAML::EndMap;
	e << YAML::Newline;
}

#endif
