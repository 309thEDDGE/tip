#include "translate_tabular_arinc429_main.h"

int TranslateTabularARINC429Main(int argc, char** argv)
{
    if (!SetLineBuffering(stdout))
        return 0;

    if (CheckForVersionArgument(argc, argv))
    {
        printf("%s version %s\n", TRANSLATE_429_EXE_NAME, GetVersionString().c_str());
        return 0;
    }

    ArgumentValidation av;
    std::map<int, std::string> def_args = {
        {1, "input_path"}, {2, "icd_path"}, {3, "output_dir"}, {4, "conf_dir"}, {5, "log_dir"}
    };
    std::string usage = "Usage: " TRANSLATE_429_EXE_NAME " <1553 Parquet path> "
        "<DTS1553 path> [output dir] [config dir path] [log dir]\nNeeds ch10 and "
        "DTS (ICD) input paths."; 
    std::map<int, std::string> options = {
        {2, usage},
        {5, "If a configuration directory is specified by the user then "
            "an output log directory must also be specified"}
    };
    std::map<std::string, std::string> args;
    if(!av.TestOptionalArgCount(argc, options)) 
        return 0;
    if(!av.ParseArgs(argc, argv, def_args, args, true))
        return 0;

    ManagedPath input_path;
    ManagedPath icd_path;
    ManagedPath output_dir;
    ManagedPath log_dir;
    ManagedPath conf_file_path;
    ManagedPath conf_schema_file_path;
    ManagedPath icd_schema_file_path;
    if (!transtab429::ValidatePaths(args.at("input_path"), args.at("icd_path"), args.at("output_dir"), 
                       args.at("conf_dir"), args.at("log_dir"), input_path, icd_path, 
                       output_dir, conf_file_path, conf_schema_file_path,
                       icd_schema_file_path, log_dir))
        return 0;

    if (!transtab429::SetupLogging(log_dir))
        return 0;

    TIPMDDocument parser_md_doc;
    ManagedPath parser_md_path = input_path / "_metadata.yaml";
    if(!transtab429::GetParsedMetadata(parser_md_path, parser_md_doc))
        return 0;

    ProvenanceData prov_data;
    if(!GetProvenanceData(icd_path.absolute(), 0, prov_data))
        return 0;

    YamlSV ysv;
    std::string icd_string, icd_schema_string, conf_string, conf_schema_string;
    if(!ysv.ValidateDocument(conf_file_path, conf_schema_file_path, conf_string, conf_schema_string))
        return 0;
    if(!ysv.ValidateDocument(icd_path, icd_schema_file_path, icd_string, icd_schema_string))
        return 0;

    TranslationConfigParams config_params;
    if (!config_params.InitializeWithConfigString(conf_string))
        return 0;

    // Use logger to print and record these values after logging
    // is implemented.
    SPDLOG_INFO(TRANSLATE_429_EXE_NAME " version: {:s}", prov_data.tip_version);
    SPDLOG_INFO("Input: {:s}", input_path.RawString());
    SPDLOG_INFO("Output dir: {:s}", output_dir.RawString());
    SPDLOG_INFO("DTS429 path: {:s}", icd_path.RawString());
    SPDLOG_INFO("DTS429 hash: {:s}", prov_data.hash);
    size_t thread_count = config_params.translate_thread_count_;
    SPDLOG_INFO("Thread count: {:d}", thread_count);

    // Read and process DTS429 here
    size_t arinc429_message_count = 10;

    if (config_params.auto_sys_limits_)
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

    // if (!Translate(thread_count, input_path, output_dir, dts1553.GetICDData(),
    //                transl_output_dir, output_base_name, config_params.select_specific_messages_,
    //                translated_msg_names))
    // {
    //     SPDLOG_WARN(
    //         "Failed to configure 1553 translation stage or an error occurred "
    //         "during translation");
    // }

    auto stop_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> secs = stop_time - start_time;
    double duration = secs.count();
    SPDLOG_INFO("Duration: {:.3f} sec", duration);

    // RecordMetadata(config_params, transl_output_dir, icd_path, chanid_to_bus_name_map,
    //                excluded_channel_ids, input_path, translated_msg_names,
    //                msg_name_substitutions, elem_name_substitutions, prov_data, 
    //                parser_md_doc);

    // Avoid deadlock in windows, see
    // http://stackoverflow.com/questions/10915233/stdthreadjoin-hangs-if-called-after-main-exits-when-using-vs2012-rc
    spdlog::shutdown();

    return 0;
}

namespace transtab429
{
    bool ValidatePaths(const std::string& str_input_path, const std::string& str_icd_path,
                    const std::string& str_output_dir, const std::string& str_conf_dir,
                    const std::string& str_log_dir, ManagedPath& input_path, ManagedPath& icd_path,
                    ManagedPath& output_dir, ManagedPath& conf_file_path, ManagedPath& conf_schema_file_path,
                    ManagedPath& icd_schema_file_path, ManagedPath& log_dir)
    {
        ArgumentValidation av;

        if (!av.CheckExtension(str_input_path, {"parquet"}))
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

        if (!av.CheckExtension(str_icd_path, {"txt", "csv", "yaml", "yml"}))
        {
            printf(
                "DTS429 (ICD) path \"%s\" does not have extension: txt, csv, "
                "yaml, yml\n",
                str_icd_path.c_str());
            return false;
        }
        if (!av.ValidateInputFilePath(str_icd_path, icd_path))
        {
            printf("DTS429 (ICD) path \"%s\" is not a valid path\n", str_icd_path.c_str());
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
        std::string icd_schema_name = "tip_dts429_schema.yaml";
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

    bool SetupLogging(const ManagedPath& log_dir)  // GCOVR_EXCL_LINE
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
            console_sink->set_level(spdlog::level::info);  // GCOVR_EXCL_LINE
            console_sink->set_pattern("%^[%T %L] %v%$");  // GCOVR_EXCL_LINE

            // file sink
            ManagedPath trans_log_path = log_dir / (TRANSLATE_429_EXE_NAME ".log");  // GCOVR_EXCL_LINE
            auto trans_log_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(trans_log_path.string(),  // GCOVR_EXCL_LINE
                                                                                        max_size, max_files);  // GCOVR_EXCL_LINE
            trans_log_sink->set_level(spdlog::level::trace);  // GCOVR_EXCL_LINE
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
}  // namespace transtab429