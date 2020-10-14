#include "bus_map.h"


void BusMap::AddLinesTo_BusnameLRUAddressMap(std::vector<std::string> term_mux_lines)
{
	for (int i = 0; i < term_mux_lines.size(); i++)
	{
		// If empty string, don't add to the map
		if (term_mux_lines[i].length() == 0)
			continue;
		
		// If the first character is a point character (comment line) don't add to map
		if (term_mux_lines[i][0] == '#')
			continue;

		// Split on '|'
		std::vector<std::string> split_return = parse_text_.Split(term_mux_lines[i],'|');
		
		// If there aren't three split elements continue
		if (split_return.size() != 3)
			continue;

		std::string lru_name = split_return[0];
		std::string bus_name = split_return[1];
		std::string lru_address = split_return[2];

		// If bus name or lru address are empty string don't add to map
		if (bus_name == "" || lru_address == "")
			continue;

		// Convert lru address from string to int
		int temp_int;

		// Check if the lru_address is convertible to an integer
		if (!parse_text_.ConvertInt(lru_address, temp_int))
			continue;

		// Make sure the lru address is positive
		if (temp_int < 0)
			continue;

		// If the bus name is not already a key, create a new set and add it into the map
		if (bus_name_to_lru_addresses_comet_map_.find(bus_name) 
			== bus_name_to_lru_addresses_comet_map_.end())
		{
			std::set<uint64_t> temp_set;
			temp_set.insert(temp_int);
			bus_name_to_lru_addresses_comet_map_[bus_name] = temp_set;
		}
		else
		{
			bus_name_to_lru_addresses_comet_map_[bus_name].insert(temp_int);
		}
	}

	if (tmats_present_)
		CleanTmatsMaps();
}

/*
	Consolide tmats_chanid_to_source_map from the tmats file
	create tmats_1553_chanid_to_busname_map_
	Includes:
	1. TMATS bus names that are equal to comet bus names
	2. Only includes TMATS channel IDs that appear in the scanned channel id map
*/
void BusMap::CleanTmatsMaps()
{
	std::map<uint64_t, std::string> temp_tmats_chanid_to_source_map 
		= tmats_chanid_to_source_map_;

	// Correct tmats bus names according to config file specifications
	temp_tmats_chanid_to_source_map = 
		iterable_tools_.UpdateMapVals<uint64_t, std::string>(tmats_chanid_to_source_map_, 
			tmats_busname_corrections_);
	
	// Insert all comet bus names into a set
	std::set<std::string> comet_bus_names = 
		iterable_tools_.VecToSet(
			iterable_tools_.GetKeys<std::string, std::set<uint64_t>>(bus_name_to_lru_addresses_comet_map_));
	
	// Add to the final tmats mapping if the tmats bus name
	// is equivalent to the bus name found in comet
	for (std::map<uint64_t, std::string>::iterator 
		it = temp_tmats_chanid_to_source_map.begin(); 
		it != temp_tmats_chanid_to_source_map.end(); it++)
	{
		for (auto comet_bus : comet_bus_names)
		{
			// If the comet bus name is the same name as the tmats bus name 
			// AND the tmats channel ID exists in the scanned channel id map
			// add the comet bus name to the final tmats chid to bus name map
			if (it->second == comet_bus 
				&& ch10_scanned_chanid_to_lruaddrs_map_.count(it->first) > 0)
			{
				tmats_1553_chanid_to_busname_map_[it->first] = comet_bus;
			}
		}
		
	}
}

void BusMap::PrepareFinalMap()
{
	for (std::map<uint64_t, std::pair<std::string, std::string>>::iterator 
		it = final_bus_map_with_sources_.begin(); 
		it != final_bus_map_with_sources_.end(); it++)
	{
		(*final_map_ptr_)[it->first] = it->second.first;
	}
}

