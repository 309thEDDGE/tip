#include "translator_helper_funcs.h"

// TODO: Create a class with all of the required translation routine
// data and parameters. Pass this class around instead of long lists
// of vars.

int main(int argc, char* argv[])
{
	ManagedPath input_path;
	uint8_t thread_count = 0;
	ManagedPath dts_path;
	std::vector<LogItem> log_items;

	if (!GetArguments(argc, argv, input_path, dts_path))
		return 0;

	TranslationConfigParams config;
	std::string conf_root_path = "";
	std::string schema_root_path = "";
	if (!InitializeConfig(conf_root_path, schema_root_path, config,
		log_items))
		return 1;
	thread_count = config.translate_thread_count_;

	if (!ValidateDTS1553YamlSchema(schema_root_path, dts_path,
		log_items))
		return 0;

	// TODO: handle log_items

	printf("DTS1553 path: %s\n", dts_path.RawString().c_str());
	printf("Input: %s\n", input_path.RawString().c_str());
	printf("Thread count: %hhu\n", thread_count);

	DTS1553 dts1553;
	std::map<uint64_t, std::string> chanid_to_bus_name_map;
	std::set<uint64_t> excluded_channel_ids = std::set<uint64_t>();
	if (!PrepareICDAndBusMap(dts1553, input_path, dts_path, config.stop_after_bus_map_,
		config.prompt_user_, config.vote_threshold_, config.vote_method_checks_tmats_,
		config.bus_name_exclusions_,	config.tmats_busname_corrections_, config.use_tmats_busmap_, 
		chanid_to_bus_name_map, excluded_channel_ids))
	{
		return 0;
	}
	
	// Start translation routine for multi-threaded use case (or single-threaded using the threading framework
	// if thread_count = 1 is specified).
	double duration = 0.0;
	if (thread_count > 0)
	{
		MTTranslate(config, input_path, dts1553.GetICDData(),
			 dts_path, chanid_to_bus_name_map, excluded_channel_ids, duration);
	}
	// Start the translation routine that doesn't use threading.
	else
	{
		Translate(config, input_path, dts1553.GetICDData(),
			dts_path, chanid_to_bus_name_map, excluded_channel_ids, duration);
	}

	//system("pause");
	return 0;
}
