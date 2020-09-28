#include "parquet_translation_manager.h"
#include "translation_master.h"
//#include "config_manager.h"
//#include "platform.h"
#include "yaml_reader.h"
#include "metadata.h"
#include "file_reader.h"
#include "icd_data.h"
#include "bus_map.h"
#include "parquet_reader.h"
#include "translation_config_params.h"
#include <arrow/api.h>
#include <arrow/io/api.h>
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

bool PrepareICDAndBusMap(ICDData& icd_data, const std::string& input_path,
	const std::string& icd_path, bool stop_after_bus_map, bool prompt_user,
	std::map<std::string, std::string>& tmats_bus_name_corrections,
	bool use_tmats_busmap,
	std::map<uint64_t, std::string>& chanid_to_bus_name_map);
bool PrepareICD(ICDData& icd_data, const std::string& icd_path);
bool SynthesizeBusMap(ICDData& icd_data, const std::string& input_path, bool prompt_user,
	std::map<std::string, std::string>& tmats_bus_name_corrections,
	bool use_tmats_busmap,
	std::map<uint64_t, std::string>& chanid_to_bus_name_map);
bool MTTranslate(std::string input_path, uint8_t thread_count, bool select_msgs,
	std::vector<std::string> select_msg_names, ICDData& icd, const std::string& icd_path,
	std::map<uint64_t, std::string>& chanid_to_bus_name_map);
bool Translate(std::string input_path, bool select_msgs,
	std::vector<std::string> select_msg_names, ICDData& icd, const std::string& icd_path,
	std::map<uint64_t, std::string>& chanid_to_bus_name_map);
