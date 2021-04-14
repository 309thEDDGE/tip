#include "parser_helper_funcs.h"

bool ValidateConfig(ParserConfigParams& config, std::string config_path,
	std::string config_schema_path)
{
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

	bool settings_validated = config.Initialize(conf_path.string());
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
	printf("Ch10 file path: %s\n", input_path.RawString().c_str());

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
	printf("Output path: %s\n", output_path.RawString().c_str());
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
	printf("Duration: %.3f sec\n", duration);
	return true;
}