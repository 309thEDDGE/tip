#include "organize_429_icd.h"


bool Organize429ICD::OrganizeICDMap(std::unordered_map<std::string, std::vector<ICDElement>>& word_elements,
                        YAML::Node& md_chanid_to_subchan_node,
                        std::unordered_map<uint16_t,std::unordered_map<uint16_t, std::unordered_map<
                        uint16_t,std::unordered_map<int8_t, std::vector<ICDElement>>>>>& organized_output_map)
{
    if(!ValidateInputs(word_elements, md_chanid_to_subchan_node))
        return false;

    return true;
}

bool Organize429ICD::ValidateInputs(std::unordered_map<std::string, std::vector<ICDElement>>& word_elements,
                        YAML::Node& md_chanid_to_subchan_node)
{
    if(word_elements.empty())
    {
        SPDLOG_WARN("Organize429ICD::ValidateInputs(): Argument word_elements is empty");
        return false;
    }
    if(md_chanid_to_subchan_node.IsNull())
    {
        SPDLOG_WARN("Organize429ICD::ValidateInputs(): Argument md_chanid_to_subchan_node"
                    " is null.");
        return false;
    }
   if(!md_chanid_to_subchan_node.IsMap())
    {
        SPDLOG_WARN("Organize429ICD::ValidateInputs(): Argument transl_wrd_defs_node"
                    " doesn't contain a map.");
        return false;
    }
    return true;
}