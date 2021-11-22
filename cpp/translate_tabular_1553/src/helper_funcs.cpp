#include "helper_funcs.h"

bool ParseArgs(int argc, char* argv[], std::string& str_input_path,
               std::string& str_icd_path, std::string& str_output_dir, std::string& str_conf_dir,
               std::string& str_log_dir)
{
    if (argc < 3)
    {
        printf(
            "Usage: tip_translate <1553 Parquet path> <DTS1553 path> [output dir] "
            "[config dir path] [log dir]\nNeeds ch10 and DTS (ICD) input paths.\n");
        return false;
    }
    str_input_path = std::string(argv[1]);
    str_icd_path = std::string(argv[2]);

    str_output_dir = "";
    str_conf_dir = "";
    str_log_dir = "";
    if (argc < 4) return true;
    str_output_dir = std::string(argv[3]);

    if (argc < 6)
    {
        printf(
            "If an output directory is specified then a confguration files directory "
            "and log directory must also be specified\n");
        return false;
    }
    str_conf_dir = std::string(argv[4]);
    str_log_dir = std::string(argv[5]);

    return true;
}

bool ValidatePaths(const std::string& str_input_path, const std::string& str_icd_path,
                   const std::string& str_output_dir, const std::string& str_conf_dir,
                   const std::string& str_log_dir, ManagedPath& input_path, ManagedPath& icd_path,
                   ManagedPath& output_dir, ManagedPath& conf_file_path, ManagedPath& conf_schema_file_path,
                   ManagedPath& icd_schema_file_path, ManagedPath& log_dir)
{
    ArgumentValidation av;

    if (!av.CheckExtension(str_input_path, "parquet"))
    {
        printf("Input path \"%s\" does not have extension \"parquet\"\n",
               str_input_path.c_str());
        return false;
    }
    if (!av.ValidateDirectoryPath(str_input_path, input_path))
    {
        printf("Input path \"%s\" is not a valid path\n", str_input_path.c_str());
        return false;
    }

    if (!av.CheckExtension(str_icd_path, "txt", "csv", "yaml", "yml"))
    {
        printf(
            "DTS1553 (ICD) path \"%s\" does not have extension: txt, csv, "
            "yaml, yml\n",
            str_icd_path.c_str());
        return false;
    }
    if (!av.ValidateInputFilePath(str_icd_path, icd_path))
    {
        printf("DTS1553 (ICD) path \"%s\" is not a valid path\n", str_icd_path.c_str());
        return false;
    }

    ManagedPath default_output_dir = input_path.parent_path();
    if (!av.ValidateDefaultOutputDirectory(default_output_dir, str_output_dir,
                                           output_dir, true))
        return false;

    ManagedPath default_conf_dir({"..", "conf"});
    std::string translate_conf_name = "translate_conf.yaml";
    if (!av.ValidateDefaultInputFilePath(default_conf_dir, str_conf_dir,
                                         translate_conf_name, conf_file_path))
    {
        printf(
            "The constructed path may be the default path, relative to CWD "
            "(\"../conf\"). Specify an explicit output directory, conf path, and log "
            "directory to remedy.\n");
        return false;
    }

    std::string conf_schema_name = "tip_translate_conf_schema.yaml";
    std::string icd_schema_name = "tip_dts1553_schema.yaml";
    std::string schema_dir = "yaml_schemas";
    conf_schema_file_path = conf_file_path.parent_path() / schema_dir / conf_schema_name;
    icd_schema_file_path = conf_file_path.parent_path() / schema_dir / icd_schema_name;
    if (!conf_schema_file_path.is_regular_file())
        return false;
    if (!icd_schema_file_path.is_regular_file())
        return false;

    ManagedPath default_log_dir({"..", "logs"});
    if (!av.ValidateDefaultOutputDirectory(default_log_dir, str_log_dir,
                                           log_dir, true))
        return false;

    return true;
}

bool ValidateConfSchema(const ManagedPath& conf_file_path,
                        const ManagedPath& conf_schema_file_path, std::string& conf_doc)
{
    ArgumentValidation av;
    if (!av.ValidateDocument(conf_file_path, conf_doc)) return false;

    std::string schema_doc;
    if (!av.ValidateDocument(conf_schema_file_path, schema_doc)) return false;

    YamlSV ysv;
    std::vector<LogItem> log_items;
    if (!ysv.Validate(conf_doc, schema_doc, log_items))
    {
        printf(
            "Failed to validate translator configuration file (%s) with schema "
            "(%s)\n",
            conf_file_path.RawString().c_str(),
            conf_schema_file_path.RawString().c_str());
        int print_count = 25;
        YamlSV::PrintLogItems(log_items, print_count, std::cout);
        return false;
    }
    return true;
}