bool RecordMetadata(const std::filesystem::path translated_data_dir,
	const std::string& icd_path, std::map<uint64_t, std::string>& chanid_to_bus_name_map);

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

	printf("ICD path: %s\n", icd_path.c_str());
	printf("Input: %s\n", input_path.c_str());
	printf("Thread count: %hhu\n", thread_count);

	ICDData icd_data;
	std::map<uint64_t, std::string> chanid_to_bus_name_map;
	if (!PrepareICDAndBusMap(icd_data, input_path, icd_path, config.stop_after_bus_map_,
		config.prompt_user_, config.tmats_busname_corrections_, config.use_tmats_busmap_, 
		chanid_to_bus_name_map))
	{
		return 0;
	}
	
	// Start translation routine for multi-threaded use case (or single-threaded using the threading framework
	// if thread_count = 1 is specified).
	if (thread_count > 0)
	{
		MTTranslate(input_path, thread_count, !config.select_specific_messages_.empty(),
			config.select_specific_messages_, icd_data, icd_path, chanid_to_bus_name_map);
	}
	// Start the translation routine that doesn't use threading.
	else
	{
		Translate(input_path, !config.select_specific_messages_.empty(), 
			config.select_specific_messages_, icd_data, icd_path, chanid_to_bus_name_map);
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
	bool use_tmats_busmap, 
	std::map<uint64_t, std::string>& chanid_to_bus_name_map)
{
	// Read metadata from raw Parquet file. The important output from
	// this step are a map of TMATS data, bus name to channel ID,
	// and the required map of the Ch10-specific channel ID to set
	// of LRU addresses. The data for both of these maps are contained
	// within the object. Therefore the object will be passed around
	// to subsequent functions. If the channel ID to LRU address metadata
	// is not present, translation can't proceed.
	/*ParserMetadata parser_md;
	if (!parser_md.read_yaml_metadata(input_path))
	{
		printf("Failed to read yaml metadata!\n");
		return false;
	}*/

	// Read lines from ICD text file, ingest, and manipulate.
	auto start_time = std::chrono::high_resolution_clock::now();
	if (!PrepareICD(icd_data, icd_path))
	{
		return false;
	}
	auto stop_time = std::chrono::high_resolution_clock::now();
	printf("Duration: %zd sec\n", std::chrono::duration_cast<std::chrono::seconds>(stop_time - start_time).count());

	printf("\nStarting Bus Map\n");
	auto bus_map_start_time = std::chrono::high_resolution_clock::now();

	
	// Generate the bus map from metadata and possibly user
	// input.
	if (!SynthesizeBusMap(icd_data, input_path, prompt_user, 
		tmats_bus_name_corrections, use_tmats_busmap, 
		chanid_to_bus_name_map))
	{
		return false;
	}

	auto bus_map_end_time = std::chrono::high_resolution_clock::now();
	printf("Bus Map Duration: %zd sec\n", std::chrono::duration_cast<std::chrono::seconds>(bus_map_end_time - bus_map_start_time).count());

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

bool SynthesizeBusMap(ICDData& icd_data, const std::string& input_path, bool prompt_user,
	std::map<std::string, std::string>& tmats_bus_name_corrections,
	bool use_tmats_busmap, std::map<uint64_t,std::string>& chanid_to_bus_name_map)
{
	std::unordered_map<uint64_t, std::set<std::string>> message_key_to_busnames_map;
	icd_data.PrepareMessageKeyMap(message_key_to_busnames_map);

	// Create the metadata file path from the input raw parquet path.
	Metadata input_md;
	std::filesystem::path input_md_path = input_md.GetYamlMetadataPath(
		std::filesystem::path(input_path), "_metadata");

	// Use YamlReader to read the yaml metadata file.
	YamlReader yr;
	if (!yr.LinkFile(input_md_path.string()))
	{
		printf("Translate main: SynthesizeBusMap(): YamlReader failed to link file!\n");
		return false;
	}

	// Get the set of channel IDs from metadata -- REQUIRED.
	// Initially read in a map of uint64_t to vector of uint64_t 
	// and then get channel IDs from the map keys
	std::set<uint64_t> chanid_to_lruaddrs_set;
	std::map<uint64_t, std::vector<uint64_t>> chanid_to_lruaddrs_vec_map;
	if (!yr.GetParams("chanid_to_lru_addrs", chanid_to_lruaddrs_vec_map, true))
	{
		printf("Translate main: SynthesizeBusMap(): Failed to get chanid_to_lru_addrs"
			" map from metadata!\n");
		return false;
	}

	if (chanid_to_lruaddrs_vec_map.size() == 0)
		return false;

	for (std::map<uint64_t, std::vector<uint64_t>>::const_iterator it =
		chanid_to_lruaddrs_vec_map.begin(); it != chanid_to_lruaddrs_vec_map.end();
		++it)
	{
		chanid_to_lruaddrs_set.insert(it->first);
	}

	// Get the map of TMATS channel ID to source -- NOT REQUIRED.
	std::map<uint64_t, std::string> tmats_chanid_to_source_map;
	yr.GetParams("tmats_chanid_to_source", tmats_chanid_to_source_map, false);

	//// Get the map of TMATS channel ID to type -- NOT REQUIRED.
	std::map<uint64_t, std::string> tmats_chanid_to_type_map;
	yr.GetParams("tmats_chanid_to_type", tmats_chanid_to_type_map, false);

	// Initialize the maps necessary to synthesize the channel ID to bus name map.
	BusMap bm;
	bm.InitializeMaps(&message_key_to_busnames_map, chanid_to_lruaddrs_set,
		tmats_chanid_to_source_map, tmats_bus_name_corrections);

	ParquetReader pr;
	pr.SetManualRowgroupIncrementMode();
	if (!pr.SetPQPath(input_path))
	{
		printf("Non Existant Path %s\n",input_path.c_str());
		return false;
	};

	std::vector<uint64_t> transmit_cmds;
	std::vector<uint64_t> recieve_cmds;
	std::vector<uint64_t> channel_ids;

	int transmit_cmd_column = pr.GetColumnNumberFromName("txcommwrd");
	int recieve_cmd_column = pr.GetColumnNumberFromName("rxcommwrd");
	int channel_id_column = pr.GetColumnNumberFromName("channelid");

	// if any of the essential columns don't exist for busmapping return
	if (transmit_cmd_column == -1)
	{
		printf("txcommwrd doesn't exist in parquet table!\n");
		return false;
	}
	if (recieve_cmd_column == -1)
	{
		printf("rxcommwrd doesn't exist in parquet table!\n");
		return false;
	}
	if (channel_id_column == -1)
	{
		printf("channelid doesn't exist in parquet table!\n");
		return false;
	}
	
	// Loop over the parquet file row group by row group and submit
	// entries for votes to the bus map tool
	std::set<bool> status;
	int size = 0;
	while (status.count(false) == 0)
	{
		status.insert(pr.GetNextRG<uint64_t, arrow::NumericArray<arrow::Int32Type>>(transmit_cmd_column, transmit_cmds, size));
		status.insert(pr.GetNextRG<uint64_t, arrow::NumericArray<arrow::Int32Type>>(recieve_cmd_column, recieve_cmds, size));
		status.insert(pr.GetNextRG<uint64_t, arrow::NumericArray<arrow::Int32Type>>(channel_id_column, channel_ids, size));
		if(!bm.SubmitMessages(transmit_cmds, recieve_cmds, channel_ids, size))
			return false;
		pr.IncrementRG();
	}


	// Fill the channel ID to bus name map.
	// Note: will also need to pass prompt_user in future vresion of this function.
	if (!bm.Finalize(chanid_to_bus_name_map, use_tmats_busmap, 
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
	std::vector<std::string> select_msg_names, ICDData& icd, const std::string& icd_path,
	std::map<uint64_t, std::string>& chanid_to_bus_name_map)
{
	TranslationMaster tm(input_path, thread_count, select_msgs,
		select_msg_names, icd);

	RecordMetadata(tm.GetTranslatedDataDirectory(), icd_path, chanid_to_bus_name_map);

	auto start_time = std::chrono::high_resolution_clock::now();
	uint8_t ret_val = tm.translate();
	if (ret_val != 0)
	{
		printf("Translation error!\n");
		return false;
	}
	auto stop_time = std::chrono::high_resolution_clock::now();
	printf("Duration: %zd sec\n", std::chrono::duration_cast<std::chrono::seconds>(
		stop_time - start_time).count());
	return true;
}

bool Translate(std::string input_path, bool select_msgs,
	std::vector<std::string> select_msg_names, ICDData& icd, const std::string& icd_path,
	std::map<uint64_t, std::string>& chanid_to_bus_name_map)
{
	ParquetTranslationManager ptm(input_path, icd);
	ptm.set_select_msgs_list(select_msgs, select_msg_names);

	RecordMetadata(ptm.GetTranslatedDataDirectory(), icd_path, chanid_to_bus_name_map);

	auto start_time = std::chrono::high_resolution_clock::now();
	ptm.translate();
	if (ptm.get_status() < 0)
	{
		printf("Translation error\n");
		return false;
	}
	auto stop_time = std::chrono::high_resolution_clock::now();
	printf("Duration: %zd sec\n", std::chrono::duration_cast<std::chrono::seconds>(
		stop_time - start_time).count());
	return true;
}

bool RecordMetadata(const std::filesystem::path translated_data_dir,
	const std::string& icd_path, std::map<uint64_t, std::string>& chanid_to_bus_name_map)
{
	// Use Metadata class to create the output metadata file path.
	Metadata md;
	std::filesystem::path md_path = md.GetYamlMetadataPath(translated_data_dir,
		"_metadata");

	// Record the final bus map used for translation.
	md.RecordSimpleMap(chanid_to_bus_name_map, "chanid_to_bus_name_map");

	// Record the ICD path.
	md.RecordSingleKeyValuePair("icd_path", icd_path);

	// Get a string containing the complete metadata output and
	// and write it to the yaml file.
	std::ofstream stream_translation_metadata(md_path.string(),
		std::ofstream::out | std::ofstream::trunc);
	if (!(stream_translation_metadata.good() && stream_translation_metadata.is_open()))
	{
		printf("RecordMetadata(): Failed to open metadata file for writing, %s\n",
			md_path.string().c_str());
		return false;
	}
	stream_translation_metadata << md.GetMetadataString();
	stream_translation_metadata.close();
	return true;
}
