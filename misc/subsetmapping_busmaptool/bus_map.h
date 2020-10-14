#ifndef BUSMAP_H
#define BUSMAP_H

#include <string>
#include <map>
#include <unordered_map>
#include <set>
#include <fstream>
#include <vector>
#include <sstream>
#include <iostream>
#include "parse_text.h"
#include "iterable_tools.h"
#include "user_input.h"

class BusMap
{
private:
	/*
		Pulled directly from metadata 
		("sources" are bus name specifications in TMATS)
	*/
	std::map<uint64_t, std::string> tmats_chanid_to_source_map_;		

	/*
		Pulled from comet flat files
	*/
	std::map<std::string, std::set<uint64_t>> bus_name_to_lru_addresses_comet_map_;		
	
	/*
		Scanned from the chapter 10 (retrieved from matadata)
		The goal is to map every chanid in this map to a bus name
	*/
	std::map<uint64_t, std::set<uint64_t>> ch10_scanned_chanid_to_lruaddrs_map_;		
	
	/*
		Consolidated version of tmats_chanid_to_source_map from the tmats file 
		Includes:
		1. TMATS bus names that are supersets of comet bus names 
		   (the TMATS bus names is updated to the comet bus name if
		   the comet bus name is a subset of the tmats bus name)
		2. Only includes TMATS channel IDs that appear in the scanned channel id map
	*/
	std::map<uint64_t, std::string> tmats_1553_chanid_to_busname_map_;
	
	/*
		Channel id to bus name together with the source used for the final mapping
	*/
	std::map<uint64_t, std::pair<std::string, std::string>> final_bus_map_with_sources_;
	
	/*
		Channel id to bus name together with the source used for suggestion sources
		also monitors missing elements from final_bus_map_with_sources_
	*/
	std::map<uint64_t, std::pair<std::string, std::string>> bus_map_suggestions_;
	
	/*
		Corrections to TMATS bus names 
		key		-> tmats bus name
		value	-> bus name that tmats is corrected to
	*/
	std::map<std::string, std::string> tmats_busname_corrections_;	

	ParseText parse_text_;
	IterableTools iterable_tools_;
	bool tmats_present_;
	UserInput user_input_;

	void PrepareFinalMap();

	void FillFinalBusMap(const std::map<uint64_t, 
		std::string>& insert_map, 
		std::string source);

	void CleanTmatsMaps();
	std::string PrintFinalBusMap();
	

	void FillSuggestionsMap(const std::map<uint64_t, std::string>& unique_lru_map,
		const std::map<uint64_t, std::string>& unique_subset_map,
		const std::map<uint64_t, std::string>& subset_map,
		const std::map<uint64_t, std::string>& trailing_map);

	/*
		Checks if scanned lru addresses are a subset of comet lru addresses and 
		returns	matching busnames to channelids
	
		unique =  true:  "UniqueSubset" source, LRU addresses must be unique subsets 
						 of the comet map
		unique = false:  "Subset" source, LRU addresses don't need to be unique subsets
						 in the comet map and it removes both comet map and scanned
						 matches as it goes also sorting by largest lru subsets
						 first and doing comparisons on largest subsets first

		comet_skips:	Removes the comet bus names before beginning the search
		scanned_skips:	Removes the scanned channel IDs before beginning search
	*/
	std::map<uint64_t, std::string> SubsetMapping(bool unique, 
		std::set<std::string> comet_skips = std::set<std::string>(), 
		std::set<uint64_t> scanned_skips = std::set<uint64_t>());

	// Runs SubsetMapping in a loop with unique=false until it can not find any more matches
	std::map<uint64_t, std::string> 
		TrailingSubsetMapping(std::set<std::string> comet_skips = std::set<std::string>(), 
		std::set<uint64_t> scanned_skips = std::set<uint64_t>());

	// Returns the bus map from unique LRU addresses existing on a bus
	std::map<uint64_t, std::string> UniqueLRUIdentification();

