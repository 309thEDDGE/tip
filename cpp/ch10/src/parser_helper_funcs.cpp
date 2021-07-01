#include "parser_helper_funcs.h"

bool ParseArgs(int argc, char* argv[], std::string& str_input_path, std::string& str_output_path,
	std::string& str_conf_dir, std::string& str_log_dir)

{
	if(argc < 2)
	{
		printf("Usage: tip_parse <ch10 path> [output path] [config dir] [log dir]\n"
			   "Needs input path.\n");
		return false;
	}
	str_input_path = std::string(argv[1]);

	str_output_path = "";
	if(argc < 3) return true;
	str_output_path = std::string(argv[2]);

	str_conf_dir = "";
	if(argc < 4) return true;
	str_conf_dir = std::string(argv[3]);

	str_log_dir = "";
	if(argc < 5) return true;
	str_log_dir = std::string(argv[4]);

	return true;
}

bool ValidatePaths(const std::string& str_input_path, const std::string& str_output_path,
	const std::string& str_conf_dir, const std::string& str_log_dir, 
	ManagedPath& input_path, ManagedPath& output_path,
	ManagedPath& conf_file_path, ManagedPath& schema_file_path, ManagedPath& log_dir)
{
	ArgumentValidation av;

	if (!av.CheckExtension(str_input_path, "ch10", "c10"))
	{
		printf("User-defined input path does not have one of the case-insensitive "
			"extension: ch10, c10\n");
		return false;
	} 
	// Check utf-8 conformity and verify existence of input path
	if (!av.ValidateInputFilePath(str_input_path, input_path))
	{
		printf("User-defined input path is not a file/does not exist: %s\n",
			str_input_path.c_str());
		return false;
	}

	// If no output path is specified, use the input path.
	if (str_output_path == "")
	{
		output_path = input_path.absolute().parent_path();
	}
	else
	{
		// Check if the path conforms to utf-8 and exists
		if (!av.ValidateDirectoryPath(str_output_path, output_path))
		{
			printf("User-defined output path is not a directory: %s\n",
				str_output_path.c_str());
			return false;
		}
	}

	// Create parse configuration path from base conf path 
	std::string conf_file_name = "parse_conf.yaml";
	ManagedPath default_conf_base_path({ "..", "conf" });
	if (!av.ValidateDefaultInputFilePath(default_conf_base_path.absolute(), 
		str_conf_dir, conf_file_name, conf_file_path))
	{
		printf("Failed to create parser configuration file path\n");
		return false;
	}

	// Construct the schema path. The user schema path is constructed from
	// the user conf path only if the user conf path is not an empty string.
	std::string schema_file_name = "tip_parse_conf_schema.yaml";
	ManagedPath default_schema_path({ "..", "conf", "yaml_schemas" });
	ManagedPath user_schema_path(std::string(""));
	if(str_conf_path != "")
		user_schema_path = ManagedPath({str_conf_path, "yaml_schemas"});
	if (!av.ValidateDefaultInputFilePath(default_schema_path, user_schema_path.RawString(),
		schema_file_name, schema_file_path))
	{
		printf("Failed to create yaml schema file path\n");
		return false;
	}

	if(!ValidateLogDir(str_log_dir, log_dir)) return false;

	return true;
}

bool ValidateConfig(ParserConfigParams& config, const ManagedPath& conf_file_path,
	const ManagedPath& schema_file_path)
{
	ArgumentValidation av;

	std::string schema_doc;
	if (!av.ValidateDocument(schema_file_path, schema_doc)) return false;

	std::string config_doc;
	if (!av.ValidateDocument(conf_file_path, config_doc)) return false;

	// Validate configuration file using yaml schema
	YamlSV yamlsv;
	std::vector<LogItem> log_items;
	if (!yamlsv.Validate(config_doc, schema_doc, log_items))
	{
		printf("Failed to validate config file (%s) against schema (%s)\n",
			conf_file_path.RawString().c_str(), schema_file_path.RawString().c_str());

		// Print log items which may contain information about which yaml elements
		// do not conform to schema.
		for(std::vector<LogItem>::const_iterator it = log_items.cbegin(); 
			it != log_items.cend(); ++it)
		{
			if (static_cast<uint8_t>(it->log_level) > static_cast<uint8_t>(LogLevel::Debug))
				it->Print();		
		}
		return false;
	}

	bool settings_validated = config.InitializeWithConfigString(config_doc);
	return settings_validated;
}

