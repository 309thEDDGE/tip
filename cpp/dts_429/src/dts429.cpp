#include "dts429.h"


// bool DTS429::OpenYamlFile(const ManagedPath& dts_path,
//                         std::map<std::string, std::string>& wrd_name_substitutions,
//                         std::map<std::string, std::string>& elem_name_substitutions)
// {
//     // Check if yaml or text file
//     bool is_yaml = icd_data_.IsYamlFile(dts_path);

//     if (is_yaml)
//     {
//         SPDLOG_INFO("DTS429: Handling yaml file data");

//         // Open input file and build vector<string> of it's lines
//         std::ifstream fin(dts_path);
//         std::string str;
//         if(!fin)
//         {
//             SPDLOG_INFO("DTS429: Cannot open file at path\n");
//             return false;
//         }
//         while (std::getline(fin, str))
//         {
//             // remove newline from string
//             str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());

//             if(str.size() > 0)
//             {
//                 yaml_lines_.push_back(str);
//             }
//         }
//         fin.close();

//         // Ingest Lines from input file
//         if(!IngestLines(yaml_lines_, wrd_name_substitutions,
//         elem_name_substitutions))
//         {
//             SPDLOG_INFO("DTS429::OpenYamlFile(): Faild. IngestLines failure\n");
//             return false;
//         }

//     }
//     else
//     {
//         SPDLOG_WARN("DTS429::OpenYamlFile(): Faild. dts_path is not a yaml file\n");
//         return false;
//     }

//     return true;
// }

bool DTS429::IngestLines(const std::vector<std::string>& lines,
                      std::unordered_map<std::string, std::vector<ICDElement>> word_elements)
{
    SPDLOG_WARN("DTS429::IngestLines(): Handling yaml file data\n");

    if(lines.empty())
    {
        SPDLOG_WARN("DTS429::IngestLines(): const std::vector<std::string>& lines is empty");
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

    // Root node must have entry for translatable_word_definitions and supplemental_bus_map_labels.
    if (root_node.size() < 2)
    {
        SPDLOG_WARN("DTS429::IngestLines(): Root note has size 0\n");
        return false;
    }

    // Root node must be a map because all root-level items are maps.
    if (!root_node["translatable_word_definitions"].IsMap() ||
        !root_node["supplemental_bus_map_labels"].IsMap())
    {
        SPDLOG_WARN("DTS429::IngestLines(): Root node is not a map\n");
        return false;
    }


    // Obtain each DTS429 component as a yaml node.
    YAML::Node wrd_defs;
    YAML::Node suppl_busmap;
    if (!ProcessLinesAsYaml(root_node, wrd_defs, suppl_busmap))
    {
        SPDLOG_WARN("DTS429::IngestLines(): Process yaml lines failure!\n");
        return false;
    }

    // // If the supplemental bus map labels node has a size greater
    // // than zero, fill the private member map.
    // if (!FillSupplBusNameToWordKeyMap(suppl_busmap, suppl_bus_name_to_word_key_map_))
    // {
    //     SPDLOG_WARN("DTS429::IngestLines(): Failed to generate bus name to message key map!\n");
    //     return false;
    // }

    return true;
}

bool DTS429::BuildNameToICDElementMap(YAML::Node& transl_wrd_defs_node,
                std::unordered_map<std::string, std::vector<ICDElement>>& word_elements)
{
    if(transl_wrd_defs_node.IsNull()) //==YAML::NodeType::Null)
    {
        SPDLOG_WARN("DTS429::BuildNameToICDElementMap(): Argument transl_wrd_defs_node"
                    " is null.");
        return false;
    }
    // The word definitions map MUST be present.
    std::string key_name = "";
    bool word_definitions_exist = false;
    ICDElement icd_element;
    YAML::Node word_data;
    YAML::Node elem_data;
    for (YAML::const_iterator it = transl_wrd_defs_node.begin(); it != transl_wrd_defs_node.end(); ++it)
    {
        key_name = it->first.as<std::string>();

        if(!ValidateWordNode(it->second))
            return false;

        word_data = (it->second)["wrd_data"];
        elem_data = (it->second)["elem"];

        // CreateICDElementFromWordNode(it)

        // Iterate over elem data -

        //      pass into builder for ICD element
        //      Add element to the map
    }
    return true;
}


bool DTS429::ValidateWordNode(const YAML::Node& word_node)
{
    if(!word_node.IsMap())
    {
        SPDLOG_WARN("word_node is not a map");
        return false;
    }

    if(!word_node["wrd_data"])
    {
        SPDLOG_WARN("word_node is missing \"wrd_data\" key");
        return false;
    }

    if(!word_node["elem"])
    {
        SPDLOG_WARN("word_node is missing \"elem\" key");
        return false;
    }

    return true;
}

bool DTS429::CreateICDElementFromWordNodes(const std::string& msg_name,
                                           const std::string& elem_name,
                                           const YAML::Node& wrd_data,
                                           const YAML::Node& elem_data,
                                           ICDElement& arinc_param)
{
    // 8-bit fields are accessed as 16-bit then cast to 8-bit
    // because it was found that accessing directly as 8 bit
    // caused the value at key to be stored as the ASCII binary
    // value rather than integer's value
    try
    {
        arinc_param.label_=wrd_data["label"].as<uint16_t>();
        arinc_param.sdi_=(int8_t)wrd_data["sdi"].as<int16_t>();
        arinc_param.bus_name_=wrd_data["bus"].as<std::string>();
        arinc_param.msg_name_= msg_name;
        arinc_param.rate_=wrd_data["rate"].as<float>();
        arinc_param.xmit_lru_name_=wrd_data["lru_name"].as<std::string>();
        arinc_param.description_=elem_data["desc"].as<std::string>();
        arinc_param.elem_name_=elem_name;
        arinc_param.schema_=StringToICDElementSchemaMap.find(
                elem_data["schema"].as<std::string>())->second;
        arinc_param.is_bitlevel_=true;
        arinc_param.bcd_partial_=-1;
        arinc_param.msb_val_=elem_data["msbval"].as<float>();
        arinc_param.bitlsb_=(uint8_t)elem_data["lsb"].as<uint16_t>();
        arinc_param.bit_count_=(uint8_t)elem_data["bitcnt"].as<uint16_t>();
        arinc_param.uom_=elem_data["uom"].as<std::string>();
        arinc_param.classification_=(uint8_t)elem_data["class"].as<uint16_t>();
    }
    catch(...)
    {
        SPDLOG_WARN("DTS429::CreateICDElementFromWordNodes(): Error "
            "bulding ICDElement from Nodes!");
        return false;
    }
    return true;
}

bool DTS429::ProcessLinesAsYaml(const YAML::Node& root_node,
                                YAML::Node& transl_wrd_defs_node,
                                YAML::Node& suppl_busmap_labels_node)
{

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
        SPDLOG_WARN("DTS429::ProcessLinesAsYaml(): Word definitions "
            "node not present!\n");
        return false;
    }

    return true;
}



