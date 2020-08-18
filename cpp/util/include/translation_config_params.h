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
	int	bus_map_confidence_level_;
	std::map<std::string, std::string> tmats_busname_corrections_;
	std::vector<std::string> select_specific_messages_;
	bool exit_after_table_creation_;
	int comet_debug_;
	bool stop_after_bus_map_;
	bool prompt_user_;
	int translate_thread_count_;
	std::map<std::string, std::set<uint64_t>> comet_busmap_replacement_;


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
		success.insert(yr.GetParams("bus_map_confidence_level", 
			bus_map_confidence_level_, 1, 4, true));

		success.insert(yr.GetParams("tmats_busname_corrections", 
			tmats_busname_corrections_, true));

		success.insert(yr.GetParams("select_specific_messages", 
			select_specific_messages_, true));

		success.insert(yr.GetParams("exit_after_table_creation", 
			exit_after_table_creation_, true));

		success.insert(yr.GetParams("comet_debug", comet_debug_, 
			0, INT_MAX, true));

		success.insert(yr.GetParams("stop_after_bus_map", 
			stop_after_bus_map_, true));

		success.insert(yr.GetParams("prompt_user", 
			prompt_user_, true));

		success.insert(yr.GetParams("comet_busmap_replacement", 
			inter_comet_busmap_replacement, true));

		success.insert(yr.GetParams("translate_thread_count", 
			translate_thread_count_, 1, (int)(std::thread::hardware_concurrency() * 2), true));

		// If one config option was not read correctly return false
		if (success.find(false) != success.end())
			return false;

		// Specific checks on intermediate types
		
		// comet_busmap_replacement
		for (std::map<std::string, std::vector<int>>::iterator it = inter_comet_busmap_replacement.begin();
			it != inter_comet_busmap_replacement.end();
			++it)
		{
			for (int i = 0; i < it->second.size(); i++)
			{
				// bus map must be positive
				if (it->second[i] < 0)
				{
					printf("\ncomet_busmap_replacement terminal address (%i) for %s is negative\n",
						it->second[i], it->first);
					return false;
				}
			}
		}

		// Adjust comet_busmap_replacement to final datatype std::map<std::string, std::set<uint64_t>>::iterator
		for (std::map<std::string, std::vector<int>>::iterator it = inter_comet_busmap_replacement.begin();
			it != inter_comet_busmap_replacement.end();
			++it)
		{
			comet_busmap_replacement_[it->first] = std::set<uint64_t>(it->second.begin(), it->second.end());
		}
		return true;
	}
};

#endif

