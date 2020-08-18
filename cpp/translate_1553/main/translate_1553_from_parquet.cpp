#include "parquet_translation_manager.h"
#include "translation_master.h"
//#include "config_manager.h"
//#include "platform.h"
#include "parser_metadata.h"
#include "file_reader.h"
#include "icd_data.h"
#include "bus_map.h"
#include "translation_config_params.h"
#include <filesystem>
#include <iostream>
#include <cstdlib>
#include <set>

// TODO: Create a class with all of the required translation routine
// data and parameters. Pass this class around instead of long lists
// of vars.


bool GetArguments(int argc, char* argv[], std::string& input_path,
	std::string& icd_path);
bool GetArguments(int argc, char* argv[], std::string& input_path,
	uint8_t& thread_count, std::string& icd_path);

//bool PrepareConfigFiles(ConfigManager& cm_parse, ConfigManager& cm_translate,
//	ConfigManager& cm_platform, ACPlatform& platform);
//bool GetConfigParams(ConfigManager& cm_parse, ConfigManager& cm_translate,
//	ConfigManager& cm_platform, std::string& comet_path,
//	bool& select_msgs,
//	std::vector<std::string>& select_msg_names,
//	std::map<std::string, std::string>& tmats_bus_name_corrections,
//	std::vector<std::vector<std::string>>& degenerate_messages,
//	bool& exit_after_table,
//	bool& stop_after_bus_map,
//	bool& prompt_user,
//	int& comet_debug,
//	int& map_confidence_level);

//bool GetParquetMetadata(const std::string& parquet_path);
bool PrepareICDAndBusMap(ICDData& icd_data, const std::string& input_path,
	const std::string& icd_path, bool stop_after_bus_map, bool prompt_user,
	std::map<std::string, std::string>& tmats_bus_name_corrections,
	int map_confidence_level, 
	std::map<std::string, std::set<uint64_t>> bus_name_to_lruaddrs_set_map);
bool PrepareICD(ICDData& icd_data, const std::string& icd_path);
bool SynthesizeBusMap(ICDData& icd_data, ParserMetadata& md, bool prompt_user,
	std::map<std::string, std::string>& tmats_bus_name_corrections,
	int map_confidence_level, std::map<std::string, std::set<uint64_t>> comet_busmap_replacement);
bool MTTranslate(std::string input_path, uint8_t thread_count, bool select_msgs,
	std::vector<std::string> select_msg_names, ICDData& icd);
bool Translate(std::string input_path, bool select_msgs,
	std::vector<std::string> select_msg_names, ICDData& icd);

int main(int argc, char* argv[])
{
	std::string input_path = "";
	uint8_t thread_count = 0;
	std::string icd_path = "";

	if (!GetArguments(argc, argv, input_path, icd_path))
		return 0;

	TranslationConfigParams config;
	std::filesystem::path file_path("../conf/translate_conf.yaml");
	if (!config.Initialize(file_path.string()))
		return 0;
	thread_count = config.translate_thread_count_;
	ICDData icd_data;
	if (!PrepareICDAndBusMap(icd_data, input_path, icd_path, config.stop_after_bus_map_,
		config.prompt_user_, config.tmats_busname_corrections_, config.bus_map_confidence_level_,
		config.comet_busmap_replacement_))
	{
		return 0;
	}
	printf("prepare icd and bus map\n");
	
	printf("ICD path: %s\n", icd_path.c_str());

	

	printf("Input: %s\n", input_path.c_str());
	printf("Thread count: %hhu\n", thread_count);

	// Start translation routine for multi-threaded use case (or single-threaded using the threading framework
	// if thread_count = 1 is specified).
	if (thread_count > 0)
	{
		MTTranslate(input_path, thread_count, !config.select_specific_messages_.empty(),
			config.select_specific_messages_, icd_data);
	}
	// Start the translation routine that doesn't use threading.
	else
	{
		Translate(input_path, !config.select_specific_messages_.empty(), config.select_specific_messages_, icd_data);
	}

	//system("pause");
	return 0;
}

