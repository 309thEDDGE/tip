#include "parser_helper_funcs.h"

bool ValidateConfig(ParserConfigParams& config, std::string config_path)
{
	ManagedPath conf_path;
	if (config_path == "")
		conf_path = conf_path.parent_path() / "conf" / "parse_conf.yaml";
	else
	{
		// Confirm that the user-input config_path is valid utf-8
		ParseText pt;
		if (!pt.IsUTF8(config_path))
		{
			printf("User-input configuration base path is not valid UTF-8\n");
			return false;
		}
		conf_path = ManagedPath(config_path) / "parse_conf.yaml";
	}

	// Confirm that configuration path exists.
	if (!conf_path.is_regular_file())
	{
		printf("Configuration file path (%s) does not exist or is not a file\n",
			conf_path.RawString().c_str());
		return false;
	}

	printf("Configuration file path: %s\n", conf_path.RawString().c_str());
	bool settings_validated = config.Initialize(conf_path.string());
	return settings_validated;
}

bool ValidatePaths(char* arg1, char* arg2, ManagedPath& input_path, ManagedPath& output_path)
{
	// Get path to ch10 file. 
	std::string ch10_path = arg1;

	// Exit if the path is not UTF-8
	ParseText pt;
	if (!pt.IsUTF8(ch10_path))
	{
		printf("Ch10 path does not conform to utf-8\n");
		return false;
	}

	input_path = ManagedPath(ch10_path);
	if (!input_path.is_regular_file())
	{
		printf("User-defined input path is not a file/does not exist: %s\n",
			input_path.RawString().c_str());
		return false;
	}
	printf("Ch10 file path: %s\n", input_path.RawString().c_str());

	// Check for a second argument. If present, this path specifies the output
	// path. If not present, the output path is the same as the input path.
	output_path = input_path.parent_path();
	if ((arg2 != NULL) && (strlen(arg2) != 0))
	{
		// Check if the path conforms to utf-8
		std::string temp_out_path = arg2;
		if (!pt.IsUTF8(temp_out_path))
		{
			printf("Ouput path does not conform to utf-8\n");
			return false;
		}
		output_path = ManagedPath(temp_out_path);
		if (!output_path.is_directory())
		{
			printf("User-defined output path is not a directory: %s\n",
				output_path.RawString().c_str());
			return false;
		}
	}
	printf("Output path: %s\n", output_path.RawString().c_str());
	return true;
}

bool StartParse(ManagedPath input_path, ManagedPath output_path,
	ParserConfigParams config, double& duration)
{
	// Get start time.
	auto start_time = std::chrono::high_resolution_clock::now();

	// Initialization includes parsing of TMATS data.
	ParseManager pm(input_path, output_path, &config);

	if (pm.error_state())
		return false;

	// Begin parsing of Ch10 data by starting workers.
	pm.start_workers();

	// Get stop time and print duration.
	auto stop_time = std::chrono::high_resolution_clock::now();
	duration = (stop_time - start_time).count() / 1.0e9;
	printf("Duration: %.3f sec\n", duration);
	return true;
}