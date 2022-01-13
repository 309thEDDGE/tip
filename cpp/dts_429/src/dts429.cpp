#include "dts429.h"


bool DTS429::OpenYamlFile(const ManagedPath& dts_path,
                        std::map<std::string, std::string>& wrd_name_substitutions,
                        std::map<std::string, std::string>& elem_name_substitutions)
{
    // Check if yaml or text file
    bool is_yaml = icd_data_.IsYamlFile(dts_path);

    if (is_yaml)
    {
        SPDLOG_INFO("DTS429: Handling yaml file data");

        // Open input file and build vector<string> of it's lines
        std::ifstream fin(dts_path);
        std::string str;
        if(!fin)
        {
            SPDLOG_INFO("DTS429: Cannot open file at path\n");
            return false;
        }
        while (std::getline(fin, str))
        {
            // remove newline from string
            str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());

            if(str.size() > 0)
            {
                yaml_lines_.push_back(str);
            }
        }
        fin.close();

        // Ingest Lines from input file
        if(!IngestLines(yaml_lines_, wrd_name_substitutions,
        elem_name_substitutions))
        {
            SPDLOG_INFO("DTS::OpenYamlFile(): Faild. IngestLines failure\n");
            return false;
        }

    }
    else
    {
        printf("DTS429::OpenYamlFile(): Faild. dts_path is not a yaml file\n");
        return false;
    }

    return true;
}

bool DTS429::IngestLines(const std::vector<std::string>& lines,
                          std::map<std::string, std::string>& wrd_name_substitutions,
                          std::map<std::string, std::string>& elem_name_substitutions)

{
    printf("DTS429::IngestLines(): Handling yaml file data\n");

    // Obtain each DTS429 component as a yaml node.
    YAML::Node msg_defs;
    YAML::Node suppl_busmap;
    if (!ProcessLinesAsYaml(lines, msg_defs, suppl_busmap))
    {
        printf("DTS429::IngestLines(): Process yaml lines failure!\n");
        return false;
    }

    // if (!icd_data_.PrepareICDQuery(msg_defs, wrd_name_substitutions,  // big TODO here
    //                                 elem_name_substitutions))
    // {
    //     printf("DTS429::IngestLines(): PrepareICDQuery failure!\n");
    //     return false;
    // }

    // If the supplemental bus map labels node has a size greater
    // than zero, fill the private member map.
    if (!FillSupplBusNameToWordKeyMap(suppl_busmap, suppl_bus_name_to_word_key_map_))
    {
        printf("DTS429::IngestLines(): Failed to generate bus name to message key map!\n");
        return false;
    }

    return true;
}

bool DTS429::ProcessLinesAsYaml(const std::vector<std::string>& lines,
                                 YAML::Node& transl_wrd_defs_node,
                                 YAML::Node& suppl_busmap_labels_node)
{
    // Bad if there are zero lines.
    if (lines.size() == 0)
    {
        printf("DTS429::ProcessLinesAsYaml(): Input lines vector has size 0\n");
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
        printf("DTS429::ProcessLinesAsYaml(): Root note has size 0\n");
        return false;
    }

    // Root node must be a map because all root-level items are maps.
    if (!root_node.IsMap())
    {
        printf("DTS429::ProcessLinesAsYaml(): Root node is not a map\n");
        return false;
    }

    // The word definitions map MUST be present.
    std::string key_name = "";
    bool word_definitions_exist = false;
    for (YAML::const_iterator it = root_node.begin(); it != root_node.end(); ++it)
    {
        key_name = it->first.as<std::string>();
        if (yaml_key_to_component_map_.count(key_name) != 0)
        {
            switch (yaml_key_to_component_map_.at(key_name))
            {
                case DTS429Component::TRANSL_WORD_DEFS:
                    word_definitions_exist = true;
                    transl_wrd_defs_node = it->second;
                    break;
                case DTS429Component::SUPPL_BUSMAP_LABELS:
                    suppl_busmap_labels_node = it->second;
                    break;
            }
        }
    }

    if (!word_definitions_exist)
    {
        printf("DTS429::ProcessLinesAsYaml(): Word definitions node not present!\n");
        return false;
    }

    return true;
}

bool DTS429::FillSupplBusNameToWordKeyMap(const YAML::Node& suppl_busmap_labels_node,
                                          std::map<std::string, std::set<uint32_t>>& output_suppl_busname_to_wrd_key_map)
{
    if (suppl_busmap_labels_node.size() == 0)
        return true;

    // Root node must be a map.
    if (!suppl_busmap_labels_node.IsMap())
    {
        printf("DTS429::FillSupplBusNameToWrdKeyMap(): Root node is not a map\n");
        return false;
    }

    std::string bus_name = "";
    std::vector<uint32_t> arinc_labels;
    for (YAML::Node::const_iterator busname_map = suppl_busmap_labels_node.begin();
         busname_map != suppl_busmap_labels_node.end(); ++busname_map)
    {
        // Fail if the value part of each mapping is not a sequence.
        if (!busname_map->second.IsSequence())
        {
            printf("DTS429::FillSupplBusNameToMsgKeyMap(): Value of mapping is not a sequence\n");
            return false;
        }

        // Iterate over the integers in the bus name sequence map to check type is int.
        YAML::Node labels_seq = busname_map->second;
        for (int labels_set_ind = 0; labels_set_ind < labels_seq.size();
             labels_set_ind++)
        {
            // Fail if the item in the sequence is not an int.
            try
            {
                if (typeid(labels_seq[labels_set_ind].as<int>()) != typeid(int()))  throw 1;
            }
            catch (...)
            {
                // catch case of error in 'casting' to int
                printf(
                    "DTS429::FillSupplBusNameToWrdKeyMap(): "
                    "Sequence item is not an integer\n");
                return false;
            }
        }

        // Build the output map.
        bus_name = busname_map->first.as<std::string>();
        arinc_labels = labels_seq.as<std::vector<uint32_t>>();
        for(int i = 0; i < arinc_labels.size(); i++){
            output_suppl_busname_to_wrd_key_map[bus_name].insert(arinc_labels[i]);
        }

    }
    return true;
}

bool ManageParseMetadata(TIPMDDocument& parser_md_doc)
{
    // build yaml nodes from parser_md_doc

    // use yaml_reader.cpp to Ingest from string and GetMapNodeParameter()?? Try this

    // if no data, return false

    // pass node tmats_chanid_to_429_subchan_and_name into subchannel mapping tool


    return false;
}
