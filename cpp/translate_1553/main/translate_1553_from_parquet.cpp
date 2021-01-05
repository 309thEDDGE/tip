#include "translator_helper_funcs.h"

// TODO: Create a class with all of the required translation routine
// data and parameters. Pass this class around instead of long lists
// of vars.

int main(int argc, char* argv[])
{
	ManagedPath input_path;
	uint8_t thread_count = 0;
	ManagedPath dts_path;

	if (!GetArguments(argc, argv, input_path, dts_path))
		return 0;

	TranslationConfigParams config;
	std::string root_path = "";
	if (!InitializeConfig(root_path, config))
		return 1;
	thread_count = config.translate_thread_count_;

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
	if (thread_count > 0)
	{
		MTTranslate(config, input_path, dts1553.GetICDData(),
			 dts_path, chanid_to_bus_name_map, excluded_channel_ids);
	}
	// Start the translation routine that doesn't use threading.
	else
	{
		Translate(config, input_path, dts1553.GetICDData(),
			dts_path, chanid_to_bus_name_map, excluded_channel_ids);
	}

	//system("pause");
	return 0;
}
