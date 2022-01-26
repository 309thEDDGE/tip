#include "organize_429_icd.h"


bool Organize429ICD::OrganizeICDMap(std::unordered_map<std::string, std::vector<ICDElement>>& word_elements,
                        YAML::Node& md_chanid_to_subchan_node,
                        std::unordered_map<uint16_t,std::unordered_map<uint16_t, std::unordered_map<
                        uint16_t,std::unordered_map<int8_t, std::vector<ICDElement>>>>>& organized_output_map)
{
    if(!ValidateInputs(word_elements, md_chanid_to_subchan_node))
        return false;

    if(!BuildBusNameToChannelAndSubchannelMap(md_chanid_to_subchan_node))
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
    if(!md_chanid_to_subchan_node["tmats_chanid_to_429_subchan_and_name"])
    {
        SPDLOG_WARN("Organize429ICD::ValidateInputs(): Argument transl_wrd_defs_node"
                    " doesn't contain \'tmats_chanid_to_429_subchan_and_name\'.");
        return false;
    }

    return true;
}

bool Organize429ICD::BuildBusNameToChannelAndSubchannelMap(YAML::Node& md_chanid_to_subchan_node)
{

    uint16_t channelid;
    uint16_t subchan_number;
    std::string subchan_name;

    YAML::Node channel_map = md_chanid_to_subchan_node["tmats_chanid_to_429_subchan_and_name"];

        for(YAML::const_iterator it = channel_map.begin();
                it != channel_map.end(); ++it)
    {
        channelid = it->first.as<uint16_t>();

        if(!it->second.IsMap()){
            SPDLOG_WARN("Organize429ICD::BuildBusNameToChannelAndSubchannelMap(): chanid doesn't map to a map!");
            return false;
        }

        YAML::Node subchan_map = it->second;
        for(YAML::const_iterator it2 = subchan_map.begin(); it2 != subchan_map.end(); ++it2)
        {
            subchan_number = it2->first.as<uint16_t>();
            subchan_name = it2->second.as<std::string>();
            if(!AddSubchannelToMap(channelid, subchan_number, subchan_name))
                return false;
        }
    }
    return true;
}

bool Organize429ICD::AddSubchannelToMap(uint16_t& channelid, uint16_t& subchan_number,
                            std::string& subchan_name)
{
    if(!busname_to_channel_subchannel_ids_.insert({subchan_name, std::make_tuple(channelid, subchan_number)}).second)
    {
        SPDLOG_WARN("Organize429ICD::AddSubchannelToMap(): Error adding the following bus name to map: ",
        subchan_name);
        return false;
    }
    return true;
}
