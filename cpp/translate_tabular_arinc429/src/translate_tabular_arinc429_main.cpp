#include "translate_tabular_arinc429_main.h"

int TranslateTabularARINC429Main(int argc, char** argv)
{
    if (!SetLineBuffering(stdout))
        return 0;

    CLIGroup cli_group;
    TranslationConfigParams config;
    bool help_requested = false;
    bool show_version = false;
    bool show_dts_info = false;
    std::string high_level_description("Translate parsed ARINC429 data in Parquet format to "
        "engineering units, grouped into word-specific Parquet tables.");
    if(!ConfigureTranslatorCLI429(cli_group, config, help_requested, show_version,
        show_dts_info, high_level_description))
    {
        printf("ConfigureTranslatorCLI failed\n");
        return 0;
    }

    std::string nickname = "";
    std::shared_ptr<CLIGroupMember> cli;
    fflush(stdout);
    if (!cli_group.Parse(argc, argv, nickname, cli))
    {
        printf("%s", cli_group.MakeHelpString().c_str());
        return 0;
    }
    fflush(stdout);

    if (help_requested && nickname == "clihelp")
    {
        printf("%s", cli_group.MakeHelpString().c_str());
        return 0;
    }

    if (show_version && nickname == "cliversion")
    {
        printf(TRANSLATE_429_EXE_NAME " version %s\n", GetVersionString().c_str());
        return 0;
    }

    if(show_dts_info && nickname == "clidts")
    {
        printf("Raw DTS429 schema (yaml-formatted):\n%s\n\n", dts_429_schema.c_str());
        printf("%s\n\n%s\n\n%s\n\n", dts_429_word_explanation.c_str(),
            dts_429_parameter_defs.c_str(), dts_429_suppl_labels.c_str());
        return 0;
    }

    ArgumentValidation av;
    ManagedPath input_path;
    ManagedPath icd_path;
    ManagedPath output_dir;
    ManagedPath log_dir;
    if (!transtab429::ValidatePaths(config.input_data_path_str_, config.input_dts_path_str_,
        config.output_path_str_, config.log_path_str_, input_path, icd_path,
        output_dir, log_dir, &av))
        return 0;

    if(av.CheckExtension(icd_path.RawString(), {"yaml", "yml"}))
    {
        YamlSV ysv;
        std::string icd_string;
        if(!av.ValidateDocument(icd_path, icd_string))
            return 0;
        std::vector<LogItem> log_items;
        if(!ysv.Validate(icd_string, dts_429_schema, log_items))
        {
            printf("Schema validation failed for %s\n", icd_path.RawString().c_str());
            int print_count = 20;
            YamlSV::PrintLogItems(log_items, print_count, std::cout);
            return 0;
        }
    }

    if (!transtab429::SetupLogging(log_dir, spdlog::level::from_str(config.stdout_log_level_)))
        return 0;

    TIPMDDocument parser_md_doc;
    ManagedPath parser_md_path = input_path / "_metadata.yaml";
    if(!transtab429::GetParsedMetadata(parser_md_path, parser_md_doc))
        return 0;

    ProvenanceData prov_data;
    if(!GetProvenanceData(icd_path.absolute(), 0, prov_data))
        return 0;


    // Use logger to print and record these values after logging
    // is implemented.
    SPDLOG_INFO(TRANSLATE_429_EXE_NAME " version: {:s}", prov_data.tip_version);
    SPDLOG_INFO("Input: {:s}", input_path.RawString());
    SPDLOG_INFO("Output dir: {:s}", output_dir.RawString());
    SPDLOG_INFO("Log dir: {:s}", log_dir.RawString());
    SPDLOG_INFO("DTS429 path: {:s}", icd_path.RawString());
    SPDLOG_INFO("DTS429 hash: {:s}", prov_data.hash);
    size_t thread_count = config.translate_thread_count_;
    SPDLOG_INFO("Thread count: {:d}", thread_count);

    // Begin to read and process DTS429 here
    size_t arinc429_message_count = 0;

    // If subchannel name in DTS isn't in parsed metadata, add to following
    // vector and output subchan names with translated metadata.
    std::vector<std::string> subchannel_name_lookup_misses;

    // get runtime data output from parser metadata
    FileReader fr;
    if(fr.ReadFile(parser_md_path.RawString()) != 0)
    {
        SPDLOG_ERROR("Failed to read input metadata file: {:s}",
            parser_md_path.RawString());
        return false;
    }
    YAML::Node root_node = YAML::Load(fr.GetDocumentAsString());
    YAML::Node runtime_node = root_node["runtime"];

    // DTS429 inputs
    DTS429 dts429;
    std::vector<std::string> dts429_contents;
    transtab429::GetFileContents(icd_path.RawString(), dts429_contents);

    Organize429ICD org429;
    ARINC429Data arinc429_dts_data;

    if(!transtab429::IngestICD(&dts429, &org429, arinc429_dts_data,
        dts429_contents, arinc429_message_count, runtime_node, subchannel_name_lookup_misses))
    {
        return 0;
    }
    SPDLOG_DEBUG(arinc429_dts_data.LookupMapToString());
    // end reading and processing DTS429

    if (config.auto_sys_limits_)
    {
        if (!transtab429::SetSystemLimits(thread_count, arinc429_message_count))
            return 0;
    }

    auto start_time = std::chrono::high_resolution_clock::now();
    ManagedPath transl_output_dir = output_dir.CreatePathObject(input_path, "_translated");
    if (!transl_output_dir.create_directory())
        return 0;
    ManagedPath output_base_name("");
    SPDLOG_INFO("Translated data output dir: {:s}", transl_output_dir.RawString());

    std::set<std::string> translate_word_names;
    std::map<uint32_t, std::map<uint32_t, std::set<uint16_t>>> chanid_busnum_labels;

    if (!transtab429::Translate(thread_count, input_path, output_dir, arinc429_dts_data,
                   transl_output_dir, output_base_name, translate_word_names, chanid_busnum_labels))

    {
        SPDLOG_WARN(
            "Failed to configure 429 translation stage or an error occurred "
            "during translation");
    }

    auto stop_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> secs = stop_time - start_time;
    double duration = secs.count();
    SPDLOG_INFO("Duration: {:.3f} sec", duration);

    transtab429::RecordMetadata(config, transl_output_dir, icd_path,
                                input_path, translate_word_names, prov_data,
                                parser_md_doc, chanid_busnum_labels);

    // Avoid deadlock in windows, see
    // http://stackoverflow.com/questions/10915233/stdthreadjoin-hangs-if-called-after-main-exits-when-using-vs2012-rc
    spdlog::shutdown();

    return 0;
}