bool ValidateDTS1553YamlSchema(const ManagedPath& icd_path,
                               const ManagedPath& icd_schema_file_path)
{
    // Do not proceed if the dts_path does not have a yaml or yml
    // extension.
    ArgumentValidation av;
    if (!av.CheckExtension(icd_path.RawString(), "yaml", "yml"))
        return true;

    // Check the schema and dts1553 documents for utf-8 conformity
    std::string schema_doc;
    if (!av.ValidateDocument(icd_schema_file_path, schema_doc)) return false;

    std::string icd_doc;
    if (!av.ValidateDocument(icd_path, icd_doc)) return false;

    // Validate the dts1553 document using schema validator.
    YamlSV ysv;
    std::vector<LogItem> log_items;
    if (!ysv.Validate(icd_doc, schema_doc, log_items))
    {
        printf("Failed to validate DTS1553 file (%s) with schema (%s)\n",
               icd_path.RawString().c_str(), icd_schema_file_path.RawString().c_str());

        int print_count = 25;
        YamlSV::PrintLogItems(log_items, print_count, std::cout);
        return false;
    }

    return true;
}

bool SetupLogging(const ManagedPath& log_dir)
{
    // Use the chart on this page for logging level reference:
    // https://www.tutorialspoint.com/log4j/log4j_logging_levels.htm

    try
    {
        // Set global logging level
        spdlog::set_level(spdlog::level::trace);

        // Setup async thread pool.
        spdlog::init_thread_pool(8192, 2);

        // Rotating logs maxima
        int max_size = 1024 * 1024 * 5;
        int max_files = 5;

        // Console sink
        // Important! "mt" = multithreaded sink used with async_logger
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::info);
        console_sink->set_pattern("%^[%T %L] %v%$");

        // file sink
        std::string log_base_name(TRANSLATE_1553_EXE_NAME);
        ManagedPath trans_log_path = log_dir / (log_base_name + ".log");
        // Important! "mt" = multithreaded sink used with async_logger
        auto trans_log_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(trans_log_path.string(),
                                                                                     max_size, max_files);
        trans_log_sink->set_level(spdlog::level::trace);
        trans_log_sink->set_pattern("[%D %T %L] [%@] %v");

        // List of sinks for translator
        spdlog::sinks_init_list trans_sinks = {console_sink, trans_log_sink};

        // Create and register the logger for translator log and console.
        auto trans_logger = std::make_shared<spdlog::async_logger>("trans_logger", trans_sinks,
                                                                   spdlog::thread_pool(), spdlog::async_overflow_policy::block);
        trans_logger->set_level(spdlog::level::trace);
        spdlog::register_logger(trans_logger);

        spdlog::set_default_logger(trans_logger);
    }
    catch (const spdlog::spdlog_ex& ex)
    {
        printf("SetupLogging() failed: %s\n", ex.what());
        return false;
    }
    return true;
}

bool IngestICD(DTS1553& dts1553, const ManagedPath& dts_path,
               std::map<std::string, std::string>& msg_name_substitutions,
               std::map<std::string, std::string>& elem_name_substitutions)
{
    // Read lines from ICD text file, ingest, and manipulate.
    auto start_time = std::chrono::high_resolution_clock::now();
    FileReader fr;
    if (fr.ReadFile(dts_path.string()) == 1)
    {
        printf("Failed to read ICD: %s\n", dts_path.RawString().c_str());
        return false;
    }

    if (!dts1553.IngestLines(dts_path, fr.GetLines(), msg_name_substitutions,
                             elem_name_substitutions))
    {
        printf("Failed to ingest DTS1553 data\n");
        return false;
    }
    auto stop_time = std::chrono::high_resolution_clock::now();
    printf("DTS1553 ingest and message lookup table synthesis duration: %zd sec\n",
           std::chrono::duration_cast<std::chrono::seconds>(stop_time - start_time).count());

    return true;
}

