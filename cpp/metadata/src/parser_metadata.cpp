#include "parser_metadata.h"

ParserMetadata::ParserMetadata() : chanid_to_lruaddrs_metadata_string_(""),
	output_path_(""), tmats_chanid_to_source_metadata_string_(""),
	parse_text_(), chanid_to_lruaddrs_map_key_("chanid_to_lru_addrs"), 
	tmats_chanid_to_source_map_key_("tmats_chanid_to_source"),
	tmats_chanid_to_type_map_key_("tmats_chanid_to_type"),
	video_chanid_to_min_timestamp_map_key_("chanid_to_first_timestamp") 
{}

void ParserMetadata::create_chanid_to_lruaddrs_metadata_strings(const std::map<uint32_t, std::set<uint16_t>>& map1,
	const std::map<uint32_t, std::set<uint16_t>>& map2)
{
	// TODO: make the string creation algorithm a template function
	std::string curr_metadata = "";
	char temp[10];
	std::map<uint32_t, std::set<uint16_t>>::const_iterator temp_it;
	int ind = 0;
	int total_ind = 0;
	for (std::map<uint32_t, std::set<uint16_t>>::const_iterator it = map1.begin(); it != map1.end(); ++it)
	{
		std::set<uint16_t> combined_set(it->second);
		temp_it = map2.find(it->first);
		if (temp_it != map2.end())
			combined_set.insert(temp_it->second.begin(), temp_it->second.end());

		// Build up the internal map.
		std::set<uint64_t> ui64set(combined_set.begin(), combined_set.end());
		chanid_to_lruaddrs_map_[uint64_t(it->first)] = ui64set;

		sprintf(temp, "%u:", it->first);
		curr_metadata += temp;
		ind = 0;
		for (std::set<uint16_t>::iterator it2 = combined_set.begin(); it2 != combined_set.end(); ++it2)
		{
			if(ind == 0)
				sprintf(temp, "%hu", *it2);
			else
				sprintf(temp, ",%hu", *it2);
			curr_metadata += temp;
			ind++;
		}

		//printf("Current metadata string: %s\n", curr_metadata.c_str());
		if (total_ind == 0)
			chanid_to_lruaddrs_metadata_string_ = curr_metadata;
		else
			chanid_to_lruaddrs_metadata_string_ += curr_metadata;

		curr_metadata += ";";
	}
#ifdef DEBUG
#if DEBUG > 1
	print_chanid_to_lruaddrs_map();
#endif
#endif

}

void ParserMetadata::create_tmats_channel_source_metadata_string(
	const std::map<uint32_t, std::string>& tmats_map)
{
	std::string curr_metadata = "";
	char temp[100];
	std::map<uint32_t, std::string>::const_iterator temp_it;
	int ind = 0;
	int total_ind = 0;
	for (std::map<uint32_t, std::string>::const_iterator it = tmats_map.begin(); it != tmats_map.end(); ++it)
	{
		if(ind == 0)
			sprintf(temp, "%u:%s", it->first, it->second.c_str());
		else
			sprintf(temp, ";%u:%s", it->first, it->second.c_str());
		curr_metadata += temp;
		ind++;
		
		tmats_chanid_to_source_map_[uint64_t(it->first)] = it->second;

		//printf("Current metadata string: %s\n", curr_metadata.c_str());
	}
	tmats_chanid_to_source_metadata_string_ = curr_metadata;
#ifdef DEBUG
#if DEBUG > 1
	print_tmats_chanid_to_source_map();
#endif
#endif
}

void ParserMetadata::create_tmats_channel_type_metadata_string(
	const std::map<uint32_t, std::string>& tmats_map)
{
	std::string curr_metadata = "";
	char temp[100];
	std::map<uint32_t, std::string>::const_iterator temp_it;
	int ind = 0;
	int total_ind = 0;
	for (std::map<uint32_t, std::string>::const_iterator it = tmats_map.begin(); it != tmats_map.end(); ++it)
	{
		if (ind == 0)
			sprintf(temp, "%u:%s", it->first, it->second.c_str());
		else
			sprintf(temp, ";%u:%s", it->first, it->second.c_str());
		curr_metadata += temp;
		ind++;

		tmats_chanid_to_type_map_[uint64_t(it->first)] = it->second;

		//printf("Current metadata string: %s\n", curr_metadata.c_str());
	}
	tmats_chanid_to_type_metadata_string_ = curr_metadata;
#ifdef DEBUG
#if DEBUG > 1
	print_tmats_chanid_to_type_map();
#endif
#endif
}

