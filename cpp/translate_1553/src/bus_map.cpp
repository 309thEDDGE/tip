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
	std::map<uint64_t, std::string> tmats_chanid_to_source_map, 
	std::map<std::string, std::string> tmats_busname_corrections)
{
	tmats_chanid_to_source_map_ = tmats_chanid_to_source_map;
	channel_ids_ = channel_ids;
	mask_ = mask;

	if (tmats_chanid_to_source_map.empty())
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
		ss << "\n--Unmapped--\n";
	else
		ss << "\n--All Channel IDs Mapped--\n";

	// Print out any unmapped elements
	for (auto channel_id : channel_ids_)
	{
		if (final_bus_map_with_sources_.count(channel_id) == 0)
			ss << " " << channel_id << "\n";
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
		key = (transmit_cmd[i] << 16 ) 
			| (recieve_cmd[i]) & mask_;
		if (icd_message_key_to_channelids_map_.count(key) == 1)
		{
			icd_message_key_to_channelids_map_[key].insert(channel_ids[i]);
		}
	}
	return true;
}


std::map<uint64_t, std::string> BusMap::VoteMapping()
{
	std::map<uint64_t, std::string> vote_map;

	// map of channel id -> busname -> vote count
	std::unordered_map<uint64_t, std::unordered_map<std::string, uint64_t>> votes;

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
			if (votes.count(channel_id) == 0)
			{
				for (auto bus : unique_buses_)
				{
					votes[channel_id][bus] = 0;
				}				
			}

			for (auto bus : icd_message_key_to_busnames_map_[it->first])
			{
				++votes[channel_id][bus];
			}
		}
	}

	// print vote map
	printf("\nChannel ID votes---\n");
	for (std::unordered_map<uint64_t, std::unordered_map<std::string, uint64_t>>::iterator
		it = votes.begin();
		it != votes.end();
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

	// assign a bus to a channel id if it has the most votes
	// but not if it has zero votes, and not if the number of
	// votes is the same as another bus
	int highest_vote = 0;
	int find_count = 0;
	std::string highest_voted_bus = "";
	for (std::unordered_map<uint64_t, std::unordered_map<std::string, uint64_t>>::iterator
		it = votes.begin();
		it != votes.end();
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

		if (highest_vote != 0 && find_count == 1)
			vote_map[it->first] = highest_voted_bus;

	}

	return vote_map;
}


bool BusMap::Finalize(std::map<uint64_t, std::string>& final_map,
	bool use_tmats_busmap,
	bool user_input_if_not_complete,
	std::vector<std::string>* user_test_input)
{
	final_map_ptr_ = &final_map;

	// Clear anything already in the final map
	final_map.clear();

	// vote mapping
	std::map<uint64_t, std::string> vote_mapping = VoteMapping();

	// Print out maps
	printf("\nChannel IDs to Map----\n");
	for (auto channel_id : channel_ids_)
	{
		printf("%i\n", channel_id);
	}
	printf("----\n");

	// Only print tmats channel ids that are in the 
	// channel id master list
	std::map<uint64_t, std::string> tmats_print_map;
	for (std::map<uint64_t, std::string>::iterator it =
		tmats_chanid_to_source_map_.begin();
		it != tmats_chanid_to_source_map_.end();
		++it)
	{
		// if the tmats channel id exists in the set of 
		// channel ids provided by initialize maps,
		// add the channel id to the tmats print map
		if (channel_ids_.count(it->first) == 1)
		{
			tmats_print_map[it->first] = it->second;
		}
	}

	if (tmats_print_map.size() > 0)
		iterable_tools_.PrintMapWithHeader_KeyToValue<uint64_t, std::string>
		(tmats_print_map,
			std::vector<std::string>({ "chID", "Bus" }),
			"TMATS Map");


	if (use_tmats_busmap)
	{
		SubmitToFinalBusMap(tmats_chanid_to_source_map_, "TMATS");
	}
	else
	{
		SubmitToFinalBusMap(vote_mapping, "Vote Method");
	}

	// Print Final Bus Map
	PrintFinalMap();

	// If final map contains all channel IDS return true
	if (iterable_tools_.GetKeys(final_bus_map_with_sources_).size() ==
		channel_ids_.size() &&
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