bool PrepareBusMap(const ManagedPath& input_path, DTS1553& dts1553,
                   bool stop_after_bus_map, bool prompt_user,
                   uint64_t vote_threshold, bool vote_method_checks_tmats,
                   std::vector<std::string> bus_exclusions,
                   std::map<std::string, std::string>& tmats_bus_name_corrections,
                   bool use_tmats_busmap, std::map<uint64_t, std::string>& chanid_to_bus_name_map,
                   std::set<uint64_t>& excluded_channel_ids)
{
    printf("\nStarting Bus Map\n");
    auto bus_map_start_time = std::chrono::high_resolution_clock::now();

    // Create the metadata file path from the input raw parquet path.
    Metadata input_md;
    ManagedPath input_md_path = input_md.GetYamlMetadataPath(
        input_path, "_metadata");

    // Generate the bus map from metadata and possibly user
    // input.
    if (!SynthesizeBusMap(dts1553, input_md_path, prompt_user, vote_threshold,
                          vote_method_checks_tmats, bus_exclusions, tmats_bus_name_corrections, use_tmats_busmap,
                          chanid_to_bus_name_map, excluded_channel_ids))
    {
        return false;
    }
    auto bus_map_end_time = std::chrono::high_resolution_clock::now();
    printf("Bus Map Duration: %zd sec\n", std::chrono::duration_cast<std::chrono::seconds>(bus_map_end_time - bus_map_start_time).count());

    // If the config file option stop_after_bus_map == true,
    // exit the program.
    if (stop_after_bus_map)
    {
        printf("User-defined config parameter \"stop_after_bus_map\" set to true\n");
        return false;
    }
    return true;
}

bool SynthesizeBusMap(DTS1553& dts1553, const ManagedPath& md_path, bool prompt_user,
                      uint64_t vote_threshold, bool vote_method_checks_tmats,
                      std::vector<std::string> bus_exclusions,
                      std::map<std::string, std::string>& tmats_bus_name_corrections,
                      bool use_tmats_busmap, std::map<uint64_t, std::string>& chanid_to_bus_name_map,
                      std::set<uint64_t>& excluded_channel_ids)
{
    std::unordered_map<uint64_t, std::set<std::string>> message_key_to_busnames_map;

    dts1553.ICDDataPtr()->PrepareMessageKeyMap(message_key_to_busnames_map,
                                               dts1553.GetSupplBusNameToMessageKeyMap());

    // Use YamlReader to read the yaml metadata file.
    YamlReader yr;
    if (!yr.LinkFile(md_path.string()))
    {
        printf("Translate main: SynthesizeBusMap(): YamlReader failed to link file!\n");
        return false;
    }

    // Get the set of channel IDs from metadata -- REQUIRED.
    // Initially read in a map of uint64_t to vector of uint64_t
    // and then get channel IDs from the map keys
    std::set<uint64_t> chanid_to_lruaddrs_set;
    std::map<uint64_t, std::vector<std::vector<uint16_t>>> chanid_to_comm_words_map;
    if (!yr.GetParams("chanid_to_comm_words", chanid_to_comm_words_map, true))
    {
        printf(
            "Translate main: SynthesizeBusMap(): Failed to get chanid_to_comm_words"
            " map from metadata!\n");
        return false;
    }

    for (std::map<uint64_t, std::vector<std::vector<uint16_t>>>::const_iterator it =
             chanid_to_comm_words_map.begin();
         it != chanid_to_comm_words_map.end();
         ++it)
    {
        chanid_to_lruaddrs_set.insert(it->first);
    }

    // Get the map of TMATS channel ID to source -- NOT REQUIRED.
    std::map<uint64_t, std::string> tmats_chanid_to_source_map;
    yr.GetParams("tmats_chanid_to_source", tmats_chanid_to_source_map, false);

    //// Get the map of TMATS channel ID to type -- NOT REQUIRED.
    std::map<uint64_t, std::string> tmats_chanid_to_type_map;
    yr.GetParams("tmats_chanid_to_type", tmats_chanid_to_type_map, false);

    // Initialize the maps necessary to synthesize the channel ID to bus name map.
    BusMap bm;

    // convert vector to set
    IterableTools it;
    std::set<std::string> bus_exclusions_set = it.VecToSet(bus_exclusions);

    // The mask will mask out word count/mode code from the transmit and
    // recieve command word portion of the key.
    // The last 16 bits of the key represent the recieve command word
    // and the second to last 16 bits of the key represent the transmit command word.
    // The last five bits of the transmit and recieve command words represent
    // word count/mode code. The word count/mode code portion is masked out
    // because in the case where the command word is a mode code, the last five
    // bits of the command word are the mode code bits and no longer match
    // the command words found in the input icd file unless they are masked to zero.
    uint64_t mask = 0b1111111111111111111111111111111111111111111000001111111111100000;
    bm.InitializeMaps(&message_key_to_busnames_map,
                      chanid_to_lruaddrs_set,
                      mask,
                      vote_threshold,
                      vote_method_checks_tmats,
                      bus_exclusions_set,
                      tmats_chanid_to_source_map,
                      tmats_bus_name_corrections);

    // Submit votes for each transmit/recieve command word from each
    // channel ID found in metadata from parameter chanid_to_comm_words
    for (std::map<uint64_t, std::vector<std::vector<uint16_t>>>::const_iterator it =
             chanid_to_comm_words_map.begin();
         it != chanid_to_comm_words_map.end();
         ++it)
    {
        for (int i = 0; i < it->second.size(); i++)
        {
            if (it->second[i].size() == 2)
            {
                bm.SubmitMessage(it->second[i][0], it->second[i][1], it->first);
            }
        }
    }

    if (!bm.Finalize(chanid_to_bus_name_map, use_tmats_busmap,
                     prompt_user))
    {
        printf("Bus mapping failed!\n");
        return false;
    }

    // Convert the chanid to bus name map to a map from
    // int to set of strings.
    std::map<uint64_t, std::set<std::string>> chanid_to_bus_name_set_map;
    for (std::map<uint64_t, std::string>::iterator it = chanid_to_bus_name_map.begin();
         it != chanid_to_bus_name_map.end(); ++it)
    {
        std::set<std::string> bus_name_set;
        bus_name_set.insert(it->second);
        chanid_to_bus_name_set_map[it->first] = bus_name_set;
    }

    // If a channel ID from chanid_to_lruaddrs_set is not in
    // chanid_to_bus_name_map add it as an excluded channel
    // id and save to metadata later
    for (auto channel_id : chanid_to_lruaddrs_set)
    {
        if (chanid_to_bus_name_map.count(channel_id) == 0)
            excluded_channel_ids.insert(channel_id);
    }

    // Reverse the map that is populated in the previous step.
    std::map<std::string, std::set<uint64_t>> bus_name_to_chanid_map =
        it.ReverseMapSet(chanid_to_bus_name_set_map);

    // Correct and update the lookup table in ICDData with the new map.
    if (!dts1553.ICDDataPtr()->ReplaceBusNameWithChannelIDInLookup(bus_name_to_chanid_map))
    {
        printf("Failed to update Lookup map with bus_name_to_chanid_map!\n");
        return false;
    }

    return true;
}