std::string ParserMetadata::get_chanid_to_lruaddrs_metadata_string()
{
	return chanid_to_lruaddrs_metadata_string_;
}

void ParserMetadata::set_chanid_to_lruaddrs_metadata_string(std::string& md_str)
{
	chanid_to_lruaddrs_metadata_string_ = md_str;
}

std::map<uint64_t, std::set<uint64_t>> ParserMetadata::get_chanid_to_lruaddrs_map()
{
	return chanid_to_lruaddrs_map_;
}

std::map<uint64_t, std::string> ParserMetadata::get_tmats_chanid_to_source_map()
{
	return tmats_chanid_to_source_map_;
}

std::map<uint64_t, std::string> ParserMetadata::get_tmats_chanid_to_type_map()
{
	return tmats_chanid_to_type_map_;
}

void ParserMetadata::recover_chanid_to_lruaddrs_map()
{
	// Split the string by semicolons.
	std::vector<std::string> semicolon_split = parse_text_.Split(chanid_to_lruaddrs_metadata_string_, ';');

	// Iterate over the substrings between semicolons.
	uint32_t chanid = 0;
	std::vector < std::string>::iterator it2;
	for (std::vector<std::string>::iterator it = semicolon_split.begin();
		it != semicolon_split.end(); ++it)
	{
		// For each substring, split on the colon and
		// cast the value before the colon a uint32_t and
		// split the second part by commas.
		std::vector<std::string> colon_split = parse_text_.Split(*it, ':');
		chanid = std::stol(colon_split[0].c_str());
		std::vector < std::string> comma_split = parse_text_.Split(colon_split[1], ',');

		// Iterate over the strings in the comma_split vector, 
		// cast them to integers, and append to the set.
		std::set<uint64_t> lruaddrs;
		for (it2 = comma_split.begin(); it2 != comma_split.end(); ++it2)
		{
			lruaddrs.insert(std::stol(*it2));
		}

		// Add the mapped entry of channel ID to LRU addresses set.
		chanid_to_lruaddrs_map_[chanid] = lruaddrs;
	}
}

void ParserMetadata::recover_tmats_chanid_to_source_map()
{
	// Split the original string by semicolons.
	std::vector<std::string> semicolon_split = parse_text_.Split(tmats_chanid_to_source_metadata_string_, ';');

	// Loop over the substrings.
	for (std::vector<std::string>::iterator it = semicolon_split.begin();
		it != semicolon_split.end(); ++it)
	{
		// Split each substring by colon, then cast the first
		// string to the channel ID and the second string is the 
		// source type.
		std::vector<std::string> colon_split = parse_text_.Split(*it, ':');
		tmats_chanid_to_source_map_[std::stol(colon_split[0])] = colon_split[1];
	}
}

void ParserMetadata::recover_tmats_chanid_to_type_map()
{
	// Split the original string by semicolons.
	std::vector<std::string> semicolon_split = parse_text_.Split(tmats_chanid_to_type_metadata_string_, ';');

	// Loop over the substrings.
	for (std::vector<std::string>::iterator it = semicolon_split.begin();
		it != semicolon_split.end(); ++it)
	{
		// Split each substring by colon, then cast the first
		// string to the channel ID and the second string is the 
		// source type.
		std::vector<std::string> colon_split = parse_text_.Split(*it, ':');
		tmats_chanid_to_type_map_[std::stol(colon_split[0])] = colon_split[1];
	}
}

std::vector<size_t> ParserMetadata::positions_of(std::string str, std::string search_str)
{
	std::vector<size_t> pos;
	size_t currpos = 0;
	size_t foundpos = 0;
	size_t search_str_len = search_str.size();
	while ((foundpos = str.find(search_str, currpos)) != std::string::npos)
	{
		//printf("pushing back %zu\n", foundpos);
		pos.push_back(foundpos);
		currpos = foundpos + search_str_len;
	}
	return pos;
}

void ParserMetadata::print_chanid_to_lruaddrs_map()
{
	if (chanid_to_lruaddrs_map_.size() == 0)
	{
		printf("\nTotal chanid to lru addrs metadata string:\n%s\n",
			chanid_to_lruaddrs_metadata_string_.c_str());
	}
	else
	{
		printf("\nChannel ID to LRU addrs map:");
		std::map<uint64_t, std::set<uint64_t>>::iterator it;
		std::set<uint64_t>::iterator it2;
		int ind = 0;
		for (it = chanid_to_lruaddrs_map_.begin(); it != chanid_to_lruaddrs_map_.end(); ++it)
		{
			ind = 0;
			printf("\n%02u: ", it->first);
			for (it2 = it->second.begin(); it2 != it->second.end(); ++it2)
			{
				if (ind == 0)
					printf("%02hu", *it2);
				else
					printf(", %02hu", *it2);
				ind++;
			}
		}
		printf("\n");	
	}
}

