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
	const std::unordered_map<uint64_t, std::set<std::string>> *icd_message_key_to_busnames_map_;
	std::unordered_map<uint64_t, std::set<uint64_t>> icd_message_key_to_channelids_map_;
	std::map<uint64_t, std::string> tmats_chanid_to_source_map_;
	std::set<std::string> unique_buses_;
	std::set<uint64_t> channel_ids_;
	int64_t key;
	std::string PrintFinalMap();
	void SubmitToFinalBusMap(const std::map<uint64_t,
		std::string>& insert_map,
		std::string source);
	

	std::map<uint64_t, std::string> VoteMapping();			
	
	/*
		Channel id to bus name together with the source used for the final mapping
	*/
	std::map<uint64_t, std::pair<std::string, std::string>> final_bus_map_with_sources_;
	
	/*
		Corrections to TMATS bus names 
		key		-> tmats bus name
		value	-> bus name that tmats is corrected to
	*/
	std::map<std::string, std::string> tmats_busname_corrections_;	

	IterableTools iterable_tools_;
	bool tmats_present_;
	UserInput user_input_;

	void PrepareFinalMap();

	// Used to bind final_map input from PerformBusMapping
	// so that PrepareFinalMap can prepare the final output
	std::map<uint64_t, std::string>* final_map_ptr_;


public:
	BusMap() : tmats_present_(false) {};
	~BusMap() {};
	
	void InitializeMaps(
		const std::unordered_map<uint64_t, std::set<std::string>>* icd_message_keys_to_busnames,
		std::set<uint64_t> channel_ids,
		std::map<uint64_t, std::string> tmats_chanid_to_source_map
		= std::map<uint64_t, std::string>(),
		std::map<std::string, std::string> tmats_busname_corrections
		= std::map<std::string, std::string>());

	bool SubmitMessages(
			const std::vector<uint64_t>& transmit_cmd,
			const std::vector<uint64_t>& recieve_cmd,
			const std::vector<uint64_t>& channel_ids,
			size_t submission_size = -1);

	bool Finalize(std::map<uint64_t, std::string>& final_map,
		bool use_tmats_busmap,
		bool user_input_if_not_complete,
		std::vector<std::string>* test_options = NULL);

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



	 
	///////////////////////// Utilities used for unit tests

	const std::unordered_map<uint64_t, std::set<std::string>> * GetICD_MessageKeyToBusNamesMap()
	{ 
		return icd_message_key_to_busnames_map_;
	}

	std::unordered_map<uint64_t, std::set<uint64_t>> GetICD_MessageKeyToChannelIDSMap()
	{ 
		return icd_message_key_to_channelids_map_; 
	}
	
	std::set<std::string> GetUniqueBuses()
	{
		return unique_buses_;
	}

	std::set<uint64_t> GetChannelIDs()
	{
		return channel_ids_; 
	}

	bool TmatsPresent()
	{
		return tmats_present_;
	}

	const std::map<uint64_t, std::string>& GetTMATSchannelidToSourceMap()
	{
		return tmats_chanid_to_source_map_;
	}
	
	std::map<uint64_t, std::string> TestVoteMapping(		
		std::unordered_map<uint64_t, std::set<uint64_t>> icd_message_key_to_channelids_map)
	{
		icd_message_key_to_channelids_map_ = icd_message_key_to_channelids_map;
		return VoteMapping();
	}

	const std::map<uint64_t, std::pair<std::string, std::string>>& GetFinalBusMap_withSource() 
	{ 
		return final_bus_map_with_sources_; 
	}

	bool UserAdjustments(std::vector<std::string>* test_options = NULL);
	
	/////////////////////////
};

#endif