void BusMap::FillFinalBusMap(const std::map<uint64_t, 
	std::string>& insert_map, 
	std::string source)
{
	// iterate over the map to add, if the channel ID does not 
	// exist yet in the final bus map, add it in with the source
	for (std::map<uint64_t, std::string>::const_iterator 
		it = insert_map.begin(); it != insert_map.end(); it++)
	{
		if (final_bus_map_with_sources_.count(it->first) == 0 
			&& ch10_scanned_chanid_to_lruaddrs_map_.count(it->first) > 0)
		{
			final_bus_map_with_sources_[it->first] = 
				std::pair<std::string, std::string>(it->second, source);
		}
	}
}

void BusMap::FillSuggestionsMap(const std::map<uint64_t, std::string>& unique_lru_map, 
	const std::map<uint64_t, std::string>& unique_subset_map, 
	const std::map<uint64_t, std::string>& subset_map,
	const std::map<uint64_t, std::string>& trailing_map)
{
	// iterate over each map
	// IF the channel ID does not exist yet in the suggestions map 
	// AND the channel ID does exist in the scanned map
	// add to the the suggestions map
	for (std::map<uint64_t, std::string>::const_iterator 
		it = unique_lru_map.begin(); 
		it != unique_lru_map.end(); it++)
	{
		if (bus_map_suggestions_.count(it->first) == 0 
			&& ch10_scanned_chanid_to_lruaddrs_map_.count(it->first) > 0)
		{
			bus_map_suggestions_[it->first] = 
				std::pair<std::string, std::string>(it->second, "UniqueLRU");
		}
	}

	for (std::map<uint64_t, std::string>::const_iterator 
		it = unique_subset_map.begin(); 
		it != unique_subset_map.end(); it++)
	{
		if (bus_map_suggestions_.count(it->first) == 0 
			&& ch10_scanned_chanid_to_lruaddrs_map_.count(it->first) > 0)
		{
			bus_map_suggestions_[it->first] = 
				std::pair<std::string, std::string>(it->second, "UniqueSubset");
		}
	}

	for (std::map<uint64_t, std::string>::const_iterator 
		it = tmats_1553_chanid_to_busname_map_.begin(); 
		it != tmats_1553_chanid_to_busname_map_.end(); it++)
	{
		if (bus_map_suggestions_.count(it->first) == 0 
			&& ch10_scanned_chanid_to_lruaddrs_map_.count(it->first) > 0)
		{
			bus_map_suggestions_[it->first] = 
				std::pair<std::string, std::string>(it->second, "TMATS");
		}
	}

	for (std::map<uint64_t, std::string>::const_iterator 
		it = subset_map.begin(); it != subset_map.end(); it++)
	{
		if (bus_map_suggestions_.count(it->first) == 0 
			&& ch10_scanned_chanid_to_lruaddrs_map_.count(it->first) > 0)
		{
			bus_map_suggestions_[it->first] = 
				std::pair<std::string, std::string>(it->second, "Subset");
		}
	}

	for (std::map<uint64_t, std::string>::const_iterator 
		it = trailing_map.begin(); 
		it != trailing_map.end(); it++)
	{
		if (bus_map_suggestions_.count(it->first) == 0 
			&& ch10_scanned_chanid_to_lruaddrs_map_.count(it->first) > 0)
		{
			bus_map_suggestions_[it->first] = 
				std::pair<std::string, std::string>(it->second, "TrailingSubset");
		}
	}

	// Add any additional channel ids that were not mapped by any 
	// of the above methods
	for (std::map<uint64_t, std::set<uint64_t>>::const_iterator 
		it = ch10_scanned_chanid_to_lruaddrs_map_.begin(); 
		it != ch10_scanned_chanid_to_lruaddrs_map_.end(); it++)
	{
		if (bus_map_suggestions_.count(it->first) == 0)
		{
			bus_map_suggestions_[it->first] = 
				std::pair<std::string, std::string>("NA", "NONE");
		}
	}
}