bool StartParse(ManagedPath input_path, ManagedPath output_path,
	ParserConfigParams config, double& duration)
{
	// Get start time.
	auto start_time = std::chrono::high_resolution_clock::now();

	// Configure checks configuration, prepares output paths,
	// and calculates internal quantities in preparation for
	// parsing.
	ParseManager pm;
	if (!pm.Configure(input_path, output_path, config))
		return false;

	// Begin parsing of Ch10 data by starting workers.
	if (!pm.Parse(config))
		return false;

	// Record metadata
	if (!pm.RecordMetadata(input_path, config))
		return false;

	// Get stop time and print duration.
	auto stop_time = std::chrono::high_resolution_clock::now();
	duration = (stop_time - start_time).count() / 1.0e9;
	
	return true;
}

bool ValidateLogDir(const std::string& user_log_dir, ManagedPath& final_log_dir)
{
	// Note: In later iterations, this function may receive optional 
	// arguments which instruct it to use the %LOCALAPPDIR% (or whatever the
	// default is in windows) or the linux equivalent.

	// Set up the default logging directory, relative to assumed bin dir.
	ManagedPath default_log_dir({ "..", "logs" });

	// Create the final log dir object, using the value of user_log_dir
	// if it is not an empty string.
	ArgumentValidation av;
	if (!av.ValidateDefaultOutputDirectory(default_log_dir, user_log_dir,
		final_log_dir, true))
		return false;

	return true;
}

bool SetupLogging(const ManagedPath& log_dir)
{
	// Use the chart on this page for logging level reference:
	// https://www.tutorialspoint.com/log4j/log4j_logging_levels.htm

	try
	{
		// Set global logging level
		spdlog::set_level(spdlog::level::debug);

		// Setup async thread pool.
		spdlog::init_thread_pool(8192, 2);

		// Rotating logs maxima
		int max_size = 1024 * 512; // 512 KB
		int max_files = 5; 

		// Console sink
		auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		console_sink->set_level(spdlog::level::info);
		//console_sink->set_level(spdlog::level::debug);
		console_sink->set_pattern("%^[%T] [%n] [%l] %v%$");

		// ParseManager log
		ManagedPath pm_log_path = log_dir / std::string("parse_manager.log");
		auto pm_log_sink = std::make_shared<spdlog::sinks::rotating_file_sink_st>(pm_log_path.string(),
			max_size, max_files);
		pm_log_sink->set_level(spdlog::level::debug);
		pm_log_sink->set_pattern("[%D %T] [%l] %v");

		// List of sinks for ParseManager, console logger
		spdlog::sinks_init_list pm_sinks = { console_sink, pm_log_sink };

		// Create and register the logger for ParseManager log and console.
		auto pm_logger = std::make_shared<spdlog::logger>("pm_logger", pm_sinks.begin(), pm_sinks.end());
		pm_logger->set_level(spdlog::level::debug);
		spdlog::register_logger(pm_logger);

		// Parser primary threaded file sink.
		max_size = 1024 * 1024 * 10; // 10 MB
		max_files = 20; 
		ManagedPath parser_log_path = log_dir / std::string("parser.log");
		auto parser_log_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(parser_log_path.string(),
			max_size, max_files);
		parser_log_sink->set_level(spdlog::level::debug);
		parser_log_sink->set_pattern("[%D %T] [%L] [%@] %v");

		// List of sinks for async parser, console logger
		spdlog::sinks_init_list parser_sinks = { parser_log_sink, console_sink };

		// Create and register async parser, consoler logger
		auto parser_logger = std::make_shared<spdlog::async_logger>("parser_logger", parser_sinks,
			spdlog::thread_pool(), spdlog::async_overflow_policy::block);
		parser_logger->set_level(spdlog::level::debug);
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