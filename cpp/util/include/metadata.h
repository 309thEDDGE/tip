#ifndef METADATA_H
#define METADATA_H

#include <map>
#include <set>
#include <string>
#include <filesystem>
#include "yaml-cpp/yaml.h"
#include "iterable_tools.h"

class Metadata
{
private:
	// Tool for organization, printing and yaml emitting of iterables
	IterableTools iterable_tools_;

	// Yaml emitter for storing yaml output
	YAML::Emitter emitter_;

public:
	Metadata();

	// Get a Path object representative of the absolute
	// path of the yaml output file.
	//
	// Input:
	// - output_dir: Absolute path of output directory
	// - base_file_name: stem of metadata file name to be used
	// 
	// Returns:
	// - metadata_path: Absolute metadata file output path generated
	//		by this function
	//
	std::filesystem::path GetYamlMetadataPath(const std::filesystem::path& output_dir,
		const std::string& base_file_name);

	// Emit a single key value pair as a stand-alone Yaml map. 
	// See IterableTools::EmitKeyValuePair, the backbone
	// of this function.
	// 
	// Note: Data are not written with this function. Use ::GetMetadataString()
	// function to record all emitted records.
	template<typename Key, typename Val>
	void RecordSingleKeyValuePair(const Key& key, const Val& val);

	// Emit a named map, essentially a map of a single key to a
	// map of multiple key/value pairs. See IterableTools::EmitSimpleMap,
	// which is the backbone of this function.
	//
	// Note: Data are not written with this function. Use ::GetMetadataString()
	// function to record all emitted records.
	template<typename Key, typename Val>
	void RecordSimpleMap(const std::map<Key, Val>& input_map,
		const std::string& map_name);

	// Emit a named map of a single key to a map of key/vector values.
	// See IterableTools::EmitCompoundMapToVector,
	// which is the backbone of this function.
	//
	// Note: Data are not written with this function. Use ::GetMetadataString()
	// function to record all emitted records.
	template<typename Key, typename Val>
	void RecordCompoundMapToVector(const std::map<Key, std::vector<Val>>& input_map, 
		const std::string& map_name);

	// Emit a named map of a single key to a map of key/set values.
	// See IterableTools::EmitCompoundMapToSet,
	// which is the backbone of this function.
	//
	// Note: Data are not written with this function. Use ::GetMetadataString()
	// function to record all emitted records.
	template<typename Key, typename Val>
	void RecordCompoundMapToSet(const std::map<Key, std::set<Val>>& input_map, 
		const std::string& map_name);

	// Get a std::string containing the complete Yaml document
	// with each of the patterns accumulated using the Record* functions.
	std::string GetMetadataString();
};

template<typename Key, typename Val>
void Metadata::RecordSingleKeyValuePair(const Key& key, const Val& val)
{
	iterable_tools_.EmitKeyValuePair(emitter_, key, val);
}

template<typename Key, typename Val>
void Metadata::RecordSimpleMap(const std::map<Key, Val>& input_map,
	const std::string& map_name)
{
	iterable_tools_.EmitSimpleMap(emitter_, input_map, map_name);
}

template<typename Key, typename Val>
void Metadata::RecordCompoundMapToVector(const std::map<Key, std::vector<Val>>& input_map,
	const std::string& map_name)
{
	iterable_tools_.EmitCompoundMapToVector(emitter_, input_map, map_name);
}

template<typename Key, typename Val>
void Metadata::RecordCompoundMapToSet(const std::map<Key, std::set<Val>>& input_map,
	const std::string& map_name)
{
	iterable_tools_.EmitCompoundMapToSet(emitter_, input_map, map_name);
}

#endif