bool GetArguments(int argc, char* argv[], std::string& input_path, 
	uint8_t& thread_count, std::string& icd_path)
{

	if (argc < 3)
	{
		printf("Args not present\n");
		return false;
	}

	input_path = argv[1]; // path to parquet "file" (could be .parquet directory)
	thread_count = atoi(argv[2]);

	return true;
}

bool GetArguments(int argc, char* argv[], std::string& input_path,
	std::string& icd_path)
{
	if (argc < 3)
	{
		printf("Args not present\n");
		return false;
	}

	input_path = argv[1]; // path to parquet "file" (could be .parquet directory)
	icd_path = argv[2];

	return true;
}

bool PrepareICDAndBusMap(ICDData& icd_data, const std::string& input_path, 
	const std::string& icd_path, bool stop_after_bus_map, bool prompt_user,
	std::map<std::string, std::string>& tmats_bus_name_corrections,
	int map_confidence_level, 
	std::map<std::string, std::set<uint64_t>> bus_name_to_lruaddrs_set_map)
{
	// Read metadata from raw Parquet file. The important output from
	// this step are a map of TMATS data, bus name to channel ID,
	// and the required map of the Ch10-specific channel ID to set
	// of LRU addresses. The data for both of these maps are contained
	// within the object. Therefore the object will be passed around
	// to subsequent functions. If the channel ID to LRU address metadata
	// is not present, translation can't proceed.
	ParserMetadata parser_md;
	if (!parser_md.read_yaml_metadata(input_path))
	{
		printf("Failed to read yaml metadata!\n");
		return false;
	}

	// Read lines from ICD text file, ingest, and manipulate.
	auto start_time = std::chrono::high_resolution_clock::now();
	if (!PrepareICD(icd_data, icd_path))
	{
		return false;
	}
	auto stop_time = std::chrono::high_resolution_clock::now();
	printf("Duration: %zd sec\n", std::chrono::duration_cast<std::chrono::seconds>(stop_time - start_time).count());

	// Generate the bus map from metadata and possibly user
	// input.
	if (!SynthesizeBusMap(icd_data, parser_md, prompt_user, 
		tmats_bus_name_corrections, map_confidence_level, 
		bus_name_to_lruaddrs_set_map))
	{
		return false;
	}

	// If the config file option stop_after_bus_map == true, 
	// exit the program.
	if (stop_after_bus_map)
	{
		printf("User-defined config parameter \"stop_after_bus_map\" set to true\n");
		return false;
	}
	return true;
}

bool PrepareICD(ICDData& icd_data, const std::string& icd_path)
{
	bool is_yaml_file = icd_data.IsYamlFile(icd_path);
	if (is_yaml_file)
		printf("Using ICD Yaml file\n");
	
	FileReader fr;
	if (fr.ReadFile(icd_path) == 1)
	{
		printf("Failed to read ICD: %s\n", icd_path.c_str());
		return false;
	}

	if (!icd_data.PrepareICDQuery(fr.GetLines(), is_yaml_file))
	{
		printf("Failed to ingest and organize ICD data\n");
		return false;
	}
	return true;
	//return false;
}

