#include "organize_429_icd.h"


bool Organize429ICD::OrganizeICDMap(std::unordered_map<std::string, std::vector<ICDElement>>& word_elements,
                        YAML::Node& md_chanid_to_subchan_node,
                        std::unordered_map<uint16_t,std::unordered_map<uint16_t, std::unordered_map<
                        uint16_t,std::unordered_map<int8_t, std::vector<ICDElement>>>>>& organized_output_map)
{
    if(word_elements.empty())
    {
        SPDLOG_WARN("Organize429ICD::OrganizeICDMap(): Argument word_elements is empty");
        return false;
    }
    return true;
}