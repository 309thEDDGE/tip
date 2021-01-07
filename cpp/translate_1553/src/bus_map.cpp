#include "bus_map.h"

void BusMap::PrepareFinalMap()
{
	for (std::map<uint64_t, std::pair<std::string, std::string>>::iterator 
		it = final_bus_map_with_sources_.begin(); 
		it != final_bus_map_with_sources_.end(); it++)
	{
		(*final_map_ptr_)[it->first] = it->second.first;
	}
}

void BusMap::InitializeMaps(const std::unordered_map<uint64_t, std::set<std::string>> *icd_message_keys, 
	std::set<uint64_t> channel_ids,
	uint64_t mask,
	uint64_t vote_threshold,
	bool vote_method_checks_tmats,
	std::set<std::string> bus_exclusions,
	std::map<uint64_t, std::string> tmats_chanid_to_source_map, 
	std::map<std::string, std::string> tmats_busname_corrections)
{
	vote_threshold_ = vote_threshold;
	tmats_chanid_to_source_map_ = tmats_chanid_to_source_map;
	channel_ids_ = channel_ids;
	mask_ = mask;
	vote_method_checks_tmats_ = vote_method_checks_tmats;
	tmats_busname_corrections_ = tmats_busname_corrections;

	// Filter out channel IDs in TMATs that are not
	// in the channel ID master list
	std::map<uint64_t, std::string> temp;
	for (std::map<uint64_t, std::string>::iterator it =
		tmats_chanid_to_source_map_.begin();
		it != tmats_chanid_to_source_map_.end();
		++it)
	{
		// if the tmats channel id exists in the set of 
		// channel ids provided by initialize maps,
		// add the channel id to a temp map that will be 
		// assigned to tmats_chanid_to_source_map_
		if (channel_ids_.count(it->first) == 1)
		{
			temp[it->first] = it->second;
		}
	}

	tmats_chanid_to_source_map_ = temp;

	// Print out maps
	printf("\nChannel IDs to Map----\n");
	for (auto channel_id : channel_ids_)
	{
		printf("%i\n", channel_id);
	}
	printf("----\n");
	
	// Print TMATS map before tmats busname corrections
	iterable_tools_.PrintMapWithHeader_KeyToValue<uint64_t, std::string>
	(tmats_chanid_to_source_map_,
		std::vector<std::string>({ "chID", "Bus" }),
		"Original TMATS Map");	

	if (tmats_chanid_to_source_map_.empty())
		tmats_present_ = false;
	else
	{
		tmats_present_ = true;
		// update tmats bus names with bus name corrections
		// found in the configuration file
		tmats_chanid_to_source_map_ = 
			iterable_tools_.UpdateMapVals<uint64_t, std::string>
			(	tmats_chanid_to_source_map_,
				tmats_busname_corrections);
	}

	// Print TMATS map after tmats busname corrections
	iterable_tools_.PrintMapWithHeader_KeyToValue<uint64_t, std::string>
	(tmats_chanid_to_source_map_,
		std::vector<std::string>({ "chID", "Bus" }),
		"TMATS Map After Corrections");

	// convert all bus exclusion elements to upper case
	// to remove case sensitivity
	for (auto bus : bus_exclusions)
	{
		std::transform(bus.begin(),
			bus.end(),
			bus.begin(),
			::toupper);

		upper_case_bus_exclusions_.insert(bus);
	}

	// Create message_key_to_channel_ids_map_ and
	// unique_bus_names_
	uint64_t key;
	for (std::unordered_map<uint64_t, std::set<std::string>>::const_iterator
		it = icd_message_keys->begin();
		it != icd_message_keys->end(); it++)
	{
		key = it->first & mask;
		if (icd_message_key_to_channelids_map_.count(key) == 0)
		{
			icd_message_key_to_channelids_map_[key] = std::set<uint64_t>();
			icd_message_key_to_busnames_map_[key] = it->second;
		}
		else
		{
			for (auto bus : it->second)
				icd_message_key_to_busnames_map_[key].insert(bus);
		}
		for (auto bus : it->second)
			unique_buses_.insert(bus);
	}		
}

