#ifndef TRANSLATION_CONFIG_PARAMS_H
#define TRANSLATION_CONFIG_PARAMS_H

#include <string>
#include <map>
#include <set>
#include <vector>
#include <thread>
#include "yaml_reader.h"

class TranslationConfigParams
{
public:
	

	// Parameters (refer to translate_conf.yaml for more detail)
	bool use_tmats_busmap_;
	std::map<std::string, std::string> tmats_busname_corrections_;
	std::vector<std::string> select_specific_messages_;
	bool exit_after_table_creation_;
	bool stop_after_bus_map_;
	int vote_threshold_;
	bool prompt_user_;
	int translate_thread_count_;
	std::vector<std::string> bus_exclusions_;


	bool Initialize(std::string file_path)
	{
		YamlReader yr;
		if (!yr.LinkFile(file_path))
		{
			return false;
		}
		else
		{
			printf("\nConfig file found %s\n", file_path.c_str());
		}

		// Intermediate data types
		std::map<std::string, std::vector<int>> inter_comet_busmap_replacement;

		std::set<bool> success;

		// Add one parameter at a time
		success.insert(yr.GetParams("use_tmats_busmap", 
			use_tmats_busmap_, true));

		success.insert(yr.GetParams("tmats_busname_corrections", 
			tmats_busname_corrections_, true));

		success.insert(yr.GetParams("select_specific_messages", 
			select_specific_messages_, true));

		success.insert(yr.GetParams("exit_after_table_creation", 
			exit_after_table_creation_, true));

		success.insert(yr.GetParams("stop_after_bus_map", 
			stop_after_bus_map_, true));

		success.insert(yr.GetParams("vote_threshold",
			vote_threshold_, 1, INT_MAX, true));

		success.insert(yr.GetParams("bus_name_exclusions",
			bus_exclusions_, true));

		success.insert(yr.GetParams("prompt_user", 
			prompt_user_, true));

		success.insert(yr.GetParams("translate_thread_count", 
			translate_thread_count_, 1, (int)(std::thread::hardware_concurrency() * 2), true));

		// If one config option was not read correctly return false
		if (success.find(false) != success.end())
			return false;

		return true;
	}
};

#endif

