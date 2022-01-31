#include "dts429.h"


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
        SPDLOG_WARN("DTS429::IngestLines(): translatable_word_definitions"
        " or supplemental_bus_map_labels likely missing from file\n");
        return false;
    }

    // Root node must be a map because all root-level items are maps.
    if (!root_node["translatable_word_definitions"].IsMap())
    {
        SPDLOG_WARN("DTS429::IngestLines(): translatable_word_definitions is not a map\n");
        return false;
    }
    if ((root_node["translatable_word_definitions"]).size() < 1)
    {
        SPDLOG_WARN("DTS429::IngestLines(): translatable_word_definitions is an empty map\n");
        return false;
    }
    if(!root_node["supplemental_bus_map_labels"].IsMap())
    {
        SPDLOG_WARN("DTS429::IngestLines(): supplemental_bus_map_labels is not a map\n");
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

    if(!BuildNameToICDElementMap(wrd_defs, word_elements))
        return false;

    return true;
}

bool DTS429::BuildNameToICDElementMap(YAML::Node& transl_wrd_defs_node,
                std::unordered_map<std::string, std::vector<ICDElement>>& word_elements)
{
    if(transl_wrd_defs_node.IsNull())
    {
        SPDLOG_WARN("DTS429::BuildNameToICDElementMap(): Argument transl_wrd_defs_node"
                    " is null.");
        return false;
    }
   if(!transl_wrd_defs_node.IsMap())
    {
        SPDLOG_WARN("DTS429::BuildNameToICDElementMap(): Argument transl_wrd_defs_node"
                    " doesn't contain a map.");
        return false;
    }

    // The word definitions map MUST be present.
    std::string word_name = "";
    std::string elem_name = "";
    ICDElement icd_element;
    YAML::Node word_data;
    YAML::Node elem_data;
    for (YAML::const_iterator it = transl_wrd_defs_node.begin(); it != transl_wrd_defs_node.end(); ++it)
    {
        if(!ValidateWordNode(it->second))
            return false;

        word_name = it->first.as<std::string>();
        word_data = (it->second)["wrd_data"];
        elem_data = (it->second)["elem"];

        // Iterate over elem data
        for(YAML::const_iterator it2 = elem_data.begin(); it2 != elem_data.end(); ++it2)
        {
            elem_name = it2->first.as<std::string>();
            if(!CreateICDElementFromWordNodes(word_name,
                        elem_name, word_data, (it2->second), icd_element))
                return false;

            word_elements[word_name].push_back(icd_element);
        }
    }

    return true;
}


bool DTS429::ValidateWordNode(const YAML::Node& word_node)
{
    if(!word_node.IsMap())
    {
        SPDLOG_WARN("DTS429::ValidateWordNode():"
                    " word_node is not a map");
        return false;
    }

    if(!word_node["wrd_data"])
    {
        SPDLOG_WARN("DTS429::ValidateWordNode(): "
                    "word_node is missing \"wrd_data\" key");
        return false;
    }

    if(!word_node["elem"])
    {
        SPDLOG_WARN("DTS429::ValidateWordNode(): "
                    "word_node is missing \"elem\" key");
        return false;
    }

    // wrd_data and elem are maps
    if(!word_node["elem"].IsMap())
    {
        SPDLOG_WARN("DTS429::ValidateWordNode(): "
                    "word_node \"elem\" key isn't a map");
        return false;
    }
    if(!word_node["wrd_data"].IsMap())
    {
        SPDLOG_WARN("DTS429::ValidateWordNode(): word_node "
                    "\"wrd_data\" key isn't a map");
        return false;
    }

    // wrd_data and elem maps aren't empty maps
    if(word_node["elem"].size() == 0)
    {
        SPDLOG_WARN("DTS429::ValidateWordNode(): "
                    "word_node \"elem\" key's map is empty");
        return false;
    }
    if(word_node["wrd_data"].size() == 0)
    {
        SPDLOG_WARN("DTS429::ValidateWordNode(): word_node "
                    "\"wrd_data\" map is empty");
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
    if(!elem_data.IsMap())
    {
        SPDLOG_WARN("DTS429::CreateICDElementFromWordNodes(): "
                    "elem_data isn't a map");
        return false;
    }

    if(elem_data.size() == 0)
    {
        SPDLOG_WARN("DTS429::CreateICDElementFromWordNodes(): "
                    "elem_data map is empty");
        return false;
    }

    if(!wrd_data.IsMap() || wrd_data.size()==0)
    {
        SPDLOG_WARN("DTS429::CreateICDElementFromWordNodes(): "
                    "wrd_data input format is incorrect");
        return false;
    }

    try
    {
        arinc_param.label_=wrd_data["label"].as<uint16_t>();
        arinc_param.sdi_=static_cast<int8_t>(wrd_data["sdi"].as<int16_t>());
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
        arinc_param.bitlsb_=static_cast<uint8_t>(elem_data["lsb"].as<uint16_t>());
        arinc_param.bit_count_=static_cast<uint8_t>(elem_data["bitcnt"].as<uint16_t>());
        arinc_param.uom_=elem_data["uom"].as<std::string>();
        arinc_param.classification_=static_cast<uint8_t>(elem_data["class"].as<uint16_t>());
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
