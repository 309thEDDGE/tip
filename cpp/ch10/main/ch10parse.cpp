// ch10parse.h

#ifndef ARROW_STATIC
#error ARROW_STATIC must be defined in conjunction with PARQUET
#endif

#ifndef PARQUET_STATIC
#error PARQUET_STATIC must be defined in conjunction with PARQUET
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
	ManagedPath final_config_path;
	ManagedPath final_schema_path;
	if (!ValidateConfig(config, tip_root_path, config_schema_path, final_config_path,
		final_schema_path))
		return 0;

	std::string user_log_dir = "";
	ManagedPath log_dir;
	if (!ValidateLogDir(user_log_dir, log_dir))
		return 0;

	if (!SetupLogging(log_dir))
		return 0;

	ManagedPath input_path;
	ManagedPath output_path;
	char* arg2 = "";
	if (argc == 3)
		arg2 = argv[2];
	if (!ValidatePaths(argv[1], arg2, input_path, output_path))
		return 0;

	spdlog::get("pm_logger")->info("Ch10 file path: {:s}", input_path.RawString());
	spdlog::get("pm_logger")->info("Output path: {:s}", output_path.RawString());
	spdlog::get("pm_logger")->info("Configuration file path: {:s}",
		final_config_path.RawString());
	spdlog::get("pm_logger")->info("Configuration schema path: {:s}",
		final_schema_path.RawString());
	spdlog::get("pm_logger")->info("Log directory: {:s}", log_dir.RawString());

	double duration;
	StartParse(input_path, output_path, config, duration);
	spdlog::get("pm_logger")->info("Duration: {:.3f} sec", duration);

	// Avoid deadlock in windows, see 
	// http://stackoverflow.com/questions/10915233/stdthreadjoin-hangs-if-called-after-main-exits-when-using-vs2012-rc
	spdlog::shutdown();

	return 0;
}