std::map<uint64_t, std::string> BusMap::SubsetMapping(bool unique, 
	std::set<std::string> comet_skips, 
	std::set<uint64_t> scanned_skips)
{
	std::map<uint64_t, std::string> subset_map;

	std::vector<std::string> comet_map_vec = 
		iterable_tools_.SortMapToIterableByValueSize(bus_name_to_lru_addresses_comet_map_);
	
	std::reverse(comet_map_vec.begin(), comet_map_vec.end());
	
	std::vector<std::uint64_t> scanned_map_vec = 
		iterable_tools_.SortMapToIterableByValueSize(ch10_scanned_chanid_to_lruaddrs_map_);
	std::reverse(scanned_map_vec.begin(), scanned_map_vec.end());

	// Remove Skips from the comet map
	for (auto comet_skip : comet_skips)
	{
		comet_map_vec.erase(std::remove(comet_map_vec.begin(), comet_map_vec.end(), comet_skip), 
			comet_map_vec.end());
	}

	// Remove Skips from the scanned map
	for (auto scanned_skip : scanned_skips)
	{
		scanned_map_vec.erase(std::remove(scanned_map_vec.begin(), scanned_map_vec.end(), scanned_skip), 
			scanned_map_vec.end());
	}

	// Removal of subset matches as you go
	// Subsets don't need to be unique in the comet map
	if (unique == false)
	{
		for (int i = 0; i < comet_map_vec.size(); i++)
		{
			std::string comet_bus = comet_map_vec[i];
			std::set<uint64_t> comet_set = 
				bus_name_to_lru_addresses_comet_map_[comet_bus];

			for (int j = 0; j < scanned_map_vec.size(); j++)
			{
				uint64_t scanned_chid = scanned_map_vec[j];
				std::set<uint64_t> scanned_set = 
					ch10_scanned_chanid_to_lruaddrs_map_[scanned_chid];

				// If the scanned lru address set is a subset of the comet lru 
				// address subset, add the channel id to the channel 
				// id -> to bus name map
				if (iterable_tools_.IsSubset(scanned_set, comet_set))
				{
					subset_map[scanned_chid] = comet_bus;

					// Remove the scanned match so it does not get 
					// compared again
					scanned_map_vec.erase(scanned_map_vec.begin() + j);
					break;
				}
			}
		}
	}
	// Search for unique subsets
	else
	{
		for (int i = 0; i < scanned_map_vec.size(); i++)
		{
			uint64_t scanned_chid = scanned_map_vec[i];
			int matches = 0;
			std::map<uint64_t, std::string> temp_match_map;
			std::set<uint64_t> scanned_set = ch10_scanned_chanid_to_lruaddrs_map_[scanned_chid];

			for (int j = 0; j < comet_map_vec.size(); j++)
			{
				std::string comet_bus = comet_map_vec[j];
				std::set<uint64_t> comet_set = bus_name_to_lru_addresses_comet_map_[comet_bus];

				// If the scanned lru address set is a subset of the comet 
				// lru address subset, add the channel id to the temp channel 
				// id -> to bus name map and log a matched item
				if (iterable_tools_.IsSubset(scanned_set, comet_set))
				{
					temp_match_map[scanned_chid] = comet_bus;
					matches++;
				}	
			}
			// If only one match was found, add the match to the map
			if (matches == 1)
			{
				subset_map = iterable_tools_.CombineMaps(subset_map, temp_match_map);
			}
		}
		
	}

	return subset_map;
}

std::map<uint64_t, std::string> BusMap::TrailingSubsetMapping(std::set<std::string> comet_skips, 
	std::set<uint64_t> scanned_skips)
{
	std::map<uint64_t, std::string> return_map = SubsetMapping(false, comet_skips, scanned_skips);
	std::map<uint64_t, std::string> new_map = return_map;

	while (!new_map.empty())
	{
		// Update return map with newly found matches
		return_map = iterable_tools_.CombineMaps(return_map, new_map);

		// Add newly found scanned skips
		for (std::map<uint64_t, std::string>::iterator 
			it = new_map.begin(); 
			it != new_map.end(); it++)
		{
			scanned_skips.insert(it->first);
		}
		
		new_map = SubsetMapping(false, comet_skips, scanned_skips);
	}

	return return_map;
}

