#include "parser_helper_funcs.h"

bool ValidateConfig(ParserConfigParams& config, std::string config_path,
	std::string config_schema_path, ManagedPath& final_config_path,
	ManagedPath& final_schema_path)
{
	final_config_path = ManagedPath(std::string(""));
	final_schema_path = ManagedPath(std::string(""));

	ArgumentValidation av;
	std::string conf_file_name = "parse_conf.yaml";

	// Get the current workding directory, then the parent path / "conf"
	ManagedPath default_conf_base_path({ "..", "conf" });
	ManagedPath conf_path;
	if (!av.ValidateDefaultInputFilePath(default_conf_base_path, config_path,
		conf_file_name, conf_path))
	{
		printf("Failed to create parser configuration file path\n");
		return false;
	}
	printf("Configuration file path: %s\n", conf_path.RawString().c_str());

	// Construct the schema path.
	std::string schema_file_name = "tip_parse_conf_schema.yaml";
	ManagedPath default_schema_path({ "..", "conf", "yaml_schemas" });
	ManagedPath schema_path;
	if (!av.ValidateDefaultInputFilePath(default_schema_path, config_schema_path,
		schema_file_name, schema_path))
	{
		printf("Failed to create yaml schema file path\n");
		return false;
	}

	std::string schema_doc;
	if (!av.ValidateDocument(schema_path, schema_doc)) return false;

	std::string config_doc;
	if (!av.ValidateDocument(conf_path, config_doc)) return false;

	// Validate configuration file using yaml schema
	YamlSV yamlsv;
	std::vector<LogItem> log_items;
	if (!yamlsv.Validate(config_doc, schema_doc, log_items))
	{
		printf("Failed to validate config file (%s) against schema (%s)\n",
			conf_path.RawString().c_str(), schema_path.RawString().c_str());
		return false;
	}

	bool settings_validated = config.InitializeWithConfigString(config_doc);
	final_config_path = conf_path;
	final_schema_path = schema_path;
	return settings_validated;
}

bool ValidatePaths(char* arg1, char* arg2, ManagedPath& input_path, ManagedPath& output_path)
{
	ArgumentValidation av;

	// Get path to ch10 file. 
	std::string ch10_path = arg1;
	if (!av.CheckExtension(ch10_path, "ch10", "c10")) return false;
	if (!av.ValidateInputFilePath(ch10_path, input_path))
	{
		printf("User-defined input path is not a file/does not exist: %s\n",
			ch10_path.c_str());
		return false;
	}
	
	// Check for a second argument. If present, this path specifies the output
	// path. If not present, the output path is the same as the input path.
	output_path = input_path.parent_path();
	if ((arg2 != NULL) && (strlen(arg2) != 0))
	{
		// Check if the path conforms to utf-8
		std::string temp_out_path = arg2;
		if (!av.ValidateDirectoryPath(temp_out_path, output_path))
		{
			printf("User-defined output path is not a directory: %s\n",
				temp_out_path.c_str());
			return false;
		}
	}
	
	return true;
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
	if (!pm.Configure(input_path, output_path, &config))
		return false;

	// Begin parsing of Ch10 data by starting workers.
	pm.Parse();

	// Record metadata
	pm.RecordMetadata();

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