#include "translator_helper_funcs.h"
#include "version_info.h"

#ifndef TRANSLATE_1553_EXE_NAME
#error "translate_1553_from_parquet.cpp: TRANSLATE_1553_EXE_NAME must be defined"
#endif

int main(int argc, char* argv[])
{
    printf("%s version %s\n", TRANSLATE_1553_EXE_NAME, GetVersionString().c_str());
    if(CheckForVersionArgument(argc, argv))
        return 0;

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
    printf("Input: %s\n", input_path.RawString().c_str());
    printf("Output dir: %s\n", output_dir.RawString().c_str());
    printf("DTS1553 path: %s\n", icd_path.RawString().c_str());
    uint8_t thread_count = config_params.translate_thread_count_;
    printf("Thread count: %hhu\n", thread_count);

    DTS1553 dts1553;
    std::map<std::string, std::string> msg_name_substitutions;
    std::map<std::string, std::string> elem_name_substitutions;
    if(!IngestICD(dts1553, icd_path, msg_name_substitutions, elem_name_substitutions))
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

    if(config_params.auto_sys_limits_)
    {
        if(!SetSystemLimits(thread_count, dts1553.ICDDataPtr()->valid_message_count))
            return 0;
    }

    double duration = 0.0;
    ManagedPath translated_data_dir;
    std::set<std::string> translated_message_names;
    if (thread_count > 0)
    {
        MTTranslate(config_params, input_path, output_dir, dts1553.GetICDData(),
                    duration, translated_data_dir, translated_message_names);
    }

    RecordMetadata(config_params, translated_data_dir, icd_path, chanid_to_bus_name_map,
                   excluded_channel_ids, input_path, translated_message_names, 
                   msg_name_substitutions, elem_name_substitutions);


    //system("pause");
    return 0;
}