namespace transtab429
{
    bool ValidatePaths(const std::string& str_input_path, const std::string& str_icd_path,
                    const std::string& str_output_dir, const std::string& str_log_dir,
                    ManagedPath& input_path, ManagedPath& icd_path,
                    ManagedPath& output_dir, ManagedPath& log_dir, ArgumentValidation* av)
    {
        if (!av->CheckExtension(str_input_path, {"parquet"}))
        {
            printf("Input path \"%s\" does not have extension \"parquet\"\n",
                str_input_path.c_str());
            return false;
        }
        if (!av->ValidateDirectoryPath(str_input_path, input_path))
        {
            printf("Input path \"%s\" is not a valid path\n", str_input_path.c_str());
            return false;
        }

        if (!av->CheckExtension(str_icd_path, {"txt", "csv", "yaml", "yml"}))
        {
            printf(
                "DTS429 (ICD) path \"%s\" does not have extension: txt, csv, "
                "yaml, yml\n",
                str_icd_path.c_str());
            return false;
        }
        if (!av->ValidateInputFilePath(str_icd_path, icd_path))
        {
            printf("DTS429 (ICD) path \"%s\" is not a valid path\n", str_icd_path.c_str());
            return false;
        }

        if (!av->ValidateDirectoryPath(str_output_dir, output_dir))
            return false;

        if (!av->ValidateDirectoryPath(str_log_dir, log_dir))
            return false;

        return true;
    }