bool SetSystemLimits(uint8_t thread_count, size_t message_count)
{
    // Add 6 more FDs for multiple spdlog logs.
    uint64_t requested_file_limit = thread_count * (message_count + 6) + thread_count + 3;
    printf("Requested file descriptor limit: %llu\n", requested_file_limit);

    if (!SetFileDescriptorLimits(requested_file_limit))
        return false;
    return true;
}

bool Translate(size_t thread_count, const ManagedPath& input_path,
               const ManagedPath& output_dir, ICDData icd,
               const ManagedPath& translated_data_dir,
               const ManagedPath& output_base_name,
               const std::vector<std::string>& selected_msg_names,
               std::set<std::string>& translated_msg_names)
{
    // Get list of input files
    bool success = false;
    std::vector<ManagedPath> dir_entries;
    input_path.ListDirectoryEntries(success, dir_entries);
    if (!success)
        return false;

    // Filter to get files only and exclude certain files.
    std::vector<std::string> file_exclusion_substrings({"metadata", "TMATS"});
    dir_entries = ManagedPath::ExcludePathsWithSubString(
        ManagedPath::SelectFiles(dir_entries), file_exclusion_substrings);

    SPDLOG_INFO("Files to be read in for translation:");
    for (std::vector<ManagedPath>::const_iterator it = dir_entries.cbegin();
         it != dir_entries.cend(); ++it)
        SPDLOG_INFO("{:s}", it->RawString());

    // Configure Context
    std::set<std::string> selected_msg_names_set;
    for (size_t i = 0; i < selected_msg_names.size(); i++)
    {
        selected_msg_names_set.insert(selected_msg_names.at(i));
    }
    std::shared_ptr<TranslateTabularContext1553> context =
        std::make_shared<TranslateTabularContext1553>(icd, selected_msg_names_set);
    // context->Configure(".parquet", 2);
    std::vector<std::string> ridealong_col_names{"time"};
    std::vector<std::string> data_col_names{"time", "data", "channelid", "txrtaddr",
                                            "rxrtaddr", "txsubaddr", "rxsubaddr"};
    context->SetColumnNames(ridealong_col_names, data_col_names);

    // Create primary object which controls translation
    TranslateTabular translate(thread_count, context);
    if (!translate.SetInputFiles(dir_entries, ".parquet"))
    {
        return false;
    }

    translate.SetOutputDir(translated_data_dir, output_base_name);
    if (!translate.Translate())
    {
        return false;
    }

    // Collect the translated message names from each of the Context objects.
    std::vector<std::shared_ptr<TranslationManager>> managers = translate.GetManagers();
    std::shared_ptr<TranslateTabularContext1553> ctx;
    for (size_t i = 0; i < managers.size(); i++)
    {
        ctx = std::dynamic_pointer_cast<TranslateTabularContext1553>(
            managers[i]->GetContext());
        translated_msg_names.insert(ctx->translated_msg_names.begin(),
                                    ctx->translated_msg_names.end());
    }

    return true;
}