	// Used to bind final_map input from PerformBusMapping
	// so that PrepareFinalMap can prepare the final output
	std::map<uint64_t, std::string>* final_map_ptr_;


public:
	BusMap() : tmats_present_(false) {};
	~BusMap() {};

	/*
	 Initialize bus map with required maps
	
	 comet_map					-> retrieved from comet, map of bus names to 
									a set of channel IDs
	 chanid_to_lruaddrs_map		-> map of channel ids to lru addresses from the 
									chapter 10 the information is in metadata  
									inside the parsed ch10 parquet file
	 tmats_chanid_to_source_map	-> non required map of channel ids to source(bus name) 
									from tmats the information is in metadata 
									inside the parsed ch10 parquet file
	 tmats_busname_corrections  -> non required map of tmats bus name corrections. 
									It will correct the tmats source name (busname) 
									using the key as the tmats source name and the 
									value as the desired correction
	*/
	void InitializeMaps(
		std::map<std::string, std::set<uint64_t>> comet_map,
		std::map<uint64_t, std::set<uint64_t>> chanid_to_lruaddrs_map,
		std::map<uint64_t, std::string> tmats_chanid_to_source_map 
		= std::map<uint64_t, std::string>(),
		std::map<std::string, std::string> tmats_busname_corrections 
		= std::map<std::string, std::string>());

	/*
		 Perform Bus Mapping
	
		 Returns: True -> If all scanned channel ids are mapped within the 
							auto_map_confidence_level
						  |OR| at least one ch10_scanned_chanid_to_lruaddrs_map_ 
							channel id is mapped within the auto_map_confidence_level 
							and user_input_if_not_complete is false
						  |OR| user_input_if_not_complete is set to true and the user 
							selects option 1 to continue with bus map (user input is 
							available when mapping is incomplete and user_input_if_not_complete 
							is set to true)
				  False-> If zero ch10_scanned_chanid_to_lruaddrs_map_ channel ids are
							mapped and user_input_if_not_complete is false
						  |OR| user_input_if_not_complete is set to true and the user 
							selects option "q" to quit translation
	
		 final_map					-> channel ID to bus name map passed by reference and 
										filled out by bus map
		 auto_map_confidence_level	-> confidence level specified in config file (1,2,3 or 4).
										More detail in config file comments
		 user_input_if_not_complete	-> True: If all the ch10_scanned_chanid_to_lruaddrs_map_ 
												channel ids are not mapped within the
												auto_map_confidence_level, prompt the user 
												for help with bus mapping
									   False: Continue translation without user intervention 
												if at least one scanned channel id was mapped 
												within the confidence level
		user_test_input				-> Used to by pass user input for unit tests
	*/
	bool PerformBusMapping(std::map<uint64_t, std::string>& final_map, 
		uint64_t auto_map_confidence_level, 
		bool user_input_if_not_complete,
		std::vector<std::string>* test_options = NULL);

	/*
		Old Function replaced by comet_map input in InitializeMaps
		Adds a vector of strings with the format LRUNAME|BUSNAME|LRUADDRESS
		to bus_name_to_lru_addresses_comet_map_, the format is consistent
		with comet mux_term_addr files.
	*/
	void AddLinesTo_BusnameLRUAddressMap(std::vector<std::string> term_mux_lines);	




	 
	///////////////////////// Utilities used for unit tests
	const std::map<std::string, std::set<uint64_t>>& GetBusName_ToLRUAddressesCometMap() 
	{ return bus_name_to_lru_addresses_comet_map_; }

	const std::map<uint64_t, std::set<uint64_t>>& GetCH10ScannedChanID_ToLRUAddressesMap() 
	{ return ch10_scanned_chanid_to_lruaddrs_map_; }

	const std::map<uint64_t, std::string>& GetTmats1553ChanID_ToBusNameMap() 
	{ return tmats_1553_chanid_to_busname_map_; }

	const std::map<uint64_t, std::pair<std::string, std::string>>& GetBusSuggestionsMap() 
	{ return bus_map_suggestions_; }

