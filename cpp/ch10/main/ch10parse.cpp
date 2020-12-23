// ch10parse.h

// Require logical/reasonable preprocessor definitions.

#ifndef PARQUET
#error PARQUET must be defined!
#endif

#ifdef XDAT
#ifndef LOCALDB
#error XDAT can only be defined in conjunction with LOCALDB
#endif
#endif

#ifdef PARQUET
#ifndef ARROW_STATIC
#error ARROW_STATIC must be defined in conjunction with PARQUET
#endif
#ifndef PARQUET_STATIC
#error PARQUET_STATIC must be defined in conjunction with PARQUET
#endif
#endif

#include "parse_manager.h"
#include "parser_config_params.h"
#include "managed_path.h"

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

int main(int argc, char* argv[])
{	
	
	if (argc < 2)
	{
		printf("Requires single argument, path to *.ch10 file\n");
		return 0;
	}

	ParserConfigParams config;
	std::string tip_root_path = "";
	if (!ValidateConfig(config, tip_root_path))
		return 0;

	ManagedPath input_path;
	ManagedPath output_path;
	char* arg2 = "";
	if (argc == 3)
		arg2 = argv[2];
	if (!ValidatePaths(argv[1], arg2, input_path, output_path))
		return 0;

	StartParse(input_path, output_path, config);
	return 0;
}

extern "C"
{
	int RunParser(char* input_path, char* output_path, char* tip_path)
	{
		ParserConfigParams config;
		if (!ValidateConfig(config, tip_path))
			return 1;

		ManagedPath mp_input_path;
		ManagedPath mp_output_path;
		if (!ValidatePaths(input_path, output_path, mp_input_path, mp_output_path))
			return 1;

		if(!StartParse(mp_input_path, mp_output_path, config))
			return 1;

		return 0;
	}
}