bool SynthesizeBusMap(ICDData& icd_data, ParserMetadata& md, bool prompt_user,
	std::map<std::string, std::string>& tmats_bus_name_corrections,
	int map_confidence_level, std::map<std::string, std::set<uint64_t>> comet_busmap_replacement)
{
	// If comet_busmap_replacement is given in the config file
	// use that for bus_name_to_lruaddrs_set_map instead of the ICD
	bool use_config_comet_busmap = !comet_busmap_replacement.empty();

	std::map<std::string, std::set<uint64_t>> bus_name_to_lruaddrs_set_map;
	if (use_config_comet_busmap)
	{
		bus_name_to_lruaddrs_set_map = comet_busmap_replacement;
	} else
	{
		// Get the map of bus name to set of LRU addresses from the ICD -- REQUIRED
		bus_name_to_lruaddrs_set_map = icd_data.GetBusNameToLRUAddrsMap();
		if (bus_name_to_lruaddrs_set_map.size() == 0)
			return false;
	}	

	// Get the map of channel ID to LRU address sets from metadata -- REQUIRED
	std::map<uint64_t, std::set<uint64_t>> chanid_to_lruaddrs_set_map =
		md.get_chanid_to_lruaddrs_map();
	if (chanid_to_lruaddrs_set_map.size() == 0)
		return false;

	// Get the map of TMATS channel ID to source -- NOT REQUIRED.
	std::map<uint64_t, std::string> tmats_chanid_to_source_map =
		md.get_tmats_chanid_to_source_map();

	// Get the map of TMATS channel ID to type -- NOT REQUIRED.
	std::map<uint64_t, std::string> tmats_chanid_to_type_map =
		md.get_tmats_chanid_to_type_map();

	// Initialize the maps necessary to synthesize the channel ID to bus name map.
	BusMap bm;
	bm.InitializeMaps(bus_name_to_lruaddrs_set_map, chanid_to_lruaddrs_set_map,
		tmats_chanid_to_source_map, tmats_bus_name_corrections);

	// Create the channel ID to bus name map.
	// Note: will also need to pass prompt_user in future vresion of this function.
	std::map<uint64_t, std::string> chanid_to_bus_name_map;
	if (!bm.PerformBusMapping(chanid_to_bus_name_map, map_confidence_level, 
		prompt_user))
	{
		printf("Bus mapping failed!\n");
		return false;
	}

	// Convert the chanid to bus name map to a map from 
	// int to set of strings.
	std::map<uint64_t, std::set<std::string>> chanid_to_bus_name_set_map;
	for (std::map<uint64_t, std::string>::iterator it = chanid_to_bus_name_map.begin();
		it != chanid_to_bus_name_map.end(); ++it)
	{
		std::set<std::string> bus_name_set;
		bus_name_set.insert(it->second);
		chanid_to_bus_name_set_map[it->first] = bus_name_set;
	}

	// Reverse the map that is populated in the previous step.
	IterableTools it;
	std::map<std::string, std::set<uint64_t>> bus_name_to_chanid_map =
		it.ReverseMapSet(chanid_to_bus_name_set_map);
	/*std::vector<std::string> cols = { "BusName", "chID" };
	it.PrintMapWithHeader_KeyToSet(bus_name_to_chanid_map, cols, "bus_name_to_chanid_map");*/

	// Correct and update the lookup table in ICDData with the new map.
	if (!icd_data.ReplaceBusNameWithChannelIDInLookup(bus_name_to_chanid_map))
	{
		printf("Failed to update Lookup map with bus_name_to_chanid_map!\n");
		return false;
	}

	return true;
}

bool MTTranslate(std::string input_path, uint8_t thread_count, bool select_msgs,
	std::vector<std::string> select_msg_names, ICDData& icd)
{
	auto start_time = std::chrono::high_resolution_clock::now();
	TranslationMaster tm(input_path, thread_count, select_msgs,
		select_msg_names, icd);

	//return true;
	uint8_t ret_val = tm.translate();
	if (ret_val != 0)
	{
		printf("Translation error!\n");
		return false;
	}
	auto stop_time = std::chrono::high_resolution_clock::now();
	printf("Duration: %zd sec\n", std::chrono::duration_cast<std::chrono::seconds>(stop_time - start_time).count());
	return true;
}

bool Translate(std::string input_path, bool select_msgs,
	std::vector<std::string> select_msg_names, ICDData& icd)
{
	ParquetTranslationManager ptm(input_path, icd);
	ptm.set_select_msgs_list(select_msgs, select_msg_names);
	ptm.translate();
	if (ptm.get_status() < 0)
	{
		printf("Translation error\n");
		return false;
	}
	return true;
}