void ParserMetadata::print_tmats_chanid_to_source_map()
{
	if (tmats_chanid_to_source_map_.size() == 0)
	{
		printf("\nTotal channel id --> source metadata string:\n%s\n",
			tmats_chanid_to_source_metadata_string_.c_str());
	}
	else
	{
		printf("\nTMATS Channel ID to source map:\n");
		std::map<uint64_t, std::string>::iterator it;
		for (it = tmats_chanid_to_source_map_.begin(); it != tmats_chanid_to_source_map_.end(); ++it)
		{
			printf("%02u: %s\n", it->first, it->second.c_str());
		}
		printf("\n");
	}
}

void ParserMetadata::print_tmats_chanid_to_type_map()
{
	if (tmats_chanid_to_type_map_.size() == 0)
	{
		printf("\nTotal channel id --> type metadata string:\n%s\n",
			tmats_chanid_to_type_metadata_string_.c_str());
	}
	else
	{
		printf("\nTMATS Channel ID to type map:\n");
		std::map<uint64_t, std::string>::iterator it;
		for (it = tmats_chanid_to_type_map_.begin(); it != tmats_chanid_to_type_map_.end(); ++it)
		{
			printf("%02u: %s\n", it->first, it->second.c_str());
		}
		printf("\n");
	}
}

/*
void ParserMetadata::write_metadata_to_parquet(std::filesystem::path parquet_dir)
{
	create_output_parquet_path(parquet_dir);

	// Create instance of ParquetMilStd1553F1 for writing Parquet file
	// with schema and table but zero rows.
	ParquetMilStd1553F1 db(output_path_, 999, true);

	// Add channel ID to LRU addresses string as a key val pair.
	std::unordered_map<std::string, std::string> keyvals;
	keyvals["chanid_to_lru_addrs"] = chanid_to_lruaddrs_metadata_string_;
	keyvals["tmats_chanid_to_source"] = tmats_chanid_to_source_metadata_string_;
	keyvals["tmats_chanid_to_type"] = tmats_chanid_to_type_metadata_string_;

	// Add the TMATS key val pairs.
	// TODO

	// Append the metadata.
	db.SetMetadata(keyvals);

	// Write the file to disk and close. Note: file is 
	// closed when db goes out of scope.
	db.WriteColumns(0);
}*/


void ParserMetadata::Write1553metadataToYaml(std::filesystem::path parquet_dir)
{
	create_output_yaml_path(parquet_dir);

	YAML::Emitter emitter;

	/*recover_chanid_to_lruaddrs_map();
	recover_tmats_chanid_to_source_map();
	recover_tmats_chanid_to_type_map();*/

	emitter << YAML::BeginDoc;
	emit_compound_map(emitter, chanid_to_lruaddrs_map_, chanid_to_lruaddrs_map_key_);
	emit_simple_map(emitter, tmats_chanid_to_source_map_, tmats_chanid_to_source_map_key_);
	emit_simple_map(emitter, tmats_chanid_to_type_map_, tmats_chanid_to_type_map_key_);
	emitter << YAML::EndDoc;

	std::ofstream fout(output_path_.c_str());
	fout << emitter.c_str();
	fout.close();
}

void ParserMetadata::WriteVideoMetadataToYaml(std::filesystem::path parquet_dir,
	const std::map<uint16_t, uint64_t>& chanid_to_first_ts_map)
{
	create_output_yaml_path(parquet_dir);

	YAML::Emitter emitter;
	emitter << YAML::BeginDoc;
	emit_simple_map(emitter, chanid_to_first_ts_map, video_chanid_to_min_timestamp_map_key_);
	emitter << YAML::EndDoc;

	std::ofstream fout(output_path_.c_str());
	fout << emitter.c_str();
	fout.close();
}

void ParserMetadata::create_output_parquet_path(std::filesystem::path parquet_dir)
{
	std::filesystem::path outpath = parquet_dir / parquet_dir.stem();
	outpath += std::filesystem::path("__metadata.parquet");
	output_path_ = outpath.string();
	printf("Metadata output path: %s\n", output_path_.c_str());
}

