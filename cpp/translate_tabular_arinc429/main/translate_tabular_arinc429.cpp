
#ifndef TRANSLATE_429_EXE_NAME
#error "translate_tabular_arinc429.cpp: TRANSLATE_429_EXE_NAME must be defined"
#endif

#include <chrono>
#include "helper_funcs.h"
#include "version_info.h"
#include "stream_buffering.h"
#include "provenance_data.h"
#include "sha256_tools.h"
#include "argument_validation.h"
#include "translation_config_params.h"
#include "yaml_schema_validation.h"

int main(int argc, char* argv[])
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
    if (!ValidatePaths(args.at("input_path"), args.at("icd_path"), args.at("output_dir"), 
                       args.at("conf_dir"), args.at("log_dir"), input_path, icd_path, 
                       output_dir, conf_file_path, conf_schema_file_path,
                       icd_schema_file_path, log_dir))
        return 0;

    if (!SetupLogging(log_dir))
        return 0;

    TIPMDDocument parser_md_doc;
    ManagedPath parser_md_path = input_path / "_metadata.yaml";
    if(!GetParsedMetadata(parser_md_path, parser_md_doc))
        return false;

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
        if (!SetSystemLimits(thread_count, arinc429_message_count))
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
