#include "parser_paths.h"

ParserPaths::ParserPaths() : ch10_input_path_(""), output_dir_(""), 
    tmats_output_path_(""), tdp_output_path_("")
{}

bool ParserPaths::CreateOutputPaths(const ManagedPath& ch10_input_path,
    const ManagedPath& output_dir, const std::map<Ch10PacketType, bool>& packet_enabled_map,
    uint16_t total_worker_count)
{
    ch10_input_path_ = ch10_input_path;
    output_dir_ = output_dir;
    pkt_type_enabled_map_ = packet_enabled_map;

    tmats_output_path_ = output_dir.CreatePathObject(ch10_input_path, "_" + 
        ch10packettype_to_string_map.at(Ch10PacketType::COMPUTER_GENERATED_DATA_F1) + ".txt");
    tdp_output_path_ = output_dir.CreatePathObject(ch10_input_path, "_" + 
       ch10packettype_to_string_map.at(Ch10PacketType::TIME_DATA_F1) + ".parquet");
    spdlog::get("pm_logger")->info("TMATS output path: {:s}", tmats_output_path_.RawString());
    spdlog::get("pm_logger")->info("Time data output path: {:s}", tdp_output_path_.RawString());

    if(!CreateCh10PacketOutputDirs(output_dir_, ch10_input_path_, pkt_type_enabled_map_,
        pkt_type_output_dir_map_, true))
        return false;

    CreateCh10PacketWorkerFileNames(total_worker_count, pkt_type_output_dir_map_,
        worker_path_vec_, "parquet");

    return true;
}

bool ParserPaths::CreateCh10PacketOutputDirs(const ManagedPath& output_dir,
                                              const ManagedPath& base_file_name,
                                              const std::map<Ch10PacketType, bool>& packet_enabled_map,
                                              std::map<Ch10PacketType, ManagedPath>& pkt_type_output_dir_map, 
                                              bool create_dir)
{
    // Check for present of append_str_map entry for each packet_enabled_map
    // entry which maps to true.
    std::map<Ch10PacketType, bool>::const_iterator it;
    bool result = false;
    ManagedPath pkt_type_output_dir;
    for (it = packet_enabled_map.cbegin(); it != packet_enabled_map.cend(); ++it)
    {
        if (it->second)
        {
            if (ch10packettype_to_string_map.count(it->first) == 0)
            {
                spdlog::get("pm_logger")->warn(
                    "CreateCh10PacketOutputDirs: No append "
                    "string map entry for {:s}",
                    ch10packettype_to_string_map.at(it->first));
                return false;
            }

            // Fill pkt_type_output_dir_map for each packet type in packet_enabled_map.
            result = ManagedPath::CreateDirectoryFromComponents(output_dir, base_file_name,
                "_" + ch10packettype_to_string_map.at(it->first) + ".parquet", 
                pkt_type_output_dir, create_dir);
            if (!result)
            {
                pkt_type_output_dir_map.clear();
                spdlog::get("pm_logger")->error("CreateCh10PacketOutputDirs: Failed to "
                    "create directory: {:s}", pkt_type_output_dir.RawString());
                return false;
            }
            spdlog::get("pm_logger")->info("CreateCh10PacketOutputDirs: Create {:s} output dir: {:s}", 
                ch10packettype_to_string_map.at(it->first), pkt_type_output_dir.RawString());
            pkt_type_output_dir_map[it->first] = pkt_type_output_dir;
        }
    }

    return true;
}

void ParserPaths::CreateCh10PacketWorkerFileNames(const uint16_t& total_worker_count,
    const std::map<Ch10PacketType, ManagedPath>& pkt_type_output_dir_map,
    std::vector<std::map<Ch10PacketType, ManagedPath>>& output_vec_mapped_paths,
    std::string file_extension)
{
    std::string replacement_ext = "";
    std::map<Ch10PacketType, ManagedPath>::const_iterator it;

    for (uint16_t worker_index = 0; worker_index < total_worker_count; worker_index++)
    {
        // Create the replacement extension for the current index.
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(3) << worker_index;
        if (file_extension != "")
        {
            ss << "." << file_extension;
        }
        replacement_ext = ss.str();

        // Create a temporary map to hold all of the output file paths for the
        // current index.
        std::map<Ch10PacketType, ManagedPath> temp_output_file_map;

        // Add an output file path for each Ch10PacketType and output dir
        // in pkt_type_output_dir_map.
        for (it = pkt_type_output_dir_map.cbegin(); it != pkt_type_output_dir_map.cend(); ++it)
        {
            temp_output_file_map[it->first] = it->second / replacement_ext;
        }

        // Add the temp map to the vector maps if the temp map has items.
        if (temp_output_file_map.size() > 0)
            output_vec_mapped_paths.push_back(temp_output_file_map);
    }
}


bool ParserPaths::RemoveCh10PacketOutputDirs(const std::set<Ch10PacketType>& parsed_packet_types)
{
    return RemoveCh10PacketOutputDirs(pkt_type_output_dir_map_, parsed_packet_types);
}

bool ParserPaths::RemoveCh10PacketOutputDirs(const std::map<Ch10PacketType, ManagedPath>& output_dir_map,
    const std::set<Ch10PacketType>& parsed_packet_types)
{
    std::string packet_type_name = "";
    bool retval = true;
    for (std::map<Ch10PacketType, ManagedPath>::const_iterator it = output_dir_map.cbegin();
        it != output_dir_map.cend(); ++it)
    {
        packet_type_name = ch10packettype_to_string_map.at(it->first);
        if (it->second.is_directory())
        {
            if(parsed_packet_types.count(it->first) == 0)
            {
                spdlog::get("pm_logger")->info("Removing unused {:s} dir: {:s}",
                    packet_type_name, it->second.RawString());
                if(!it->second.remove())
                {
                    spdlog::get("pm_logger")->warn("Failed to remove {:s} dir: {:s}",
                        packet_type_name, it->second.RawString());
                    retval = false;
                }
            }
        }
        else
        {
            spdlog::get("pm_logger")->warn("Expected output {:s} dir does not exist: {:s}",
                packet_type_name, it->second.RawString());
        }
    }
    return retval;
}