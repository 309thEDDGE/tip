#include "translator_helper_funcs.h"

bool GetArguments(int argc, char* argv[], ManagedPath& input_path,
	ManagedPath& icd_path)
{
	if (argc < 3)
	{
		printf("Args not present\n");
		return false;
	}

	input_path = ManagedPath(std::string(argv[1])); // path to parquet "file" (or .parquet directory)
	icd_path = ManagedPath(std::string(argv[2]));

	return true;
}

bool InitializeConfig(std::string tip_root_path, TranslationConfigParams& tcp)
{
	ManagedPath config_path;
	if (tip_root_path == "")
	{
		config_path = config_path.parent_path() / "conf" / "translate_conf.yaml";
	}
	else
	{
		config_path = ManagedPath(tip_root_path);
		config_path = config_path / "conf" / "translate_conf.yaml";
	}

	if (!tcp.Initialize(config_path.string()))
		return false;

	return true;
}

bool PrepareICDAndBusMap(DTS1553& dts1553, const ManagedPath& input_path,
	const ManagedPath& dts_path, bool stop_after_bus_map, bool prompt_user,
	uint64_t vote_threshold, bool vote_method_checks_tmats,
	std::vector<std::string> bus_exclusions,
	std::map<std::string, std::string>& tmats_bus_name_corrections,
	bool use_tmats_busmap, std::map<uint64_t, std::string>& chanid_to_bus_name_map,
	std::set<uint64_t>& excluded_channel_ids)
{

	// Read lines from ICD text file, ingest, and manipulate.
	auto start_time = std::chrono::high_resolution_clock::now();
	FileReader fr;
	if (fr.ReadFile(dts_path.string()) == 1)
	{
		printf("Failed to read ICD: %s\n", dts_path.RawString().c_str());
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
		vote_method_checks_tmats, bus_exclusions, tmats_bus_name_corrections, use_tmats_busmap,
		chanid_to_bus_name_map, excluded_channel_ids))
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

