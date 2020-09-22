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

class UniqueKeyData
{
public:
	UniqueKeyData(std::set<size_t> channel_ids_, std::set<std::string> bus_names) : channel_ids_(channel_ids_), bus_names_(bus_names_)  {};
	std::set<std::string> bus_names_;
	std::set<size_t> channel_ids_;
};

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