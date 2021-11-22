#include "helper_funcs.h"
#include "version_info.h"
#include "stream_buffering.h"

#ifndef TRANSLATE_1553_EXE_NAME
#error "translate_tabular_1553.cpp: TRANSLATE_1553_EXE_NAME must be defined"
#endif

int main(int argc, char* argv[])
{
    if (!SetLineBuffering(stdout))
        return 0;

    if (CheckForVersionArgument(argc, argv))
    {
        printf("%s version %s\n", TRANSLATE_1553_EXE_NAME, GetVersionString().c_str());
        return 0;
    }

    std::string str_input_path;
    std::string str_icd_path;
    std::string str_output_dir;
    std::string str_conf_dir;
    std::string str_log_dir;
    if (!ParseArgs(argc, argv, str_input_path, str_icd_path, str_output_dir,
                   str_conf_dir, str_log_dir))
        return 0;

    ManagedPath input_path;
    ManagedPath icd_path;
    ManagedPath output_dir;
    ManagedPath log_dir;
    ManagedPath conf_file_path;
    ManagedPath conf_schema_file_path;
    ManagedPath icd_schema_file_path;
    if (!ValidatePaths(str_input_path, str_icd_path, str_output_dir, str_conf_dir,
                       str_log_dir, input_path, icd_path, output_dir, conf_file_path, conf_schema_file_path,
                       icd_schema_file_path, log_dir))
        return 0;

    std::string config_doc;
    if (!ValidateConfSchema(conf_file_path, conf_schema_file_path, config_doc))
        return 0;

    if (!ValidateDTS1553YamlSchema(icd_path, icd_schema_file_path))
        return 0;

    TranslationConfigParams config_params;
    if (!config_params.InitializeWithConfigString(config_doc))
        return 0;

    if (!SetupLogging(log_dir))
        return 0;

    // Use logger to print and record these values after logging
    // is implemented.
    SPDLOG_INFO("{:s} version: {:s}", TRANSLATE_1553_EXE_NAME, GetVersionString());
    SPDLOG_INFO("Input: {:s}", input_path.RawString());
    SPDLOG_INFO("Output dir: {:s}", output_dir.RawString());
    SPDLOG_INFO("DTS1553 path: {:s}", icd_path.RawString());
    size_t thread_count = config_params.translate_thread_count_;
    SPDLOG_INFO("Thread count: {:d}", thread_count);

    DTS1553 dts1553;
    std::map<std::string, std::string> msg_name_substitutions;
    std::map<std::string, std::string> elem_name_substitutions;
    if (!IngestICD(dts1553, icd_path, msg_name_substitutions, elem_name_substitutions))
        return false;

    std::map<uint64_t, std::string> chanid_to_bus_name_map;
    std::set<uint64_t> excluded_channel_ids = std::set<uint64_t>();
    if (!PrepareBusMap(input_path, dts1553, config_params.stop_after_bus_map_,
                       config_params.prompt_user_, config_params.vote_threshold_,
                       config_params.vote_method_checks_tmats_, config_params.bus_name_exclusions_,
                       config_params.tmats_busname_corrections_, config_params.use_tmats_busmap_,
                       chanid_to_bus_name_map, excluded_channel_ids))
    {
        return 0;
    }

    if (config_params.auto_sys_limits_)
    {
        if (!SetSystemLimits(thread_count, dts1553.ICDDataPtr()->valid_message_count))
            return 0;
    }

    auto start_time = std::chrono::high_resolution_clock::now();
    ManagedPath transl_output_dir = output_dir.CreatePathObject(input_path, "_translated");
    if (!transl_output_dir.create_directory())
        return 0;
    ManagedPath output_base_name("");
    std::set<std::string> translated_msg_names;
    SPDLOG_INFO("Translated data output dir: {:s}", transl_output_dir.RawString());

    if (!Translate(thread_count, input_path, output_dir, dts1553.GetICDData(),
                   transl_output_dir, output_base_name, config_params.select_specific_messages_,
                   translated_msg_names))
    {
        SPDLOG_WARN(
            "Failed to configure 1553 translation stage or an error occurred "
            "during translation");
    }

    auto stop_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> secs = stop_time - start_time;
    double duration = secs.count();
    SPDLOG_INFO("Duration: {:.3f} sec", duration);

    RecordMetadata(config_params, transl_output_dir, icd_path, chanid_to_bus_name_map,
                   excluded_channel_ids, input_path, translated_msg_names,
                   msg_name_substitutions, elem_name_substitutions);

    return 0;
}