    bool SetupLogging(const ManagedPath& log_dir, spdlog::level::level_enum stdout_log_level)  // GCOVR_EXCL_LINE
    {
        // Use the chart on this page for logging level reference:
        // https://www.tutorialspoint.com/log4j/log4j_logging_levels.htm

        try  // GCOVR_EXCL_LINE
        {
            // Set global logging level
            spdlog::set_level(spdlog::level::trace);  // GCOVR_EXCL_LINE

            // Rotating logs maxima
            int max_size = 1024 * 1024 * 5;  // GCOVR_EXCL_LINE
            int max_files = 5;  // GCOVR_EXCL_LINE

            // Console sink
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();  // GCOVR_EXCL_LINE
            console_sink->set_level(stdout_log_level);  // GCOVR_EXCL_LINE
            console_sink->set_pattern("%^[%T %L] %v%$");  // GCOVR_EXCL_LINE

            // file sink
            ManagedPath trans_log_path = log_dir / (TRANSLATE_429_EXE_NAME ".log");  // GCOVR_EXCL_LINE
            auto trans_log_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(trans_log_path.string(),  // GCOVR_EXCL_LINE
                                                                                        max_size, max_files);  // GCOVR_EXCL_LINE
            trans_log_sink->set_level(spdlog::level::debug);  // GCOVR_EXCL_LINE
            trans_log_sink->set_pattern("[%D %T %L] [%@] %v");  // GCOVR_EXCL_LINE

            // List of sinks for translator
            spdlog::sinks_init_list trans_sinks = {console_sink, trans_log_sink};  // GCOVR_EXCL_LINE

            // Create and register the logger for translator log and console.
            auto trans_logger = std::make_shared<spdlog::logger>("trans_logger", trans_sinks);  // GCOVR_EXCL_LINE
            trans_logger->set_level(spdlog::level::trace);  // GCOVR_EXCL_LINE
            spdlog::register_logger(trans_logger);  // GCOVR_EXCL_LINE

            spdlog::set_default_logger(trans_logger);  // GCOVR_EXCL_LINE
        }
        catch (const spdlog::spdlog_ex& ex)  // GCOVR_EXCL_LINE
        {
            printf("SetupLogging() failed: %s\n", ex.what());  // GCOVR_EXCL_LINE
            return false;  // GCOVR_EXCL_LINE
        }
        return true;  // GCOVR_EXCL_LINE
    }

    bool IngestICD(DTS429* dts429, Organize429ICD* org429, ARINC429Data& data429,
                    const std::vector<std::string>& icd_lines, size_t& arinc_message_count,
                    YAML::Node& parser_md_runtime_node,
                    std::vector<std::string>& subchan_name_lookup_misses)
    {
        auto start_time = std::chrono::high_resolution_clock::now();

        std::unordered_map<uint16_t,std::unordered_map<uint16_t, std::unordered_map<
                        uint16_t,std::unordered_map<int8_t, size_t>>>> organized_lookup_map;
        std::vector<std::vector<std::vector<ICDElement>>> element_table;
        std::unordered_map<size_t,std::vector<std::string>> arinc_word_names;

        // NOTE: it may be required to create a new node deeper into the parser md node, so that
        // OrganizeICDMap has access to tmats_chanid_to_429_subchan_and_name

        // DTS429
        std::unordered_map<std::string, std::vector<ICDElement>> word_elements;
        if(!dts429->IngestLines(icd_lines, word_elements))
            return false;

        // Organize429ICD inputs
        if(!org429->OrganizeICDMap(word_elements, parser_md_runtime_node, organized_lookup_map, element_table))
            return false;

        subchan_name_lookup_misses = org429->GetSubchannelNameLookupMisses();
        arinc_message_count = org429->GetValidArincWordCount();
        arinc_word_names = org429->GetArincWordNames();

        // ARINC429Datac
        data429 = ARINC429Data(organized_lookup_map, element_table, arinc_word_names, arinc_message_count);

        SPDLOG_INFO("Total ARINC 429 words defined in ICD: {:d} words", arinc_message_count);
        auto stop_time = std::chrono::high_resolution_clock::now();
        SPDLOG_INFO("DTS429 ingest and word lookup table synthesis duration: {:d} sec",
            std::chrono::duration_cast<std::chrono::seconds>(stop_time - start_time).count());

        return true;
    }

