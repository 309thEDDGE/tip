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
#include "path_manager.h"

int main(int argc, char* argv[])
{	
	/*
	Parse Settings and Configuration

	Todo: pass config file parser object to ParseManager
	*/

	if (argc < 2)
	{
		printf("Requires single argument, path to *.ch10 file\n");
		return 0;
	}

	PathManager conf_path;
	ParserConfigParams config;
	bool settings_validated = config.Initialize(conf_path.Parent().Join(
		"conf/parse_conf.yaml").AsString());

	// Get path to ch10 file. 
	std::string arg_path = argv[1];
	PathManager input_path(arg_path);
	if (!input_path.IsFile())
	{
		printf("User-defined input path is not a directory: %s\n", input_path.AsString().c_str());
		return 0;
	}
	std::string ch10_path = input_path.AsString();
	printf("Ch10 file path: %s\n", ch10_path.c_str());

	// Check for a second argument. If present, this path specifies the output
	// path. If not present, the output path is the same as the input path.
	PathManager output_path = input_path.Parent();
	if (argc == 3)
	{
		output_path = PathManager(std::string(argv[2]));
		if (!output_path.IsDirectory())
		{
			printf("User-defined output path is not a directory: %s\n", output_path.AsString().c_str());
			return 0;
		}
	}
	std::string output_dir = output_path.AsString();
	printf("Output path: %s\n", output_dir.c_str());

	if (settings_validated)
	{
		// Get start time.
		auto start_time = std::chrono::high_resolution_clock::now();

		// Initialization includes parsing of TMATS data.
		ParseManager pm(ch10_path, output_dir, &config);
		
		if (pm.error_state())
			return 0;

		// Begin parsing of Ch10 data by starting workers.
		pm.start_workers();

		// Get stop time and print duration.
		auto stop_time = std::chrono::high_resolution_clock::now();
		printf("Duration: %zd sec\n", std::chrono::duration_cast<std::chrono::seconds>(stop_time - start_time).count());
	}
	
	return 0;
}
