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
	std::string tip_root_path = "";
	std::string config_schema_path = "";
	if (!ValidateConfig(config, tip_root_path, config_schema_path))
		return 0;

	// Get the parent dir of the parent dir of the final configuration file
	// path. In the future, it may be useful to create a function which 
	// returns the log output path. It could take the config from yaml 
	// and the final_conf_path as arguments. The options may include using
	// a path taken from the yaml config or using a traditional directory 
	// such as %LOCALAPPDATA% in windows or /home/<user>/.tip/logs or similar
	// or to create a log dir in the same directory as the conf directory,
	// which can be determined from final_conf_path, as is the only option
	// used now.
	ManagedPath log_dir = final_conf_path.parent_path().parent_path();
	log_dir /= std::string("logs");

	if (!SetupLogging(log_dir))
		return 0;
	spdlog::get("pm_logger")->info("Configuration file path: {:s}", final_conf_path.RawString());
	spdlog::get("pm_logger")->info("Log directory: {:s}", log_dir.RawString());

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