	const std::map<uint64_t, std::pair<std::string, std::string>>& GetFinalBusMap_withSource() 
	{ return final_bus_map_with_sources_; }

	std::map<uint64_t, std::string> ReturnMapsForTesting(std::string function, 
		std::map<uint64_t, std::set<uint64_t>> temp_ch10_scanned_chanid_to_lruaddrs_map, 
		std::map<std::string, std::set<uint64_t>> temp_bus_name_to_lru_addresses_comet_map,
		std::set<std::string> comet_skips = std::set<std::string>(), 
		std::set<uint64_t> scanned_skips = std::set<uint64_t>());

	bool UserAdjustments(std::vector<std::string>* test_options = NULL);

	bool TmatsPresent()
	{return tmats_present_;}
	/////////////////////////
	
	
};

#endif

/*-
# bus_map_confidence_level
#
# Bus mapping must be done to match ch10 channel IDs to bus names found in the ICD.
# Bus map confidence levels can be specified to restrict the final bus map to only
# map buses within a certain confidence level. If prompt user is set to false, translation
# will continue with the buses mapped within the confidence level, even if all chapter 10 
# channel IDs from the chapter 10 are not mapped
#
# The bus map will always use the most reliable source of bus mapping first, and then will continue
# to map other buses using other sources within the bus_map_confidence_level restriction set by the user.
#
# Bus mapping sources are as follows (in order of most reliable to least reliable):
#	UniqueLRU		-> An LRU address in the chapter 10 exists on only ONE bus in the ICD. 
#	UniqueSubset	-> A set of chapter 10 lru addresses for a given channel ID is a subset of only ONE set of LRUs in the ICD for a given bus. 
#	TMATS			-> TMATS mapping is often provided by ch10 files containing channel IDs to bus names.
#						If TMATS bus names are consistently different from ICD specifications, TMATS bus name
#						corrections can be given in the platform specific configuration file
#						using the parameter "mapped_tmats_bus"
#	Subset 			-> LRU addresses are matched in order of largest to smallest LRU address sets.
#						If the chapter 10 LRU address set for a given channel ID is a subset of an ICD LRU address set for a given bus, 
#						a match is made	and the matches are removed from the search. The search will then continue to the next
#						largest LRU address set looking for subsets. The search is iterated once and then stopped.
#	TrailingSubset	-> Just like the Subset process, Trailing subsets are matched in order of largest to smallest LRU address sets.
#						Matches are also removed from the search on each iteration. The difference is that the search 
#						will continue looping until all possible subsets are matched. At the beginning of each iteration
#						all ICD bus maps are added back into the search, while previously matched channel IDs are left out.
# 	
# The bus map will use the following order to select matches based off of the bus_map_confidence_level provided:
#
# 1: UniqueLRU -> UniqueSubset
# 2: UniqueLRU -> UniqueSubset -> TMATS
# 3: UniqueLRU -> UniqueSubset -> TMATS -> Subset
# 4: UniqueLRU -> UniqueSubset -> TMATS -> Subset -> TrailingSubset
# 
# bus_map_confidence_level = 1 can be used with 100% confidence if the provided ICD is correct
#
bus_map_confidence_level: 1

#
# comet_busmap_replacement
#
# When the comet busname to lru address set cannot be deduced from the provided ICD,
# a replacement can be provided through this config option.
# 
# Provide comet_busmap_replacement entries if you want the config
# entries to be used instead of being drawn from the ICD.
# Leave comet_busmap_replacement empty if you want the entries drawn
# from the ICD.
# Each entry consists of <bus name> : [<list of lru addresses>]
#
# Examples:
# (Use comet_busmap_replacement instead of mappings drawn from the ICD)
# comet_busmap_replacement:
# {
#  A: [1,2,3], 
#  B: [2,5,8], 
#  C: [1,3,4]
# }
#
# (Use mappings drawn from ICD)
# comet_busmap_replacement: 
#   {}
#
comet_busmap_replacement:
  {}
*/