std::string BusMap::PrintFinalMap()
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
		!= channel_ids_.size())
		ss << "\n--Unmapped (reason)--\n";
	else
		ss << "\n--All Channel IDs Mapped--\n";

	// Print out any unmapped elements
	for (auto channel_id : channel_ids_)
	{
		if (final_bus_map_with_sources_.count(channel_id) == 0)
		{
			std::string exclusion_reason = "";
			if (excluded_channel_ids_.count(channel_id) > 0)
				exclusion_reason = excluded_channel_ids_[channel_id];

			ss << " " << channel_id << "\t" << exclusion_reason << "\n";
		}
	}

	ss << iterable_tools_.GetPrintBar() << "\n\n";

	printf(ss.str().c_str());
	return ss.str();
}

bool BusMap::UserAdjustments(std::vector<std::string>* test_options)
{
	// Get potential channel ids and bus names
	std::set<std::uint64_t> valid_channel_ids = channel_ids_;
	std::set<std::string> valid_bus_names = unique_buses_;

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

			PrintFinalMap();
		}
	}
}

bool BusMap::SubmitMessages(
	const std::vector<uint64_t>& transmit_cmd,
	const std::vector<uint64_t>& recieve_cmd,
	const std::vector<uint64_t>& channel_ids,
	size_t submission_size)
{
	size_t count;

	// ensure vectors are of the same size
	if (transmit_cmd.size() != recieve_cmd.size()
		|| transmit_cmd.size() != channel_ids.size())
	{
		printf("Vectors must be of the same length in");
		printf(" BusMap::SubmitMessages\n");
		return false;
	}

	// if message_count is not the default value
	// submit the number of rows passed
	if (submission_size == -1)
	{
		count = transmit_cmd.size();
	}
	else
	{
		count = submission_size;
		if (count > transmit_cmd.size())
		{
			printf("message_count must be less than vector sizes ");
			printf("in BusMap::SubmitMessages\n");
			return false;
		}
	}
	for (int i = 0; i < count; i++)
	{
		key_ = ((transmit_cmd[i] << 16 ) 
			| (recieve_cmd[i])) & mask_;
		if (icd_message_key_to_channelids_map_.count(key_) == 1)
		{
			icd_message_key_to_channelids_map_[key_].insert(channel_ids[i]);
		}
	}
	return true;
}

void BusMap::SubmitMessage(const uint64_t& transmit_cmd, 
	const uint64_t& recieve_cmd, 
	const uint64_t& channel_id)
{
	key_ = ((transmit_cmd << 16)
		| (recieve_cmd)) & mask_;
	if (icd_message_key_to_channelids_map_.count(key_) == 1)
	{
		icd_message_key_to_channelids_map_[key_].insert(channel_id);
	}
}