// bool DTS429::FillSupplBusNameToWordKeyMap(const YAML::Node& suppl_busmap_labels_node,
//                                           std::map<std::string, std::set<uint32_t>>& output_suppl_busname_to_wrd_key_map)
// {
//     if (suppl_busmap_labels_node.size() == 0)
//         return true;

//     // Root node must be a map.
//     if (!suppl_busmap_labels_node.IsMap())
//     {
//         SPDLOG_WARN("DTS429::FillSupplBusNameToWrdKeyMap(): Root node is not a map\n");
//         return false;
//     }

//     std::string bus_name = "";
//     std::vector<uint32_t> arinc_labels;
//     for (YAML::Node::const_iterator busname_map = suppl_busmap_labels_node.begin();
//          busname_map != suppl_busmap_labels_node.end(); ++busname_map)
//     {
//         // Fail if the value part of each mapping is not a sequence.
//         if (!busname_map->second.IsSequence())
//         {
//             SPDLOG_WARN("DTS429::FillSupplBusNameToMsgKeyMap(): Value of mapping is not a sequence\n");
//             return false;
//         }

//         // Iterate over the integers in the bus name sequence map to check type is int.
//         YAML::Node labels_seq = busname_map->second;
//         for (int labels_set_ind = 0; labels_set_ind < labels_seq.size();
//              labels_set_ind++)
//         {
//             // Fail if the item in the sequence is not an int.
//             try
//             {
//                 if (typeid(labels_seq[labels_set_ind].as<int>()) != typeid(int()))  throw 1;
//             }
//             catch (...)
//             {
//                 // catch case of error in 'casting' to int
//                 SPDLOG_WARN(
//                     "DTS429::FillSupplBusNameToWrdKeyMap(): "
//                     "Sequence item is not an integer\n");
//                 return false;
//             }
//         }

//         // Build the output map.
//         bus_name = busname_map->first.as<std::string>();
//         arinc_labels = labels_seq.as<std::vector<uint32_t>>();
//         for(int i = 0; i < arinc_labels.size(); i++){
//             output_suppl_busname_to_wrd_key_map[bus_name].insert(arinc_labels[i]);
//         }

//     }
//     return true;
// }
