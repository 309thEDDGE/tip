#include "parser_helper_funcs.h"

bool ValidateConfig(ParserConfigParams& config, std::string tip_root_path)
{
	ManagedPath conf_path;
	if (tip_root_path == "")
		conf_path = conf_path.parent_path() / "conf" / "parse_conf.yaml";
	else
		conf_path = ManagedPath(tip_root_path) / "conf" / "parse_conf.yaml";
	printf("Configuration file path: %s\n", conf_path.RawString().c_str());
	bool settings_validated = config.Initialize(conf_path.string());
	return settings_validated;
}

bool ValidatePaths(char* arg1, char* arg2, ManagedPath& input_path, ManagedPath& output_path)
{
	// Get path to ch10 file. 
	std::string arg_path = arg1;
	input_path = ManagedPath(arg_path);
	if (!input_path.is_regular_file())
	{
		printf("User-defined input path is not a directory: %s\n",
			input_path.RawString().c_str());
		return false;
	}
	printf("Ch10 file path: %s\n", input_path.RawString().c_str());

	// Check for a second argument. If present, this path specifies the output
	// path. If not present, the output path is the same as the input path.
	output_path = input_path.parent_path();
	if ((arg2 != NULL) && (*arg2 != 0))
	{
		output_path = ManagedPath(std::string(arg2));
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
	ParserConfigParams config)
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
	printf("Duration: %.3f sec\n", (stop_time - start_time).count() / 1.0e9);
	return true;
}