std::map<uint64_t, std::string> BusMap::UniqueLRUIdentification()
{
	std::map<uint64_t, std::string> return_map;

	// Find unique LRU addresses
	std::map<uint64_t, std::string> unique_lrus_in_bus_map;
	for (std::map<std::string, std::set<uint64_t>>::iterator 
		it = bus_name_to_lru_addresses_comet_map_.begin(); 
		it != bus_name_to_lru_addresses_comet_map_.end(); it++)
	{
		std::set<uint64_t> unique_set = it->second;
		for (std::map<std::string, std::set<uint64_t>>::iterator 
			it2 = bus_name_to_lru_addresses_comet_map_.begin(); 
			it2 != bus_name_to_lru_addresses_comet_map_.end(); it2++)
		{
			// If it is the same entry, skip
			if (it->first == it2->first)
				continue;

			// Remove intersection from the list of LRUs
			std::set<uint64_t> intersect = 
				iterable_tools_.Intersection(unique_set, it2->second);

			unique_set = iterable_tools_.DeleteFromSet(unique_set, intersect);
		}

		// If there are LRUs left after looking for duplicates, 
		// point the LRU to the associated bus in the map
		for (auto lru : unique_set)
		{
			unique_lrus_in_bus_map[lru] = it->first;
		}
	}

	// Search all lrus in the scanned chid to lru addresses map
	for (std::map<uint64_t, std::set<uint64_t>>::iterator 
		it = ch10_scanned_chanid_to_lruaddrs_map_.begin(); 
		it != ch10_scanned_chanid_to_lruaddrs_map_.end(); it++)
	{
		for (auto lru : it->second)
		{
			// If any lrus are unique to a specific bus as determined 
			// by comet, match the channel id to the bus name from the unique map
			if (iterable_tools_.IsKeyInMap(unique_lrus_in_bus_map, lru))
			{
				return_map[it->first] = unique_lrus_in_bus_map[lru];
				break;
			}
		}
	}

	return return_map;
}

void BusMap::InitializeMaps(std::map<std::string, std::set<uint64_t>> comet_map,
								std::map<uint64_t, std::set<uint64_t>> chanid_to_lruaddrs_map,									
								std::map<uint64_t, std::string> tmats_chanid_to_source_map,								
								std::map<std::string, std::string> tmats_busname_corrections)
{
	// Read in the comet map directly
	bus_name_to_lru_addresses_comet_map_ = comet_map;

	tmats_busname_corrections_ = tmats_busname_corrections;

	// Copy the important maps retrieved from metadata
	ch10_scanned_chanid_to_lruaddrs_map_ = chanid_to_lruaddrs_map;
	tmats_chanid_to_source_map_ = tmats_chanid_to_source_map;
	 
	if (tmats_chanid_to_source_map.empty())
		tmats_present_ = false;
	else
		tmats_present_ = true;

	if (tmats_present_)
		CleanTmatsMaps();
}