bool SynthesizeBusMap(DTS1553& dts1553, const ManagedPath& input_path, bool prompt_user,
	uint64_t vote_threshold, bool vote_method_checks_tmats,
	std::vector<std::string> bus_exclusions,
	std::map<std::string, std::string>& tmats_bus_name_corrections,
	bool use_tmats_busmap, std::map<uint64_t, std::string>& chanid_to_bus_name_map,
	std::set<uint64_t>& excluded_channel_ids)
{
	std::unordered_map<uint64_t, std::set<std::string>> message_key_to_busnames_map;

	dts1553.ICDDataPtr()->PrepareMessageKeyMap(message_key_to_busnames_map,
		dts1553.GetSupplBusNameToMessageKeyMap());

	// Create the metadata file path from the input raw parquet path.
	Metadata input_md;
	ManagedPath input_md_path = input_md.GetYamlMetadataPath(
		input_path, "_metadata");

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
	std::map<uint64_t, std::vector<std::vector<uint16_t>>> chanid_to_comm_words_map;
	if (!yr.GetParams("chanid_to_comm_words", chanid_to_comm_words_map, true))
	{
		printf("Translate main: SynthesizeBusMap(): Failed to get chanid_to_comm_words"
			" map from metadata!\n");
		return false;
	}

	for (std::map<uint64_t, std::vector<std::vector<uint16_t>>>::const_iterator it =
		chanid_to_comm_words_map.begin(); it != chanid_to_comm_words_map.end();
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

	// convert vector to set
	IterableTools it;
	std::set<std::string> bus_exclusions_set = it.VecToSet(bus_exclusions);

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
		vote_method_checks_tmats,
		bus_exclusions_set,
		tmats_chanid_to_source_map,
		tmats_bus_name_corrections);

	ParquetReader pr;
	pr.SetManualRowgroupIncrementMode();
	if (!pr.SetPQPath(input_path))
	{
		printf("Non Existent Path %s\n", input_path.RawString().c_str());
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

	// Submit votes for each transmit/recieve command word from each 
	// channel ID found in metadata from parameter chanid_to_comm_words 
	for (std::map<uint64_t, std::vector<std::vector<uint16_t>>>::const_iterator it =
		chanid_to_comm_words_map.begin(); it != chanid_to_comm_words_map.end();
		++it)
	{
		for (int i = 0; i < it->second.size(); i++)
		{
			if (it->second[i].size() == 2)
			{
				bm.SubmitMessage(it->second[i][0], it->second[i][1], it->first);
			}
		}

	}

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

	// If a channel ID from chanid_to_lruaddrs_set is not in 
	// chanid_to_bus_name_map add it as an excluded channel 
	// id and save to metadata later
	for (auto channel_id : chanid_to_lruaddrs_set)
	{
		if (chanid_to_bus_name_map.count(channel_id) == 0)
			excluded_channel_ids.insert(channel_id);
	}

	// Reverse the map that is populated in the previous step.
	std::map<std::string, std::set<uint64_t>> bus_name_to_chanid_map =
		it.ReverseMapSet(chanid_to_bus_name_set_map);

	// Correct and update the lookup table in ICDData with the new map.
	if (!dts1553.ICDDataPtr()->ReplaceBusNameWithChannelIDInLookup(bus_name_to_chanid_map))
	{
		printf("Failed to update Lookup map with bus_name_to_chanid_map!\n");
		return false;
	}

	return true;
}

bool MTTranslate(TranslationConfigParams config, const ManagedPath& input_path,
	ICDData icd, const ManagedPath& dts_path,
	std::map<uint64_t, std::string>& chanid_to_bus_name_map,
	const std::set<uint64_t>& excluded_channel_ids)
{
	bool select_msgs = !config.select_specific_messages_.empty();

	TranslationMaster tm(input_path, config.translate_thread_count_, select_msgs,
		config.select_specific_messages_, icd);

	auto start_time = std::chrono::high_resolution_clock::now();
	uint8_t ret_val = tm.translate();
	if (ret_val != 0)
	{
		printf("Translation error!\n");
		return false;
	}

	RecordMetadata(config, tm.GetTranslatedDataDirectory(), dts_path, chanid_to_bus_name_map,
		excluded_channel_ids, input_path, tm.GetTranslatedMessages());

	auto stop_time = std::chrono::high_resolution_clock::now();
	printf("Duration: %zd sec\n", std::chrono::duration_cast<std::chrono::seconds>(
		stop_time - start_time).count());
	return true;
}

bool Translate(TranslationConfigParams config, const ManagedPath& input_path,
	ICDData icd, const ManagedPath& dts_path,
	std::map<uint64_t, std::string>& chanid_to_bus_name_map,
	const std::set<uint64_t>& excluded_channel_ids)
{
	bool select_msgs = !config.select_specific_messages_.empty();

	ParquetTranslationManager ptm(input_path, icd);
	ptm.set_select_msgs_list(select_msgs, config.select_specific_messages_);

	auto start_time = std::chrono::high_resolution_clock::now();
	ptm.translate();
	if (ptm.get_status() < 0)
	{
		printf("Translation error\n");
		return false;
	}

	RecordMetadata(config, ptm.GetTranslatedDataDirectory(), dts_path, chanid_to_bus_name_map,
		excluded_channel_ids, input_path, ptm.GetTranslatedMessages());

	auto stop_time = std::chrono::high_resolution_clock::now();
	printf("Duration: %zd sec\n", std::chrono::duration_cast<std::chrono::seconds>(
		stop_time - start_time).count());
	return true;
}

bool RecordMetadata(TranslationConfigParams config, const ManagedPath& translated_data_dir,
	const ManagedPath& dts_path, std::map<uint64_t, std::string>& chanid_to_bus_name_map,
	const std::set<uint64_t>& excluded_channel_ids, const ManagedPath& input_path,
	const std::set<std::string>& translated_messages)
{
	// Use Metadata class to create the output metadata file path.
	Metadata md;
	ManagedPath md_path = md.GetYamlMetadataPath(translated_data_dir,
		"_metadata");

	// Record config parameters.
	md.RecordSingleKeyValuePair("translate_thread_count", config.translate_thread_count_);
	md.RecordSingleKeyValuePair("use_tmats_busmap", config.use_tmats_busmap_);
	md.RecordSingleKeyValuePair("tmats_busname_corrections", config.tmats_busname_corrections_);
	md.RecordSingleKeyValuePair("prompt_user", config.prompt_user_);
	md.RecordSingleKeyValuePair("vote_threshold", config.vote_threshold_);
	md.RecordSingleKeyValuePair("vote_method_checks_tmats", config.vote_method_checks_tmats_);
	md.RecordSingleKeyValuePair("bus_name_exclusions", config.bus_name_exclusions_);
	md.RecordSingleKeyValuePair("stop_after_bus_map", config.stop_after_bus_map_);
	md.RecordSingleKeyValuePair("select_specific_messages", config.select_specific_messages_);
	md.RecordSingleKeyValuePair("exit_after_table_creation", config.exit_after_table_creation_);

	// Record the ICD path.
	md.RecordSingleKeyValuePair("dts_path", dts_path.RawString());

	// Record parsed parquet file used to translate.
	md.RecordSingleKeyValuePair("1553_parquet_file_path", input_path.RawString());

	// Record the final bus map used for translation.
	md.RecordSimpleMap(chanid_to_bus_name_map, "chanid_to_bus_name_map");

	// Record the busmap status.
	md.RecordSingleKeyValuePair("excluded_channel_ids", excluded_channel_ids);

	// Record translated messages.
	md.RecordSingleKeyValuePair("translated_messages", translated_messages);



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