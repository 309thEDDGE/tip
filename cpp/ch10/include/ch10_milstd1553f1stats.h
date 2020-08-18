#ifndef CH10MILSTD1553F1STATS_H
#define CH10MILSTD1553F1STATS_H

#include <cstdint>
#include <vector>
#include <string>
#include <map>
#include <algorithm>

class Ch10MilStd1553F1Stats
{
private:
	std::map<std::string, uint32_t> milstd_1553_msg_count;
	std::vector<std::string> unique_msg_names;
	std::vector<std::string>::iterator str_itr;
	std::string temp_name;

public:
	uint32_t msg_identity_fail;
	uint32_t wrd_count_mismatch;
	uint32_t ts_not_impl;
	uint32_t ts_reserved;
	Ch10MilStd1553F1Stats() : msg_identity_fail(0), wrd_count_mismatch(0),
		ts_not_impl(0), ts_reserved(0) { }

	void add_msg(char* msg_name);
	void add_msg(std::string msg_name, uint32_t count);
	void print_msg_stats();
	void add_stats(Ch10MilStd1553F1Stats* new_stats);
	std::vector<std::string>& get_msg_names();
	std::map<std::string, uint32_t>& get_msg_count();
};

#endif 