    bool GetParsedMetadata(const ManagedPath& input_md_path,
        TIPMDDocument& parser_md_doc)
    {
        if(!input_md_path.is_regular_file())
        {
            SPDLOG_ERROR("Input metadata path not present: {:s}",
                input_md_path.RawString());
            return false;
        }

        FileReader fr;
        if(fr.ReadFile(input_md_path.string()) != 0)
        {
            SPDLOG_ERROR("Failed to read input metadata file: {:s}",
                input_md_path.RawString());
            return false;
        }

        if(!parser_md_doc.ReadDocument(fr.GetDocumentAsString()))
        {
            SPDLOG_ERROR("Failed to interpret input metadata as TIPMDDocument: {:s}",
                input_md_path.RawString());
            return false;
        }

        std::string parsed_type = parser_md_doc.type_category_->node.as<std::string>();
        std::string expected_type = "parsed_" + ch10packettype_to_string_map.at(Ch10PacketType::ARINC429_F0);
        if(parsed_type != expected_type)
        {
            SPDLOG_ERROR("Parsed metadata document at {:s} has type {:s}. "
                "Must be {:s}", input_md_path.RawString(), parsed_type, expected_type);
            return false;
        }

        return true;
    }

    bool SetSystemLimits(uint8_t thread_count, size_t message_count)
    {
        // Add 6 more FDs for multiple spdlog logs.
        uint64_t requested_file_limit = thread_count * (message_count + 6) + thread_count + 3;
        SPDLOG_INFO("Requested file descriptor limit: {:d}", requested_file_limit);

        if (!SetFileDescriptorLimits(requested_file_limit))
            return false;
        return true;
    }

    bool GetFileContents(std::string file_name, std::vector<std::string> & file_contents)
    {
        // modified from: https://thispointer.com/c-how-to-read-a-file-line-by-line-into-a-vector/

        // Open the File
        std::ifstream in(file_name.c_str());
        // Check if object is valid
        if(!in)
        {
            std::cerr << "Cannot open the File : "<<file_name<<std::endl;
            return false;
        }
        std::string str;
        // Read the next line from File untill it reaches the end.
        while (std::getline(in, str))
        {
            // Line contains string of length > 0 then save it in vector
            if(str.size() > 0)
                file_contents.push_back(str);
        }
        //Close The File
        in.close();
        return true;
    }

    bool Translate(size_t thread_count, const ManagedPath& input_path,
            const ManagedPath& output_dir, const ARINC429Data& icd,
            const ManagedPath& translated_data_dir,
            const ManagedPath& output_base_name,
            std::set<std::string>& translated_msg_names,
            std::map<uint32_t, std::map<uint32_t, std::set<uint16_t>>>& chanid_busnum_labels_map)
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
        std::shared_ptr<TranslateTabularContextARINC429> context =
            std::make_shared<TranslateTabularContextARINC429>(icd);
        // context->Configure(".parquet", 2);
        std::vector<std::string> ridealong_col_names{"time"};
        std::vector<std::string> data_col_names{"time","channelid","bus","label","SDI","data","SSM","parity"};
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
        std::shared_ptr<TranslateTabularContextARINC429> ctx;
        for (size_t i = 0; i < managers.size(); i++)
        {
            std::map<uint32_t, std::map<uint32_t, std::set<uint16_t>>> temp_chanid_busname_labels;

            ctx = std::dynamic_pointer_cast<TranslateTabularContextARINC429>(
                managers[i]->GetContext());
            translated_msg_names.insert(ctx->translated_msg_names.begin(),
                                        ctx->translated_msg_names.end());

            temp_chanid_busname_labels = ctx->GetChanidBusnameLabels();
            chanid_busnum_labels_map.insert(temp_chanid_busname_labels.begin(),
                                        temp_chanid_busname_labels.end());
        }