bool RecordMetadata(TranslationConfigParams config, const ManagedPath& translated_data_dir,
                    const ManagedPath& dts_path, std::map<uint64_t, std::string>& chanid_to_bus_name_map,
                    const std::set<uint64_t>& excluded_channel_ids, const ManagedPath& input_path,
                    const std::set<std::string>& translated_messages,
                    const std::map<std::string, std::string>& msg_name_substitutions,
                    const std::map<std::string, std::string>& elem_name_substitutions)
{
    // Use Metadata class to create the output metadata file path.
    Metadata md;
    ManagedPath md_path = md.GetYamlMetadataPath(translated_data_dir,
                                                 "_metadata");

    // Record config parameters.
    md.RecordSingleKeyValuePair("translate_thread_count", config.translate_thread_count_);
    md.RecordSingleKeyValuePair("use_tmats_busmap", config.use_tmats_busmap_);
    md.RecordSingleKeyValuePair("tmats_busname_corrections", config.tmats_busname_corrections_);
    md.RecordSingleKeyValuePair("prompt_user", config.prompt_user_);
    md.RecordSingleKeyValuePair("vote_threshold", config.vote_threshold_);
    md.RecordSingleKeyValuePair("vote_method_checks_tmats", config.vote_method_checks_tmats_);
    md.RecordSingleKeyValuePair("bus_name_exclusions", config.bus_name_exclusions_);
    md.RecordSingleKeyValuePair("stop_after_bus_map", config.stop_after_bus_map_);
    md.RecordSingleKeyValuePair("select_specific_messages", config.select_specific_messages_);
    md.RecordSingleKeyValuePair("exit_after_table_creation", config.exit_after_table_creation_);

    // Record the ICD path.
    md.RecordSingleKeyValuePair("dts_path", dts_path.RawString());

    // Record parsed parquet file used to translate.
    md.RecordSingleKeyValuePair("1553_parquet_file_path", input_path.RawString());

    // Record the final bus map used for translation.
    md.RecordSimpleMap(chanid_to_bus_name_map, "chanid_to_bus_name_map");

    // Record the busmap status.
    md.RecordSingleKeyValuePair("excluded_channel_ids", excluded_channel_ids);

    // Record translated messages.
    md.RecordSingleKeyValuePair("translated_messages", translated_messages);

    // Record message and element name substitutions which arise from URI
    // percent-encoding all such strings retrieved from the DTS 1553 yaml.
    // Values are only recorded for those message or element names which
    // were modified by the percent-encoding algorithm.
    md.RecordSimpleMap(msg_name_substitutions, "uri_percent_encoded_message_names");
    md.RecordSimpleMap(elem_name_substitutions, "uri_percent_encoded_element_names");

    // Get a string containing the complete metadata output and
    // and write it to the yaml file.
    std::ofstream stream_translation_metadata(md_path.string(),
                                              std::ofstream::out | std::ofstream::trunc);
    if (!(stream_translation_metadata.good() && stream_translation_metadata.is_open()))
    {
        printf("RecordMetadata(): Failed to open metadata file for writing, %s\n",
               md_path.string().c_str());
        return false;
    }
    stream_translation_metadata << md.GetMetadataString();
    stream_translation_metadata.close();
    return true;
}
