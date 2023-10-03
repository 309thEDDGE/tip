#ifndef TRANSLATOR_CLI_HELP_STRINGS_1553_H_
#define TRANSLATOR_CLI_HELP_STRINGS_1553_H_

#include <string>

namespace Translate1553CLIHelpStrings
{
    const std::string help_request_help = 
        R"(Print usage information.)";

    const std::string version_request_help = 
        R"(Print version, either latest commit git SHA or tag.)";

    const std::string dts_request_help =
        R"(Print DTS1553 (Data Translation Specification for 1553) yaml file information.)";

    const std::string input_parsed_1553_help = 
        R"(Path to Parquet data to be translated, i.e., converted to EU. Path is
        "<ch10 name>_1553.parquet" directory which is output during tip_parse.)";

    const std::string input_dts1553_help =
        R"(Path to DTS1553 yaml file which contains ICD information relevant to the 
        PARSED_DATA)";

    const std::string output_path_help = 
        R"(Output directory for artifacts created at translate time)";

    const std::string log_dir_help = 
        R"(Log files output directory.)";

    const std::string translate_thread_count_help = 
        R"(Number of threads to use for translation valid ranges = [1, Cores * 2])";

    const std::string use_tmats_busmap_help =
        R"(Use tmats for mapping chapter 10 channel ids to bus names instead of the
        voting method explained in vote_threshold comments. 
        TMATS bus names must be exact matches with DTS bus names for successful
        bus mapping.)";

    const std::string tmats_busname_corrections_help = 
        R"(Corrects TMATS busname strings to the desired bus name using the provided 
        key-value mapping: key -> original TMATS bus name, value -> corrected bus name.
        The value must match comet specifications.
        One or more corrections may be provided in the format "<key>:<value>".
        Each map shall be preceded by the option flag: -T <key1>:<value1> -T <key2>:<value2> ... )";

    const std::string prompt_user_help =
        R"(Prompt the user for help with bus mapping if the automatically created
        map is incomplete.)";

    const std::string stop_after_bus_map_help =
        R"(Exit after auto-mapping. Can be used to debug
        mapping code, which prints mapping information to stdout.)"; 

    const std::string vote_threshold_help =
        R"(The vote_threshold is only used when use_tmats_busmap = false
        Bus mapping is used to associate a 1553 channel ID recorded in 
        the chapter 10 to a bus name found in the 1553 DTS file.
        One method is to use tmats, if it is correct, by setting 
        use_tmats_busmap: True, or by using a voting method by setting
        use_tmats_busmap: False.

        The bus map vote method uses the transmit/recieve command words
        off the 1553 bus and matches them to a transmit/recieve command
        word in the 1553 DTS file. If a transmit/recieve command word
        from a given channel ID matches a transmit/recieve command word
        from the 1553 DTS file, the channel ID votes for the bus/buses associated
        with the transmit/recieve command word from the 1553 DTS file. 
        (Note: word count is ignored from transmit/recieve command word matching)
        If there are a lot of different 1553 buses recorded in the chapter 10 
        and not enough command words provided through the 1553 DTS file AND
        not a lot of traffic exists on one of the buses, there is a chance
        that the channel ID with lower traffic could be mapped to the wrong bus.
        vote_threshold can be used to help prevent this issue. To create a final 
        channel ID -> bus name mapping, the bus votes from a given channel ID
        must be >= vote_threshold. vote_threshold is also helpful if DTS bus names
        are not substrings or equivalent to TMATS bus names and vote_method_checks_tmats = false.

        A good threshold can be found from trial and error. It greatly depends
        on how complete the 1553 DTS file is and the amount of bus traffic
        on each bus. Valid ranges are [1, INT_MAX].)";

    const std::string vote_method_checks_tmats_help =
        R"(Only used when --use_tmats_busmap is not given. 
    Once the voting process is carried out, the results will be compared 
    with TMATS. If a bus name associated with a given channel ID is a substring 
    of the TMATS bus name for that given channel ID, it will allow the channel 
    ID to be mapped. If a TMATS match is not made, it will not map the channel 
    ID that doesn't have a TMATS bus name match.**||
        
    Example 1:
    voting method results--
    channel ID = 1,  bus name = "BUS1"
    --
    tmats--
    channel ID = 1, bus name = "BUS1" or "BUS1MUX" or "*ANYTHING*BUS1*ANYTHING*"
    -- 
    final bus map (channel ID = 1 -> "BUS1")

    Example 2:
    voting method results--
    channel ID = 1,  bus name = "BUS1"
    --
    tmats results--
    channel ID = 1, bus name = "BUS2" or missing from TMATS
    -- 
    final bus map (channel ID 1 will be left unmapped)**|| )";

    const std::string bus_name_exclusions_help =
        R"(If a bus name exists in the final bus map that also
    exists in the bus_name_exclusions list. It will be removed 
    from the final bus map. Removals include the case when 
    a bus from the bus_name_exclusions list is a substring of the full bus name
    found in either the DTS file or TMATS.**|| 
    Example: TMATs/DTS = "BUSAMUX",  
    "--busname_exclusion BUSA", will result in removal
    of BUSAMUX from the final bus map 

    Removals are also not case sensitive 
    Example: TMATs/DTS -> "BusA",  
    "--busname_exclusion BUSA", will result in removal
    of BusA from the final bus map.**|| )";

    const std::string select_specific_messages_help =
        R"(If values are provided, only messages which match provided values will be translated.
        All other messages will be ignored. Pass multiple instances of the option to
        input multiple values: "--select_msg msgA" or "-m msgA -m msgB")";

    const std::string halt_after_table_creation_help =
        R"(Halt translation routine and exit after table
        creation. Useful for debugging message table creation.)";

    const std::string auto_sys_limits_help = 
        R"(Set file descriptor (FD) limits at runtime to allow
        3 + m + n x m total FDs, where n = count of 1553 messages defined in DTS
        file (ICD expressed in custom yaml format) and m = thread count set by
        "translate_thread_count" parameter, defined above. Each thread may
        open up to n messages simultaneously (n x m term) and each thread
        will open a single parsed 1553 parquet at a time (m term). Also
        account for stdin, stdout and stderr (+3).)"; 

    const std::string stdout_log_level_help = 
        R"(Set log level of sink to minimum log level. All log entries
    with level value greater or equal to minimum level value will
    be printed to stdout.
    **||
    Log level and value:
    - trace     = 0
    - debug     = 1
    - info      = 2
    - warn      = 3
    - error     = 4
    - critical  = 5
    - off       = 6

    **||Accept values in the set {trace, debug, info, warn, error, critical, off}.)";
}

#endif  // TRANSLATOR_CLI_HELP_STRINGS_1553_H_