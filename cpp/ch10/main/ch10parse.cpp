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

#include "parser_helper_funcs.h"


int main(int argc, char* argv[])
{	
	
	if (argc < 2)
	{
		printf("Requires single argument, path to *.ch10 file\n");
		return 0;
	}

	ParserConfigParams config;
	ManagedPath final_conf_path;
	std::string tip_root_path = "";
	if (!ValidateConfig(config, tip_root_path, final_conf_path))
		return 0;

	if (!SetupLogging())
		return 0;
	spdlog::get("pm")->info("Configuration file path: {:s}", final_conf_path.RawString());

	ManagedPath input_path;
	ManagedPath output_path;
	char* arg2 = "";
	if (argc == 3)
		arg2 = argv[2];
	if (!ValidatePaths(argv[1], arg2, input_path, output_path))
		return 0;

	double duration;
	StartParse(input_path, output_path, config, duration);

	// Avoid deadlock in windows, see 
	// http://stackoverflow.com/questions/10915233/stdthreadjoin-hangs-if-called-after-main-exits-when-using-vs2012-rc
	spdlog::shutdown();

	return 0;
}
