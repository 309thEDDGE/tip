#include "helper_funcs.h"

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

bool SetupLogging(const ManagedPath& log_dir)
{
    // Use the chart on this page for logging level reference:
    // https://www.tutorialspoint.com/log4j/log4j_logging_levels.htm

    try
    {
        // Set global logging level
        spdlog::set_level(spdlog::level::trace);

        // Rotating logs maxima
        int max_size = 1024 * 1024 * 5;
        int max_files = 5;

        // Console sink
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::info);
        console_sink->set_pattern("%^[%T %L] %v%$");

        // file sink
        ManagedPath trans_log_path = log_dir / (TRANSLATE_429_EXE_NAME ".log");
        auto trans_log_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(trans_log_path.string(),
                                                                                     max_size, max_files);
        trans_log_sink->set_level(spdlog::level::trace);
        trans_log_sink->set_pattern("[%D %T %L] [%@] %v");

        // List of sinks for translator
        spdlog::sinks_init_list trans_sinks = {console_sink, trans_log_sink};

        // Create and register the logger for translator log and console.
        auto trans_logger = std::make_shared<spdlog::logger>("trans_logger", trans_sinks);
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