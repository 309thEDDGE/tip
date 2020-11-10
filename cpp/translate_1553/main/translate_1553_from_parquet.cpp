#include "parquet_translation_manager.h"
#include "translation_master.h"
//#include "config_manager.h"
//#include "platform.h"
#include "yaml_reader.h"
#include "metadata.h"
#include "file_reader.h"
#include "dts1553.h"
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

bool PrepareICDAndBusMap(DTS1553& dts1553, const std::string& input_path,
	const std::string& dts_path, bool stop_after_bus_map, bool prompt_user,
	uint64_t vote_threshold, std::map<std::string, std::string>& tmats_bus_name_corrections,
	bool use_tmats_busmap,
	std::map<uint64_t, std::string>& chanid_to_bus_name_map);
bool SynthesizeBusMap(DTS1553& dts1553, const std::string& input_path, bool prompt_user,
	uint64_t vote_threshold, std::map<std::string, std::string>& tmats_bus_name_corrections,
	bool use_tmats_busmap,
	std::map<uint64_t, std::string>& chanid_to_bus_name_map);
bool MTTranslate(std::string input_path, uint8_t thread_count, bool select_msgs,
	std::vector<std::string> select_msg_names, ICDData icd, const std::string& dts_path,
	std::map<uint64_t, std::string>& chanid_to_bus_name_map);
bool Translate(std::string input_path, bool select_msgs,
	std::vector<std::string> select_msg_names, ICDData icd, const std::string& dts_path,
	std::map<uint64_t, std::string>& chanid_to_bus_name_map);
bool RecordMetadata(const std::filesystem::path translated_data_dir,
	const std::string& dts_path, std::map<uint64_t, std::string>& chanid_to_bus_name_map);