std::map<uint64_t, std::string> BusMap::VoteMapping()
{
	std::map<uint64_t, std::string> vote_map;

	votes_.clear();

	// iterate over icd_message_key_to_channelids_map_ and extract 
	// out votes, each channel id found in icd_message_key_to_channelids_map_
	// votes once for all of the buses found in the corresponding
	// icd_message_key_to_busnames_map_
	for (std::unordered_map<uint64_t, std::set<uint64_t>>::iterator it =
		icd_message_key_to_channelids_map_.begin();
		it != icd_message_key_to_channelids_map_.end();
		++it)
	{
		for (auto channel_id : it->second)
		{
			// if the channel ID does not exist yet
			// in the vote map, initialize all the 
			// bus vote counts to zero
			if (votes_.count(channel_id) == 0)
			{
				for (auto bus : unique_buses_)
				{
					votes_[channel_id][bus] = 0;
				}				
			}

			for (auto bus : icd_message_key_to_busnames_map_[it->first])
			{
				++votes_[channel_id][bus];
			}
		}
	}

	// assign a bus to a channel id if it has the most votes
	// but not if the vote count is not >=
	// vote_threshold, and not if the number of
	// votes is the same as another bus
	int highest_vote = 0;
	int find_count = 0;
	std::string highest_voted_bus = "";
	for (std::unordered_map<uint64_t, std::unordered_map<std::string, uint64_t>>::iterator
		it = votes_.begin();
		it != votes_.end();
		++it)
	{
		highest_vote = 0;
		find_count = 0;
		highest_voted_bus = "";
		// iterate over the bus votes and determine the largest vote
		for (std::unordered_map<std::string, uint64_t>::iterator
			it2 = it->second.begin();
			it2 != it->second.end();
			++it2)
		{
			if (it2->second >= highest_vote)
			{
				++find_count;
				// if the bus vote count is larger than
				// the highest_vote found, reset the find
				// count to 1
				if (it2->second > highest_vote)
				{
					find_count = 1;
				}	
				highest_vote = it2->second;
				highest_voted_bus = it2->first;
			}
		}

		if (find_count == 1 && highest_vote >= vote_threshold_)
		{
			vote_map[it->first] = highest_voted_bus;			
		}
		else
		{
			if (highest_vote == 0)
				excluded_channel_ids_[it->first] = "No Votes";
			else if (highest_vote < vote_threshold_)
				excluded_channel_ids_[it->first] = "Vote count not >= vote_threshold";
			else if (find_count > 1)
				excluded_channel_ids_[it->first] = "Tie Vote";			
		}
	}

	// Add any channel ids that are not in the vote map
	// as "No Votes"
	for (auto channel_id : channel_ids_)
	{
		if(votes_.count(channel_id) == 0)
			excluded_channel_ids_[channel_id] = "No Votes";
	}

	/*
		If vote_method_checks_tmats_ = true
		make sure all the bus names match
		the bus names found in tmats for
		each channel ID, note! the mapped bus
		can be a substring of the TMATs bus name.
	*/
	if (vote_method_checks_tmats_)
	{
		for (std::map<uint64_t,std::string>::iterator it = vote_map.begin();
			it != vote_map.end();
			++it)
		// Check if the TMATs channel ID is present
		if (tmats_chanid_to_source_map_.count(it->first) == 0)
		{
			excluded_channel_ids_[it->first] = 
				"Missing From TMATS";
		}
		else
		{
			// If the DTS bus name and TMATS bus names are not
			// substrings of each other remove the channel IDs
			// from the final busmap
			if (tmats_chanid_to_source_map_[it->first].find(it->second)
				== std::string::npos && 
				it->second.find(tmats_chanid_to_source_map_[it->first]) == std::string::npos)
			{
				excluded_channel_ids_[it->first] =
					"TMATS Mismatch";
			}
		}
	}

	return vote_map;
}

std::map<uint64_t, std::string> BusMap::TmatsMapping()
{

	std::map<uint64_t, std::string> return_map;

	// Only map tmats bus names that exist in the DTS file
	for (std::map<uint64_t, std::string>::iterator it =
		tmats_chanid_to_source_map_.begin();
		it != tmats_chanid_to_source_map_.end();
		++it)
	{
		// if the tmats bus name matches a DTS file
		// bus name one for one, add the tmats mapping
		if (unique_buses_.count(it->second) == 1)
		{
			return_map[it->first] = it->second;
		}
		else
		{
			excluded_channel_ids_[it->first] = "TMATS Bus Name Not in DTS File";
		}
	}

	// Add any additional channel IDS that are missing from
	// TMATS to the excluded channel id list
	// Be sure not to override "TMATS Bus Name Not in DTS File" with 
	// the addition of && excluded_channel_ids_.count(channel_id) == 0
	for (auto channel_id : channel_ids_)
	{
		if (return_map.count(channel_id) == 0 &&
			excluded_channel_ids_.count(channel_id) == 0)
			excluded_channel_ids_[channel_id] = "Missing From TMATS";
	}

	return return_map;
}