bool BusMap::PerformBusMapping(std::map<uint64_t, std::string>& final_map, 
	uint64_t auto_map_confidence_level, 
	bool user_input_if_not_complete,
	std::vector<std::string>* user_test_input)
{
	final_map_ptr_ = &final_map;

	// IF the auto map confidence level isn't valid print error 
	// and return with a false
	if (auto_map_confidence_level > 4 || auto_map_confidence_level < 1)
	{
		printf("\nERROR--- AUTO MAP CONFIDENCE LEVEL MUST BE BETWEEN");
		printf("1 AND 4 AS SPECIFIED IN CONFIGURATION FILE\n");
		return false;
	}

	// Clear anything already in the final map
	final_map.clear();

	// Get bus mapping for buses that have LRUs not 
	// present on other buses
	std::map<uint64_t, std::string> unique_lru_map = UniqueLRUIdentification();

	// Get bus mapping channel ids with lru addresses 
	// that are unique subsets of the comet map
	std::map<uint64_t, std::string> unique_subset_map = SubsetMapping(true);

	// Set subset mapping from comparison by largest subsets first 
	// and removal as matches are found
	std::map<uint64_t, std::string> subset_map = SubsetMapping(false);

	// Trailing subset mapping called after skipping all initial first 
	// pass subset mappings
	std::map<uint64_t, std::string> trailing_map = 
		TrailingSubsetMapping(std::set<std::string>(), 
			iterable_tools_.VecToSet(iterable_tools_.GetKeys(subset_map)));

	// tmats mapping is in tmats_1553_chanid_to_busname_map_

	// Print out maps
	iterable_tools_.PrintMapWithHeader_KeyToSet<uint64_t, uint64_t>
		(ch10_scanned_chanid_to_lruaddrs_map_, 
			std::vector<std::string>({ "chID", "LRU" }), 
			"Chapter 10 Scanned Bus Map");

	iterable_tools_.PrintMapWithHeader_KeyToSet<std::string, uint64_t>
		(bus_name_to_lru_addresses_comet_map_, 
			std::vector<std::string>({ "Bus", "LRU" }), 
			"Comet Bus Map");

	if (tmats_1553_chanid_to_busname_map_.size() > 0)
		iterable_tools_.PrintMapWithHeader_KeyToValue<uint64_t, std::string>
		(tmats_1553_chanid_to_busname_map_, 
			std::vector<std::string>({ "chID", "Bus" }), 
			"TMATS Map");

	// Build suggestions map
	FillSuggestionsMap(unique_lru_map, unique_subset_map, subset_map, trailing_map);

	if (auto_map_confidence_level == 1)
	{
		FillFinalBusMap(unique_lru_map, "UniqueLRU");
		FillFinalBusMap(unique_subset_map, "UniqueSubset");
				
	}
	else if (auto_map_confidence_level == 2)
	{
		FillFinalBusMap(unique_lru_map, "UniqueLRU");
		FillFinalBusMap(unique_subset_map, "UniqueSubset");	
		FillFinalBusMap(tmats_1553_chanid_to_busname_map_, "TMATS");
	}
	else if (auto_map_confidence_level == 3)
	{
		FillFinalBusMap(unique_lru_map, "UniqueLRU");
		FillFinalBusMap(unique_subset_map, "UniqueSubset");	
		FillFinalBusMap(tmats_1553_chanid_to_busname_map_, "TMATS");
		FillFinalBusMap(subset_map, "Subset");
	}
	else if (auto_map_confidence_level == 4)
	{
		FillFinalBusMap(unique_lru_map, "UniqueLRU");
		FillFinalBusMap(unique_subset_map, "UniqueSubset");
		FillFinalBusMap(tmats_1553_chanid_to_busname_map_, "TMATS");
		FillFinalBusMap(subset_map, "Subset");
		FillFinalBusMap(trailing_map, "TrailingSubset");
	}

	// Print Final Bus Map
	PrintFinalBusMap();
	

	// If final map contains all scanned channel IDS return true
	if (iterable_tools_.GetKeys(final_bus_map_with_sources_).size() ==
		iterable_tools_.GetKeys(ch10_scanned_chanid_to_lruaddrs_map_).size() &&
		iterable_tools_.GetKeys(final_bus_map_with_sources_).size() > 0)
	{
		PrepareFinalMap();
		return true;
	}
			
	// If user_input_if_not_complete is false
	// If the final bus map has any mappings return true
	if (!user_input_if_not_complete)
	{
		// Check if return map is > 1
		if (iterable_tools_.GetKeys(final_bus_map_with_sources_).size() > 0)
		{
			PrepareFinalMap();
			return true;
		}
		else
			return false;
	}
	// If user_input_if_not_complete is true, let the use make adjustments
	else
	{
		if (UserAdjustments(user_test_input))
		{
			PrepareFinalMap();
			return true;
		}
		else
			return false;
	}

	
}

std::map<uint64_t, std::string> 
	BusMap::ReturnMapsForTesting(std::string function,
									std::map<uint64_t, std::set<uint64_t>> temp_ch10_scanned_chanid_to_lruaddrs_map, 
									std::map<std::string, std::set<uint64_t>> temp_bus_name_to_lru_addresses_comet_map, 
									std::set<std::string> comet_skips, 
									std::set<uint64_t> scanned_skips)
{
	ch10_scanned_chanid_to_lruaddrs_map_ = temp_ch10_scanned_chanid_to_lruaddrs_map;
	bus_name_to_lru_addresses_comet_map_ = temp_bus_name_to_lru_addresses_comet_map;

	if (function == "UniqueLRUIdentification")
		return UniqueLRUIdentification();
	else if (function == "UniqueSubsetMapping")
		return SubsetMapping(true, comet_skips, scanned_skips);
	else if (function == "SubsetMapping")
		return SubsetMapping(false, comet_skips, scanned_skips);
	else if (function == "TrailingSubsetMapping")
		return TrailingSubsetMapping(comet_skips, scanned_skips);
	else
		return std::map<uint64_t,std::string>();
}