int main(int argc, char* argv[])
{
	std::string input_path = "";
	uint8_t thread_count = 0;
	std::string dts_path = "";

	if (!GetArguments(argc, argv, input_path, dts_path))
		return 0;

	TranslationConfigParams config;
	std::filesystem::path file_path("../conf/translate_conf.yaml");
	if (!config.Initialize(file_path.string()))
		return 0;
	thread_count = config.translate_thread_count_;

	printf("DTS1553 path: %s\n", dts_path.c_str());
	printf("Input: %s\n", input_path.c_str());
	printf("Thread count: %hhu\n", thread_count);

	DTS1553 dts1553;
	std::map<uint64_t, std::string> chanid_to_bus_name_map;
	if (!PrepareICDAndBusMap(dts1553, input_path, dts_path, config.stop_after_bus_map_,
		config.prompt_user_, config.vote_threshold_, 
		config.tmats_busname_corrections_, config.use_tmats_busmap_, 
		chanid_to_bus_name_map))
	{
		return 0;
	}
	
	// Start translation routine for multi-threaded use case (or single-threaded using the threading framework
	// if thread_count = 1 is specified).
	if (thread_count > 0)
	{
		MTTranslate(input_path, thread_count, !config.select_specific_messages_.empty(),
			config.select_specific_messages_, dts1553.GetICDData(), dts_path, chanid_to_bus_name_map);
	}
	// Start the translation routine that doesn't use threading.
	else
	{
		Translate(input_path, !config.select_specific_messages_.empty(), 
			config.select_specific_messages_, dts1553.GetICDData(), dts_path, chanid_to_bus_name_map);
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

bool PrepareICDAndBusMap(DTS1553& dts1553, const std::string& input_path,
	const std::string& dts_path, bool stop_after_bus_map, bool prompt_user,
	uint64_t vote_threshold, std::map<std::string, std::string>& tmats_bus_name_corrections,
	bool use_tmats_busmap, 
	std::map<uint64_t, std::string>& chanid_to_bus_name_map)
{

	// Read lines from ICD text file, ingest, and manipulate.
	auto start_time = std::chrono::high_resolution_clock::now();
	FileReader fr;
	if (fr.ReadFile(dts_path) == 1)
	{
		printf("Failed to read ICD: %s\n", dts_path.c_str());
		return false;
	}
	if (!dts1553.IngestLines(dts_path, fr.GetLines()))
	{
		printf("Failed to ingest DTS1553 data\n");
		return false;
	}
	auto stop_time = std::chrono::high_resolution_clock::now();
	printf("DTS1553 ingest and message lookup table synthesis duration: %zd sec\n", 
		std::chrono::duration_cast<std::chrono::seconds>(stop_time - start_time).count());

	printf("\nStarting Bus Map\n");
	auto bus_map_start_time = std::chrono::high_resolution_clock::now();

	// Generate the bus map from metadata and possibly user
	// input.
	if (!SynthesizeBusMap(dts1553, input_path, prompt_user, vote_threshold,
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

bool SynthesizeBusMap(DTS1553& dts1553, const std::string& input_path, bool prompt_user,
	uint64_t vote_threshold, std::map<std::string, std::string>& tmats_bus_name_corrections,
	bool use_tmats_busmap, std::map<uint64_t,std::string>& chanid_to_bus_name_map)
{
	std::unordered_map<uint64_t, std::set<std::string>> message_key_to_busnames_map;

	dts1553.ICDDataPtr()->PrepareMessageKeyMap(message_key_to_busnames_map,
		dts1553.GetSupplBusNameToMessageKeyMap());

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

	// The mask will mask out word count/mode code from the transmit and 
	// recieve command word portion of the key.
	// The last 16 bits of the key represent the recieve command word
	// and the second to last 16 bits of the key represent the transmit command word.
	// The last five bits of the transmit and recieve command words represent
	// word count/mode code. The word count/mode code portion is masked out
	// because in the case where the command word is a mode code, the last five
	// bits of the command word are the mode code bits and no longer match
	// the command words found in the input icd file unless they are masked to zero.
	uint64_t mask = 0b1111111111111111111111111111111111111111111000001111111111100000;
	bm.InitializeMaps(&message_key_to_busnames_map, 
		chanid_to_lruaddrs_set, 
		mask,
		vote_threshold,
		tmats_chanid_to_source_map, 
		tmats_bus_name_corrections);

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

	if (!use_tmats_busmap)
	{
		// Loop over the parquet file row group by row group and submit
		// entries for votes to the bus map tool
		std::set<bool> status;
		int size = 0;
		int submission_count = 0;
		bool successful_map = false;
		std::map<uint64_t, std::string> temp_bus_map;
		bool first_loop = true;

		// Comet currently parses at least 3000 1553 packets from a chapter 10
		// to submit votes for bus mapping. For purposes of the method 
		// below, each 1553 packet is estimated to contain on average
		// 40 messages. The method requires that the final 
		// busmap not change after submitting 1500 packets (1500 * 40 messages).
		// The method also doesn't allow the busmap to be considered
		// a success until the second loop to ensure that at least
		// ~3000 packets were added to the vote system.
		int message_count_threshold = 1500 * 40;
		while (status.count(false) == 0)
		{
			status.insert(pr.GetNextRG<uint64_t, 
				arrow::NumericArray<arrow::Int32Type>>(transmit_cmd_column, transmit_cmds, size));

			status.insert(pr.GetNextRG<uint64_t, 
				arrow::NumericArray<arrow::Int32Type>>(recieve_cmd_column, recieve_cmds, size));

			status.insert(pr.GetNextRG<uint64_t,
				arrow::NumericArray<arrow::Int32Type>>(channel_id_column, channel_ids, size));

			if(!bm.SubmitMessages(transmit_cmds, recieve_cmds, channel_ids, size))
				return false;

			pr.IncrementRG();
			submission_count += size;

			if (submission_count >= message_count_threshold)
			{
				// Only perform bus mapping after the first loop
				// to ensure that at least ~3000 packets were
				// added to the vote. Each loop is ~3000/2 
				// packets.
				if (first_loop == false)
				{
					if (!bm.Finalize(temp_bus_map, false,
						false))
					{
						printf("Bus mapping failed!\n");
						return false;
					}

					// If the bus map did not change from the 
					// last bus map after at least message_count_threshold
					// messages were submitted, consider the bus map
					// a success and break out of the while loop.
					if (temp_bus_map == chanid_to_bus_name_map)
					{
						successful_map = true;
						break;
					}
				}

				submission_count = 0;
				chanid_to_bus_name_map = temp_bus_map;
				first_loop = false;
			}

		}

		// If the bus map was unsuccessful, clear any remaining
		// bus map information from the above while loop execution.
		if (!successful_map)
		{
			printf("\nVote map method failure due to small chapter 10 or "
				"zero channel ID mappings.\n");
			chanid_to_bus_name_map.clear();
		}

		// If prompt user == true, finalize again with prompt
		// user passed in to give the user a chance to edit
		// the bus map if it isn't complete
		if (prompt_user)
		{
			
			if (!bm.Finalize(chanid_to_bus_name_map, false,
				prompt_user))
			{
				printf("Bus mapping failed!\n");
				return false;
			}
		}

		// if chanid_to_bus_name_map is still empty after
		// prompt user was potentially called
		// return false, because nothing was mapped
		if (chanid_to_bus_name_map.empty())
		{
			printf("Bus mapping failed!\n");
			return false;
		}

	}

	// Use TMATS busmap
	else
	{
		if (!bm.Finalize(chanid_to_bus_name_map, use_tmats_busmap,
			prompt_user))
		{
			printf("Bus mapping failed!\n");
			return false;
		}
	}
	
	bm.Print();

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
	if (!dts1553.ICDDataPtr()->ReplaceBusNameWithChannelIDInLookup(bus_name_to_chanid_map))
	{
		printf("Failed to update Lookup map with bus_name_to_chanid_map!\n");
		return false;
	}

	return true;
}

bool MTTranslate(std::string input_path, uint8_t thread_count, bool select_msgs,
	std::vector<std::string> select_msg_names, ICDData icd, const std::string& dts_path,
	std::map<uint64_t, std::string>& chanid_to_bus_name_map)
{
	TranslationMaster tm(input_path, thread_count, select_msgs,
		select_msg_names, icd);

	RecordMetadata(tm.GetTranslatedDataDirectory(), dts_path, chanid_to_bus_name_map);

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
	std::vector<std::string> select_msg_names, ICDData icd, const std::string& dts_path,
	std::map<uint64_t, std::string>& chanid_to_bus_name_map)
{
	ParquetTranslationManager ptm(input_path, icd);
	ptm.set_select_msgs_list(select_msgs, select_msg_names);

	RecordMetadata(ptm.GetTranslatedDataDirectory(), dts_path, chanid_to_bus_name_map);

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
	const std::string& dts_path, std::map<uint64_t, std::string>& chanid_to_bus_name_map)
{
	// Use Metadata class to create the output metadata file path.
	Metadata md;
	std::filesystem::path md_path = md.GetYamlMetadataPath(translated_data_dir,
		"_metadata");

	// Record the final bus map used for translation.
	md.RecordSimpleMap(chanid_to_bus_name_map, "chanid_to_bus_name_map");

	// Record the ICD path.
	md.RecordSingleKeyValuePair("dts_path", dts_path);

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
