// ch10_packet_stats.cpp

#include "ch10_milstd1553f1stats.h"

void Ch10MilStd1553F1Stats::add_msg(char* msg_name)
{
	// Convert msg_name to string.
	temp_name = std::string(msg_name);

	// Check if msg_name is present in the unique_msg_names vector.
	str_itr = std::find(unique_msg_names.begin(), unique_msg_names.end(), temp_name);

	// If not present, append to vector and create new map element with the 
	// initial count set to 1. Otherwise, increment the value of the mapped
	// element.
	if (str_itr == unique_msg_names.end())
	{
		unique_msg_names.push_back(temp_name);
		milstd_1553_msg_count.insert(std::pair<std::string, uint32_t>(temp_name, 1));
	}
	else
	{
		milstd_1553_msg_count[temp_name]++;
	}
}

void Ch10MilStd1553F1Stats::add_msg(std::string msg_name, uint32_t count)
{
	// Check if msg_name is present in the unique_msg_names vector.
	str_itr = std::find(unique_msg_names.begin(), unique_msg_names.end(), msg_name);

	// If not present, append to vector and create new map element with the 
	// initial count set to 1. Otherwise, increment the value of the mapped
	// element.
	if (str_itr == unique_msg_names.end())
	{
		unique_msg_names.push_back(msg_name);
		milstd_1553_msg_count.insert(std::pair<std::string, uint32_t>(msg_name, count));
	}
	else
	{
		milstd_1553_msg_count[msg_name] += count;
	}
}

void Ch10MilStd1553F1Stats::print_msg_stats()
{
	// Sort unique messages vector.
	std::sort(unique_msg_names.begin(), unique_msg_names.end());

	uint64_t total_msgs = 0;
	uint32_t temp_val = 0;
	std::string temp_msg_name = "";
	printf("%zu unique message types\n", unique_msg_names.size());
	for (size_t i = 0; i < unique_msg_names.size(); i++)
	{
		temp_msg_name = unique_msg_names[i];
		temp_val = milstd_1553_msg_count[temp_msg_name];
		printf("%15s\t: %05u\n", temp_msg_name.c_str(), temp_val);
		total_msgs += temp_val;
	}
	printf("Total          \t: %05llu\n", total_msgs);
}

std::vector<std::string>& Ch10MilStd1553F1Stats::get_msg_names()
{
	return unique_msg_names;
}

std::map<std::string, uint32_t>& Ch10MilStd1553F1Stats::get_msg_count()
{
	return milstd_1553_msg_count;
}

void Ch10MilStd1553F1Stats::add_stats(Ch10MilStd1553F1Stats* new_stats)
{
	msg_identity_fail += new_stats->msg_identity_fail;
	wrd_count_mismatch += new_stats->wrd_count_mismatch;
	ts_not_impl += new_stats->ts_not_impl;
	ts_reserved += new_stats->ts_reserved;

	// Loop over all message types in new_stats and either add
	// the message count if present, or create a new map entry with
	// the new_stats count value.
	std::vector<std::string> new_msg_names = new_stats->get_msg_names();
	std::map<std::string, uint32_t> new_msg_count = new_stats->get_msg_count();
	for (size_t i = 0; i < new_msg_names.size(); i++)
	{
		add_msg(new_msg_names[i], new_msg_count[new_msg_names[i]]);
	}
}