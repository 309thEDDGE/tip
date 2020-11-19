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

	
	std::filesystem::path conf_path("../conf/parse_conf.yaml");
	ParserConfigParams config;
	bool settings_validated = config.Initialize(conf_path.string());

	// Get path to ch10 file. 
	std::string input_path = argv[1];
	printf("Ch10 file path: %s\n", input_path.c_str());

	// Check for a second argument. If present, this path specifies the output
	// path. If not present, the output path is the same as the input path.
	std::filesystem::path inpath(input_path);
	std::string output_path = inpath.parent_path().string();
	if (argc == 3)
	{
		// TODO: check that the output path is valid!
		output_path = argv[2];
		printf("Output path: %s\n", output_path.c_str());
	}

	if (settings_validated)
	{
		// Get start time.
		auto start_time = std::chrono::high_resolution_clock::now();

		// Initialization includes parsing of TMATS data.
		ParseManager pm(input_path, output_path, &config);
		
		if (pm.error_state())
			return 0;

		// Begin parsing of Ch10 data by starting workers.
		pm.start_workers();

		// Get stop time and print duration.
		auto stop_time = std::chrono::high_resolution_clock::now();
		printf("Duration: %.3f sec\n", (stop_time - start_time).count() / 1.0e9);
	}
	
	return 0;
}
