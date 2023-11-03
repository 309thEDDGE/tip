#ifndef TRANSLATOR_CLI_HELP_STRINGS_429_H_
#define TRANSLATOR_CLI_HELP_STRINGS_429_H_

#include <string>

namespace TranslateARINC429CLIHelpStrings
{
    const std::string help_request_help = 
        R"(Print usage information.)";

    const std::string version_request_help = 
        R"(Print version, either latest commit git SHA or tag.)";

    const std::string dts_request_help =
        R"(Print DTS429 (Data Translation Specification for ARINC429) yaml file information.)";

    const std::string input_parsed_data_help = 
        R"(Path to Parquet data to be translated, i.e., converted to EU. Path is
        "<ch10 name>_arinc429.parquet" directory which is output during tip_parse.)";

    const std::string input_dts429_help =
        R"(Path to DTS429 yaml file which contains ICD information relevant to the 
        PARSED_DATA)";

    const std::string output_path_help = 
        R"(Output directory for artifacts created at translate time)";

    const std::string log_dir_help = 
        R"(Log files output directory.)";

    const std::string translate_thread_count_help = 
        R"(Number of threads to use for translation valid ranges = [1, Cores * 2])";

    const std::string auto_sys_limits_help = 
        R"(Set file descriptor (FD) limits at runtime to allow
        3 + m + n x m total FDs, where n = count of 1553 messages defined in DTS
        file (ICD expressed in custom yaml format) and m = thread count set by
        "translate_thread_count" parameter, defined above. Each thread may
        open up to n messages simultaneously (n x m term) and each thread
        will open a single parsed 1553 parquet at a time (m term). Also
        account for stdin, stdout and stderr (+3).)"; 

    const std::string stdout_log_level_help = 
        R"(Set log level of stdout to minimum log level. All log entries
    with level value greater or equal to minimum level value will
    be printed to stdout. 

    Log level and value:
    - trace     = 0
    - debug     = 1
    - info      = 2
    - warn      = 3
    - error     = 4
    - critical  = 5
    - off       = 6

    Accept values in the set {trace, debug, info, warn, error, critical, off}.)";

    const std::string file_log_level_help = 
        R"(Set log level of file logs to minimum log level. All log entries
    with level value greater or equal to minimum level value will
    be printed to stdout. 

    Log level and value:
    - trace     = 0
    - debug     = 1
    - info      = 2
    - warn      = 3
    - error     = 4
    - critical  = 5
    - off       = 6

    Accept values in the set {trace, debug, info, warn, error, critical, off}.)";

    }
#endif  // TRANSLATOR_CLI_HELP_STRINGS_429_H_