bool BusMap::Finalize(std::map<uint64_t, std::string>& final_map,
	bool use_tmats_busmap,
	bool prompt_user,
	std::vector<std::string>* user_test_input)
{
	final_map_ptr_ = &final_map;

	// Clear anything already in the final map
	final_map.clear();

	// Clear any previously excluded channel ids
	excluded_channel_ids_.clear();

	// Exclude channel IDs when the bus_exclusion set contains
	// a bus name that is a substring of a bus name in TMATs.
	// The matches should not be case sensitive
	for (std::map<uint64_t, std::string>::iterator it = tmats_chanid_to_source_map_.begin();
		it != tmats_chanid_to_source_map_.end();
		++it)
	{
		std::string temp;
		temp = it->second;

		std::transform(it->second.begin(),
			it->second.end(),
			temp.begin(),
			::toupper);

		for (auto bus : upper_case_bus_exclusions_)
		{
			// handle exclusion bus being a substring 
			// of a TMATs bus name
			if (temp.find(bus) != std::string::npos)
				excluded_channel_ids_[it->first] =
				"Config Option: bus_name_exclusions";
		}
	}

	if (use_tmats_busmap)
	{
		std::map<uint64_t, std::string> tmats_mapping = TmatsMapping();
		SubmitToFinalBusMap(tmats_mapping, "TMATS");
	}
	else
	{
		std::map<uint64_t, std::string> vote_mapping = VoteMapping();
		if (vote_method_checks_tmats_)
			SubmitToFinalBusMap(vote_mapping, "Vote Method & TMATS");
		else
			SubmitToFinalBusMap(vote_mapping, "Vote Method");
	}

	// Exclude any residual bus names that match specified
	// bus_exclusions, match is not case sensitive 
	// and bus_exclusion matches can be substrings
	// of mapped bus names.
	for (std::map<uint64_t, std::pair<std::string, std::string>>::iterator it =
		final_bus_map_with_sources_.begin();
		it != final_bus_map_with_sources_.end();
		++it)
	{
		std::string temp = it->second.first;
		std::transform(temp.begin(),
			temp.end(),
			temp.begin(),
			::toupper);

		for (auto bus : upper_case_bus_exclusions_)
		{
			// handle exclusion bus being a substring 
			// of a final bus name
			if (temp.find(bus) != std::string::npos)
				excluded_channel_ids_[it->first] = 
				"Config Option: bus_name_exclusions";
		}		
	}

	// Exclude channel ids in the exclusion map
	for (std::map<uint64_t,std::string>::iterator it = excluded_channel_ids_.begin();
		it != excluded_channel_ids_.end();
		++it)
	{
		uint64_t channel_id = it->first;
		if (final_bus_map_with_sources_.count(channel_id) > 0)
			final_bus_map_with_sources_.erase(channel_id);
	}

	Print();

	// If prompt_user is false
	// If the final bus map has any mappings return true
	if (!prompt_user)
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
	// If prompt_user is true, let the use make adjustments
	else
	{
		
		if (UserAdjustments(user_test_input))
		{
			PrepareFinalMap();
			return true;
		}
		else
		{
			return false;
		}
	}
}

void BusMap::PrintVoteMap()
{
	// print vote map
	printf("\nChannel ID votes---\n");
	for (std::unordered_map<uint64_t, std::unordered_map<std::string, uint64_t>>::iterator
		it = votes_.begin();
		it != votes_.end();
		++it)
	{
		printf("\n");
		printf("Channel ID %i:\n", it->first);
		for (std::unordered_map<std::string, uint64_t>::iterator
			it2 = it->second.begin();
			it2 != it->second.end();
			++it2)
		{
			printf("%s=\t%i\n", it2->first.c_str(), it2->second);
		}
	}
	printf("---\n\n");
}
void BusMap::Print()
{
	PrintVoteMap();
	PrintFinalMap();
}

void BusMap::SubmitToFinalBusMap(const std::map<uint64_t,
	std::string>& insert_map,
	std::string source)
{
	// iterate over the map to add, if the channel ID does not 
	// exist yet in the final bus map, add it in with the source
	for (std::map<uint64_t, std::string>::const_iterator
		it = insert_map.begin(); it != insert_map.end(); it++)
	{
		if (final_bus_map_with_sources_.count(it->first) == 0
			&& channel_ids_.count(it->first) > 0)
		{
			final_bus_map_with_sources_[it->first] =
				std::pair<std::string, std::string>(it->second, source);
		}
	}
}