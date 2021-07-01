#include "translator_helper_funcs.h"

// TODO: Create a class with all of the required translation routine
// data and parameters. Pass this class around instead of long lists
// of vars.

int main(int argc, char* argv[])
{
	std::string str_input_path;
	std::string str_icd_path;
	std::string str_output_dir;
	std::string str_conf_dir;
	std::string str_log_dir;
	if(!ParseArgs(argc, argv, str_input_path, str_icd_path, str_output_dir,
		str_conf_dir, str_log_dir))
		return 0;

	ManagedPath input_path;
	ManagedPath icd_path;
	ManagedPath output_dir;
	ManagedPath log_dir;
	ManagedPath conf_file_path;
	ManagedPath conf_schema_file_path;
	ManagedPath icd_schema_file_path;
	if(!ValidatePaths(str_input_path, str_icd_path, str_output_dir, str_conf_dir, 
		str_log_dir, input_path, icd_path, output_dir, conf_file_path, conf_schema_file_path,
		icd_schema_file_path, log_dir))
		return 0;

	std::string	config_doc;
	if(!ValidateConfSchema(conf_file_path, conf_schema_file_path, config_doc))
		return 0;

	if (!ValidateDTS1553YamlSchema(icd_path, icd_schema_file_path))
		return 0;

	TranslationConfigParams config_params;
	if(!config_params.InitializeWithConfigString(config_doc))
		return 0;

	if(!SetupLogging(log_dir))
		return 0;

	// Use logger to print and record these values after logging
	// is implemented.
	uint8_t thread_count = config_params.translate_thread_count_;
	printf("DTS1553 path: %s\n", icd_path.RawString().c_str());
	printf("Input: %s\n", input_path.RawString().c_str());
	printf("Thread count: %hhu\n", thread_count);
	printf("Output dir: %s\n", output_dir.RawString().c_str());

	DTS1553 dts1553;
	std::map<uint64_t, std::string> chanid_to_bus_name_map;
	std::set<uint64_t> excluded_channel_ids = std::set<uint64_t>();
	if (!PrepareICDAndBusMap(dts1553, input_path, icd_path, config_params.stop_after_bus_map_,
		config_params.prompt_user_, config_params.vote_threshold_, 
		config_params.vote_method_checks_tmats_, config_params.bus_name_exclusions_,
		config_params.tmats_busname_corrections_, config_params.use_tmats_busmap_, 
		chanid_to_bus_name_map, excluded_channel_ids))
	{
		return 0;
	}
	
	// Start translation routine for multi-threaded use case (or single-threaded using the threading framework
	// if thread_count = 1 is specified).
	double duration = 0.0;
	if (thread_count > 0)
	{
		MTTranslate(config_params, input_path, output_dir, dts1553.GetICDData(),
			 icd_path, chanid_to_bus_name_map, excluded_channel_ids, duration);
	}
	// Start the translation routine that doesn't use threading.
	else
	{
		Translate(config_params, input_path, output_dir, dts1553.GetICDData(),
			icd_path, chanid_to_bus_name_map, excluded_channel_ids, duration);
	}

	//system("pause");
	return 0;
}