        return true;
    }

    bool RecordMetadata(const TranslationConfigParams& config,
                        const ManagedPath& translated_data_dir,
                        const ManagedPath& dts_path,
                        const ManagedPath& input_path,
                        const std::set<std::string>& translated_messages,
                        const ProvenanceData& prov_data, const TIPMDDocument& parser_md_doc,
                        const std::map<uint32_t, std::map<uint32_t, std::set<uint16_t>>>& chanid_busnum_labels)

    {
        TIPMDDocument md;
        ManagedPath md_file_path = translated_data_dir / "_metadata.yaml";

        // Parsed data provenance and that of precursor ch10 file
        std::string label = ch10packettype_to_string_map.at(Ch10PacketType::ARINC429_F0);
        std::string dts429hash = prov_data.hash;
        std::string parsed429uuid = parser_md_doc.uid_category_->node.as<std::string>();
        std::string uid = Sha256(dts429hash + prov_data.time +
            prov_data.tip_version + parsed429uuid);

        md.type_category_->SetScalarValue("translated_" + label);
        md.uid_category_->SetScalarValue(uid);
        md.prov_category_->SetMappedValue("time", prov_data.time);
        md.prov_category_->SetMappedValue("version", prov_data.tip_version);

        // DTS429 resource
        md.AddResource("DTS_" + label, dts_path.RawString(), dts429hash);

        // parsed 429 resource
        md.AddResource(parser_md_doc.type_category_->node.as<std::string>(),
            input_path.RawString(), parser_md_doc.uid_category_->node.as<std::string>());

        // Add ch10 resource which is already a resource of the parsed 429 metadata doc
        md.AddResources(parser_md_doc);

        // Record config parameters.
        md.config_category_->SetArbitraryMappedValue("translate_thread_count",
            config.translate_thread_count_);
        md.config_category_->SetArbitraryMappedValue("use_tmats_busmap",
                config.use_tmats_busmap_);
        md.config_category_->SetArbitraryMappedValue("tmats_busname_corrections",
                config.tmats_busname_corrections_);
        md.config_category_->SetArbitraryMappedValue("prompt_user",
                config.prompt_user_);
        md.config_category_->SetArbitraryMappedValue("vote_threshold",
                config.vote_threshold_);
        md.config_category_->SetArbitraryMappedValue("vote_method_checks_tmats",
                config.vote_method_checks_tmats_);
        md.config_category_->SetArbitraryMappedValue("bus_name_exclusions",
                config.bus_name_exclusions_);
        md.config_category_->SetArbitraryMappedValue("stop_after_bus_map",
                config.stop_after_bus_map_);
        md.config_category_->SetArbitraryMappedValue("select_specific_messages",
                config.select_specific_messages_);
        md.config_category_->SetArbitraryMappedValue("exit_after_table_creation",
                config.exit_after_table_creation_);
        md.config_category_->SetArbitraryMappedValue("auto_sys_limits",
                config.auto_sys_limits_);

        // Record translated messages.
        md.runtime_category_->SetArbitraryMappedValue("translated_messages",
            translated_messages);

        // Record relationship between translated: chanid -> busnum -> [labels]
        md.runtime_category_->SetArbitraryMappedValue("translated_chanid_busnum_labels_mapping",
            chanid_busnum_labels);

        // Get a string containing the complete metadata output and
        // and write it to the yaml file.
        md.CreateDocument();
        std::ofstream stream_translation_metadata(md_file_path.string(),
                                                std::ofstream::out | std::ofstream::trunc);
        if (!(stream_translation_metadata.good() && stream_translation_metadata.is_open()))
        {
            SPDLOG_ERROR("RecordMetadata(): Failed to open metadata file for writing: {:s}",
                md_file_path.string());
            return false;
        }
        stream_translation_metadata << md.GetMetadataString();
        stream_translation_metadata.close();
        return true;
    }

}  // namespace transtab429