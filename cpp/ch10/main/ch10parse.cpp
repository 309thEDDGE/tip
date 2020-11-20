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

	ParserConfigParams config;
	ManagedPath conf_path;
	printf("before assignment\n");
	conf_path = conf_path.parent_path() / 
		ManagedPath(std::string("conf/parse_conf.yaml"));
	printf("Configuration file path: %s\n", conf_path.RawString().c_str());
	//bool settings_validated = config.Initialize(conf_path.string());

	bool settings_validated = true;
	printf("before return\n");
	return 0;

	// Get path to ch10 file. 
	std::string arg_path = argv[1];
	ManagedPath input_path(arg_path);
	if (!input_path.is_regular_file())
	{
		printf("User-defined input path is not a directory: %s\n", input_path.RawString().c_str());
		return 0;
	}
	std::string ch10_path = input_path.string();
	printf("Ch10 file path: %s\n", ch10_path.c_str());

	// Check for a second argument. If present, this path specifies the output
	// path. If not present, the output path is the same as the input path.
	ManagedPath output_path = input_path.parent_path();
	if (argc == 3)
	{
		output_path = ManagedPath(std::string(argv[2]));
		if (!output_path.is_directory())
		{
			printf("User-defined output path is not a directory: %s\n", output_path.RawString().c_str());
			return 0;
		}
	}
	std::string output_dir = output_path.string();
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
