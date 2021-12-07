#include "dts1553.h"

bool DTS1553::IngestLines(const ManagedPath& dts_path, const std::vector<std::string>& lines,
                          std::map<std::string, std::string>& msg_name_substitutions,
                          std::map<std::string, std::string>& elem_name_substitutions)

{
    // Check if yaml or text file.
    bool is_yaml = icd_data_.IsYamlFile(dts_path);

    // If yaml file, interpret lines as yaml and handle each component
    // with intended object. Otherwise pass all lines to ICDData.
    if (is_yaml)
    {
        SPDLOG_INFO("DTS1553: Handling yaml file data");

        // Obtain each DTS1553 component as a yaml node.
        YAML::Node msg_defs;
        YAML::Node suppl_busmap;
        if (!ProcessLinesAsYaml(lines, msg_defs, suppl_busmap))
        {
            SPDLOG_ERROR("DTS1553: Process yaml lines failure!");
            return false;
        }

        if (!icd_data_.PrepareICDQuery(msg_defs, msg_name_substitutions,
                                       elem_name_substitutions))
        {
            SPDLOG_ERROR("DTS1553: PrepareICDQuery failure!");
            return false;
        }

        // If the supplemental bus map command words node has a size greater
        // than zero, fill the private member map.
        if (!FillSupplBusNameToMsgKeyMap(suppl_busmap, suppl_bus_name_to_message_key_map_))
        {
            SPDLOG_ERROR("DTS1553: Failed to generate bus name to message key map!");
            return false;
        }
    }
    else
    {
        SPDLOG_INFO("DTS1553: Handling text/csv file data");
        if (!icd_data_.PrepareICDQuery(lines))
        {
            SPDLOG_ERROR("DTS1553: Failed to parse input lines!");
            return false;
        }
    }

    return true;
}

bool DTS1553::ProcessLinesAsYaml(const std::vector<std::string>& lines,
                                 YAML::Node& transl_msg_defs_node,
                                 YAML::Node& suppl_busmap_comm_words_node)
{
    // Bad if there are zero lines.
    if (lines.size() == 0)
    {
        SPDLOG_WARN("DTS1553: Input lines vector has size 0");
        return false;
    }

    // Concatenate all lines into a single string. It is requisite to append
    // to each line the newline character. Yaml loader must see new lines to
    // understand context.
    std::stringstream ss;
    std::for_each(lines.begin(), lines.end(),
                  [&ss](const std::string& s) {
                      ss << s;
                      ss << "\n";
                  });
    std::string all_lines = ss.str();

    YAML::Node root_node = YAML::Load(all_lines.c_str());

    // Root node must have at least one entry.
    if (root_node.size() == 0)
    {
        SPDLOG_WARN("DTS1553: Root note has size 0");
        return false;
    }

    // Root node must be a map because all root-level items are maps.
    if (!root_node.IsMap())
    {
        SPDLOG_ERROR("DTS1553: Root node is not a map");
        return false;
    }

    // The message definitions map MUST be present.
    std::string key_name = "";
    bool message_definitions_exist = false;
    for (YAML::const_iterator it = root_node.begin(); it != root_node.end(); ++it)
    {
        key_name = it->first.as<std::string>();
        if (yaml_key_to_component_map_.count(key_name) != 0)
        {
            switch (yaml_key_to_component_map_.at(key_name))
            {
                case DTS1553Component::TRANSL_MESSAGE_DEFS:
                    message_definitions_exist = true;
                    transl_msg_defs_node = it->second;
                    break;
                case DTS1553Component::SUPPL_BUSMAP_COMM_WORDS:
                    suppl_busmap_comm_words_node = it->second;
                    break;
            }
        }
    }

    if (!message_definitions_exist)
    {
        SPDLOG_ERROR("DTS1553: Message definitions node not present!");
        return false;
    }

    return true;
}

bool DTS1553::FillSupplBusNameToMsgKeyMap(const YAML::Node& suppl_busmap_comm_words_node,
                                          std::map<std::string, std::set<uint64_t>>& output_suppl_busname_to_msg_key_map)
{
    if (suppl_busmap_comm_words_node.size() == 0)
        return true;

    // Root node must be a map.
    if (!suppl_busmap_comm_words_node.IsMap())
    {
        SPDLOG_ERROR("DTS1553: Root node is not a map");
        return false;
    }

    std::string bus_name = "";
    // Use uint64_t to avoid the need for casting prior to upshifting the original
    // 16-bit value by 16 bits.
    std::vector<uint64_t> tx_rx_comm_words;
    for (YAML::Node::const_iterator busname_map = suppl_busmap_comm_words_node.begin();
         busname_map != suppl_busmap_comm_words_node.end(); ++busname_map)
    {
        // Fail if the value part of each mapping is not a sequence.
        if (!busname_map->second.IsSequence())
        {
            SPDLOG_ERROR("DTS1553: Value of mapping is not a sequence");
            return false;
        }

        // Iterate over the sequence in the bus name map.
        YAML::Node comm_words_seq = busname_map->second;
        for (int comm_words_set_ind = 0; comm_words_set_ind < comm_words_seq.size();
             comm_words_set_ind++)
        {
            // Fail if the item in the sequence is itself not a sequence.
            if (!comm_words_seq[comm_words_set_ind].IsSequence())
            {
                SPDLOG_ERROR("DTS1553: Sequence item is not itself a sequence");
                return false;
            }

            tx_rx_comm_words = comm_words_seq[comm_words_set_ind].as<std::vector<uint64_t>>();

            // Command words sequence must have two values.
            if (tx_rx_comm_words.size() != 2)
            {
                printf("DTS1553: Command words sequence does not have exactly two values");
                return false;
            }

            // Build the output map.
            bus_name = busname_map->first.as<std::string>();
            if (output_suppl_busname_to_msg_key_map.count(bus_name) == 0)
            {
                std::set<uint64_t> temp_msg_key_set(
                    {(tx_rx_comm_words[0] << 16) + tx_rx_comm_words[1]});
                output_suppl_busname_to_msg_key_map[bus_name] = temp_msg_key_set;
            }
            else
            {
                output_suppl_busname_to_msg_key_map[bus_name].insert(
                    (tx_rx_comm_words[0] << 16) + tx_rx_comm_words[1]);
            }
        }
    }
    return true;
}