std::string BusMap::PrintFinalBusMap()
{
	std::stringstream ss;
	ss << iterable_tools_.GetHeader(std::vector<std::string>({ "chID", "Bus", "Source" }), 
		"Final Bus Map");

	// Output all the final mapping elements
	for (std::map<uint64_t, std::pair<std::string, std::string>>::const_iterator 
		it = final_bus_map_with_sources_.begin(); 
		it != final_bus_map_with_sources_.end(); it++)
	{
		ss << " " << it->first << "\t" << it->second.first << "\t" << it->second.second << "\n";
	}

	if (iterable_tools_.GetKeys(final_bus_map_with_sources_).size() 
		!= iterable_tools_.GetKeys(ch10_scanned_chanid_to_lruaddrs_map_).size())
		ss << "\n--Unmapped Suggestion(s)--\n";
	else
		ss << "\n--All Channel IDs Mapped--\n";

	// Print out any unmapped elements with their suggestions
	for (std::map<uint64_t, std::pair<std::string, std::string>>::const_iterator 
		it = bus_map_suggestions_.begin(); 
		it != bus_map_suggestions_.end(); it++)
	{
		if (final_bus_map_with_sources_.count(it->first) == 0)
			ss << " " << it->first << "\t" << it->second.first << 
			"\t" << it->second.second << "\n";
	}

	ss << iterable_tools_.GetPrintBar() << "\n\n";

	printf(ss.str().c_str());
	return ss.str();
}

bool BusMap::UserAdjustments(std::vector<std::string>* test_options)
{
	// Get potential channel ids and bus names
	std::set<std::uint64_t> valid_channel_ids = 
		iterable_tools_.VecToSet(iterable_tools_.GetKeys(ch10_scanned_chanid_to_lruaddrs_map_));
	std::set<std::string> valid_bus_names = 
		iterable_tools_.VecToSet(iterable_tools_.GetKeys(bus_name_to_lru_addresses_comet_map_));

	std::string user_input = "3";
	bool continue_execution = true;

	// Continue Prompting user until they enter 1, 2, or q
	std::set<std::string> valid_options({ "1","2" });
	while (1)
	{
		// Initial options
		std::string base_prompt = "";
		base_prompt = base_prompt +
			"Bus Map Adjustment Options:\n" +
			"1:\tUse Current Bus Map\n" + 
			"2:\tAdjust Bus Map\n" +
			"q:\tQuit Translation";

		continue_execution = user_input_.GetString(
			user_input, 
			base_prompt, 
			&valid_options, 
			test_options);

		// If the user entered a "q", exit and disscontinue translation
		if (!continue_execution)
			return false;
		// If the user entered a one continue with currect bus map
		else if (user_input == "1")
		{
			PrepareFinalMap();
			return true;
		}
		// If the user entered two continue to adjustment section
		else if (user_input == "2")
		{
			uint64_t channel_id_input;
			std::string bus_input;
			continue_execution = user_input_.GetUnsignedInt(
				channel_id_input, 
				"Enter a Channel ID To Map:", 
				&valid_channel_ids, 
				test_options);

			// If "q" was used, exit and disscontinue translation
			if (!continue_execution)
				return false;

			std::string prompt = "Enter a Bus Name For Channel ID " 
				+ std::to_string(channel_id_input) +":";

			continue_execution = 
				user_input_.GetString(bus_input, prompt, 
					&valid_bus_names, 
					test_options);

			// If "q" was used, exit and disscontinue translation
			if (!continue_execution)
				return false;

			// If at this point the user should have given a valid
			// channel ID and bus name for adjustment
			final_bus_map_with_sources_[channel_id_input] = 
				std::pair<std::string,std::string>(bus_input,"USER");

			PrintFinalBusMap();
		}
	}
}
