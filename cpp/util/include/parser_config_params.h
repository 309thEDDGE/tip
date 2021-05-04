#ifndef PARSER_CONFIG_PARAMS_H
#define PARSER_CONFIG_PARAMS_H

#include "yaml_reader.h"
#include <climits>
#include <string>
#include <thread>
#include <map>

class ParserConfigParams
{
public:
	// Parameters (refer to parse_conf.yaml for more detail)
	std::map<std::string, std::string> ch10_packet_type_map_;
	int parse_chunk_bytes_;
	int	parse_thread_count_;
	int	max_chunk_read_count_;
	int	worker_offset_wait_ms_;
	int	worker_shift_wait_ms_;

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

		// Add one parameter at a time with boundary conditions if required

		// TMATS maximum packet size is 134.22 MB. Set the minimum chunk size
		// to slightly bigger than the tmats packet so it doesn't span multiple
		// workers.
		success.insert(yr.GetParams("ch10_packet_type", ch10_packet_type_map_, true));
		success.insert(yr.GetParams("parse_chunk_bytes", parse_chunk_bytes_, 135, 1000, true));
		success.insert(yr.GetParams("parse_thread_count", parse_thread_count_, 1, (int)(std::thread::hardware_concurrency() * 1.5), true));
		success.insert(yr.GetParams("max_chunk_read_count", max_chunk_read_count_, 1, INT_MAX, true));
		success.insert(yr.GetParams("worker_offset_wait_ms", worker_offset_wait_ms_, 1, INT_MAX, true));
		success.insert(yr.GetParams("worker_shift_wait_ms", worker_shift_wait_ms_, 1, INT_MAX, true));

		// If one config option was not read correctly return false
		if (success.find(false) != success.end())
			return false;
		else
			return true;
	}

	bool Initialize(std::string file_path)
	{
		YamlReader yr;

		// If the file is bad return
		if (!yr.LinkFile(file_path))
		{
			return false;
		}

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
