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
    bool vote_method_checks_tmats_;
    bool auto_sys_limits_;
    int translate_thread_count_;
    std::vector<std::string> bus_name_exclusions_;

    /*
	Attempt to read the required parameters from the
	yaml object. This function is tested though the
	Initialize function.

	Args:
		yr	--> YamlReader already initialized with yaml data

	Return:
	True if all values are read with the proper data types. False otherwise.
	*/
    bool ValidateConfigParams(YamlReader& yr)
    {
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

        success.insert(yr.GetParams("vote_method_checks_tmats",
                                    vote_method_checks_tmats_, true));

        success.insert(yr.GetParams("vote_threshold",
                                    vote_threshold_, 1, INT_MAX, true));

        success.insert(yr.GetParams("bus_name_exclusions",
                                    bus_name_exclusions_, true));

        success.insert(yr.GetParams("prompt_user",
                                    prompt_user_, true));

        success.insert(yr.GetParams("translate_thread_count",
                                    translate_thread_count_, 1, static_cast<int>(std::thread::hardware_concurrency() * 2), true));

        success.insert(yr.GetParams("auto_sys_limits",
                                    auto_sys_limits_, true));

        // If one config option was not read correctly return false
        if (success.find(false) != success.end())
            return false;

        return true;
    }

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
        //std::map<std::string, std::vector<int>> inter_comet_busmap_replacement;

        return ValidateConfigParams(yr);
    }

    /*
	Ingest yaml matter as a string instead of reading from a file,
	then validate the params. An alternate to Initialize().

	Args:
		yaml_matter	--> Input string with yaml content

	Return:
		True if input string could be interpreted as yaml and
		the yaml content contained all of the required parameters.
		False otherwise.
	*/
    bool InitializeWithConfigString(const std::string& yaml_matter)
    {
        YamlReader yr;
        if (!yr.IngestYamlAsString(yaml_matter))
            return false;

        return ValidateConfigParams(yr);
    }
};

#endif