void ParserMetadata::create_output_yaml_path(std::filesystem::path parquet_dir)
{
	// Leading underscore to avoid Spark detection.
	std::filesystem::path outpath = parquet_dir / std::filesystem::path("_metadata.yaml");
	output_path_ = outpath.string();
	printf("Metadata output path: %s\n", output_path_.c_str());
}

/*
bool ParserMetadata::read_parquet_metadata(const std::string& parquet_dir)
{
	std::filesystem::path _parquet_dir(parquet_dir);
	create_output_parquet_path(_parquet_dir);

	// Open parquet file for read using ParquetContext and get map 
	// of metadata.
	ParquetContext db;
	db.OpenForRead(output_path_, false);
	std::unordered_map<std::string, std::string> kvmd = db.GetMetadata();
	printf("Metadata map size: %zu\n", kvmd.size());

	// If there are no metadata, return false.
	if (kvmd.size() == 0)
		return false;

	// Set the metadata string member vars.
	std::unordered_map<std::string, std::string>::iterator it;

	// Channel ID to LRU addresses is required--return false if 
	// not present.
	if ((it = kvmd.find("chanid_to_lru_addrs")) != kvmd.end())
	{
		chanid_to_lruaddrs_metadata_string_ = kvmd["chanid_to_lru_addrs"];
		recover_chanid_to_lruaddrs_map();
		print_chanid_to_lruaddrs_map();
	}
	else
	{
		printf("chanid_to_lru_addrs metadata key not present!\n");
		return false;
	}

	if ((it = kvmd.find("tmats_chanid_to_source")) != kvmd.end())
	{
		tmats_chanid_to_source_metadata_string_ = kvmd["tmats_chanid_to_source"];
		recover_tmats_chanid_to_source_map();
		print_tmats_chanid_to_source_map();
	}
	if ((it = kvmd.find("tmats_chanid_to_type")) != kvmd.end())
	{
		tmats_chanid_to_type_metadata_string_ = kvmd["tmats_chanid_to_type"];
		recover_tmats_chanid_to_type_map();
		print_tmats_chanid_to_type_map();
	}
	return true;
}
*/

bool ParserMetadata::read_yaml_metadata(const std::string& parquet_dir)
{
	std::filesystem::path _parquet_dir(parquet_dir);
	create_output_yaml_path(_parquet_dir);

	YamlReader yr;
	if (!yr.LinkFile(output_path_))
	{
		printf("ParserMetadata::read_yaml_metadata(): Failed to link file %s\n", 
			output_path_.c_str());
		return false;
	}

	//std::map<uint64_t, std::set<uint64_t>> chanid_to_lruaddrs_map_;
	//std::map<uint64_t, std::string> tmats_chanid_to_source_map_;
	//std::map<uint64_t, std::string> tmats_chanid_to_type_map_;

	// Intermediate maps
	std::map<int, std::vector<int>> intmdt_chanid_to_lruaddrs_map;
	std::map<int, std::string> intmdt_tmats_chanid_to_source_map;
	std::map<int, std::string> intmdt_tmats_chanid_to_type_map;

	if (!yr.GetParams(chanid_to_lruaddrs_map_key_, intmdt_chanid_to_lruaddrs_map, true))
		return false;
	if (!(yr.GetParams(tmats_chanid_to_source_map_key_, intmdt_tmats_chanid_to_source_map, true)))
		return false;
	if (!(yr.GetParams(tmats_chanid_to_type_map_key_, intmdt_tmats_chanid_to_type_map, true)))
		return false;

	for (std::map<int, std::vector<int>>::const_iterator it = intmdt_chanid_to_lruaddrs_map.begin();
		it != intmdt_chanid_to_lruaddrs_map.end(); ++it)
	{
		chanid_to_lruaddrs_map_[uint64_t(it->first)] = std::set<uint64_t>(
			it->second.begin(), it->second.end());
	}
	print_chanid_to_lruaddrs_map();

	for (std::map<int, std::string>::const_iterator it = intmdt_tmats_chanid_to_source_map.begin();
		it != intmdt_tmats_chanid_to_source_map.end(); ++it)
	{
		tmats_chanid_to_source_map_[uint64_t(it->first)] = it->second;
	}
	print_tmats_chanid_to_source_map();

	for (std::map<int, std::string>::const_iterator it = intmdt_tmats_chanid_to_type_map.begin();
		it != intmdt_tmats_chanid_to_type_map.end(); ++it)
	{
		tmats_chanid_to_type_map_[uint64_t(it->first)] = it->second;
	}
	print_tmats_chanid_to_type_map();

	return true;
}
