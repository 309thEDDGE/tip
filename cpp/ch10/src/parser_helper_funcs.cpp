#include "parser_helper_funcs.h"

bool ValidateConfig(ParserConfigParams& config, std::string config_path,
	ManagedPath& final_config_path)
{
	ManagedPath conf_path;
	if (config_path == "")
		conf_path = conf_path.parent_path() / "conf" / "parse_conf.yaml";
	else
		conf_path = ManagedPath(config_path) / "parse_conf.yaml";
	/*printf("Configuration file path: %s\n", conf_path.RawString().c_str());*/
	bool settings_validated = config.Initialize(conf_path.string());
	final_config_path = conf_path;
	return settings_validated;
}

bool ValidatePaths(char* arg1, char* arg2, ManagedPath& input_path, ManagedPath& output_path)
{
	// Get path to ch10 file. 
	std::string arg_path = arg1;
	input_path = ManagedPath(arg_path);
	if (!input_path.is_regular_file())
	{
		spdlog::get("pm_logger")->warn("User-defined input path is not a file/does not exist: {:s}",
			input_path.RawString());
		return false;
	}
	spdlog::get("pm_logger")->info("Ch10 file path: {:s}", input_path.RawString());

	// Check for a second argument. If present, this path specifies the output
	// path. If not present, the output path is the same as the input path.
	output_path = input_path.parent_path();
	if ((arg2 != NULL) && (strlen(arg2) != 0))
	{
		output_path = ManagedPath(std::string(arg2));
		if (!output_path.is_directory())
		{
			spdlog::get("pm_logger")->warn("User-defined output path is not a directory: {:s}",
				output_path.RawString());
			return false;
		}
	}
	spdlog::get("pm_logger")->info("Output path: {:s}", output_path.RawString());
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
	spdlog::get("pm_logger")->info("Duration: {:.3f} sec", duration);
	return true;
}

bool SetupLogging(const ManagedPath& log_dir)
{
	try
	{
		// Setup async thread pool.
		spdlog::init_thread_pool(8192, 2);

		// Rotating logs maxima
		int max_size = 1024 * 512; // 512 KB
		int max_files = 5; 

		// Console sink
		auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		console_sink->set_level(spdlog::level::info);
		console_sink->set_pattern("%^[%T] [%n] [%l] %v%$");

		// ParseManager log
		// automatically registered?
		ManagedPath pm_log_path = log_dir / std::string("parse_manager.log");
		auto pm_log_sink = std::make_shared<spdlog::sinks::rotating_file_sink_st>(pm_log_path.string(),
			max_size, max_files);
		pm_log_sink->set_level(spdlog::level::debug);
		pm_log_sink->set_pattern("[%D %T] [%l] %v");

		// List of sinks for ParseManager, console logger
		spdlog::sinks_init_list pm_sinks = { console_sink, pm_log_sink };

		// Create and register the logger for ParseManager log and console.
		auto pm_logger = std::make_shared<spdlog::logger>("pm_logger", pm_sinks.begin(), pm_sinks.end());
		spdlog::register_logger(pm_logger);

		// Parser primary threaded file sink.
		max_size = 1024 * 1024 * 10; // 10 MB
		max_files = 20; 
		ManagedPath parser_log_path = log_dir / std::string("parser.log");
		auto parser_log_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(parser_log_path.string(),
			max_size, max_files);
		parser_log_sink->set_level(spdlog::level::debug);
		parser_log_sink->set_pattern("[%D %T] [%l:%t] [%@] %v");

		// List of sinks for async parser, console logger
		spdlog::sinks_init_list parser_sinks = { parser_log_sink, console_sink };

		// Create and register async parser, consoler logger
		auto parser_logger = std::make_shared<spdlog::async_logger>("parser_logger", parser_sinks,
			spdlog::thread_pool(), spdlog::async_overflow_policy::block);
		spdlog::register_logger(parser_logger);

		// Register as default logger to simplify calls in ParseWorker and deeper where
		// the majority of parser logging calls will be made.
		spdlog::set_default_logger(parser_logger);

	}
	catch (const spdlog::spdlog_ex& ex)
	{
		printf("SetupLogging() failed: %s\n", ex.what());
		return false;
	}
	return true;
}