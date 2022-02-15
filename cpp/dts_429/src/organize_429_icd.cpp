#include "organize_429_icd.h"


bool Organize429ICD::OrganizeICDMap(std::unordered_map<std::string, std::vector<ICDElement>>& word_elements_map,
                        YAML::Node& md_chanid_to_subchan_node,
                        std::unordered_map<uint16_t,std::unordered_map<uint16_t, std::unordered_map<
                        uint16_t,std::unordered_map<int8_t, size_t>>>>& organized_lookup_map,
                        std::vector<std::vector<std::vector<ICDElement>>>& element_table)
{
    if(!ValidateInputs(word_elements_map, md_chanid_to_subchan_node))
        return false;

    if(!BuildBusNameToChannelAndSubchannelMap(md_chanid_to_subchan_node))
        return false;

    std::string bus_name;
    uint16_t chan_id;
    uint16_t subchan_id;
    uint16_t label;
    int8_t sdi;
    size_t table_index;

    std::unordered_map<std::string, std::vector<ICDElement>>::
            iterator it = word_elements_map.begin();

    // Iterate over the map using iterator
    while(it != word_elements_map.end())
    {
        if(!GetICDElementComponents(it->second,label, bus_name,sdi))
        {
            it++;
            continue;
        }
        if(!GetChannelSubchannelIDsFromMap(bus_name,chan_id,subchan_id,
                                        busname_to_channel_subchannel_ids_))
        {
            it++;
            continue;
        }

        if(IsIndexInLookupMap(chan_id,subchan_id,label,sdi,table_index,organized_lookup_map))
        {
            if(!AddToElementTable(table_index,it->second,element_table))
                return false;
        }
        else
        {
            table_index = element_table.size();
            if(!AddToElementTable(table_index,it->second,element_table))
                return false;

            organized_lookup_map[chan_id][subchan_id][label].insert({sdi,table_index});
        }
        it++;
    }

    return true;
}

bool Organize429ICD::ValidateInputs(std::unordered_map<std::string, std::vector<ICDElement>>& word_elements_map,
                        YAML::Node& md_chanid_to_subchan_node)
{
    if(word_elements_map.empty())
    {
        SPDLOG_WARN("Organize429ICD::ValidateInputs(): Argument word_elements_map is empty");
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
            // handle all 429 bus names as Uppercase to eliminate case sensitivity
            std::transform(subchan_name.begin(), subchan_name.end(),
                    subchan_name.begin(), ::toupper);
            if(!AddSubchannelToMap(channelid, subchan_number, subchan_name))
                return false;
        }
    }
    return true;
}

bool Organize429ICD::AddSubchannelToMap(uint16_t& channelid,
                                        uint16_t& subchan_number,
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

bool Organize429ICD::GetChannelSubchannelIDsFromMap(std::string& subchan_name,
                                                    uint16_t& channelid,
                                                    uint16_t& subchan_number,
                                                    std::unordered_map<std::string, std::tuple<uint16_t,
                                                    uint16_t>> bus_to_ids_map)
{
    if(bus_to_ids_map.size()<1)
    {
        SPDLOG_WARN("Organize429ICD::GetChannelSubchannelIDsFromMap(): Empty "
                "input map \'bus_to_ids_map\'");
        return false;
    }
    if(subchan_name.length()==0)
    {
        SPDLOG_WARN("Organize429ICD::GetChannelSubchannelIDsFromMap(): Empty "
                "input string \'subchan_name\'");
        return false;
    }
    if(bus_to_ids_map.find(subchan_name)==bus_to_ids_map.end())
    {
        subchannel_name_lookup_misses_.push_back(subchan_name);
        return false;
    }

    channelid = std::get<0>(bus_to_ids_map[subchan_name]);
    subchan_number = std::get<1>(bus_to_ids_map[subchan_name]);

    return true;
}

bool Organize429ICD::GetICDElementComponents(std::vector<ICDElement>& elements,
                                            uint16_t& label,
                                            std::string& bus_name,
                                            int8_t& sdi)
{
    if(elements.size()<1)
    {
        SPDLOG_WARN("Organize429ICD::GetICDElementComponents(): Empty input vector \'elements\'");
        return false;
    }
    if(elements[0].bus_name_=="")
    {
        SPDLOG_WARN("Organize429ICD::GetICDElementComponents(): Unnamed bus in \'elements\'");
        return false;
    }
    if(elements[0].label_==UINT16_MAX)
    {
        SPDLOG_WARN("Organize429ICD::GetICDElementComponents(): Invalid label in \'elements\'");
        return false;
    }
    if(elements[0].sdi_>3 || elements[0].sdi_<-1)
    {
        SPDLOG_WARN("Organize429ICD::GetICDElementComponents(): Invalid sdi in \'elements\'");
        return false;
    }

    label = elements[0].label_;
    sdi = elements[0].sdi_;
    bus_name = elements[0].bus_name_;

    return true;
}

bool Organize429ICD::IsIndexInLookupMap(uint16_t& chan_id, uint16_t& subchan_id,
                          uint16_t& label,int8_t& sdi, size_t& table_index,
                          std::unordered_map<uint16_t,std::unordered_map<uint16_t, std::unordered_map<
                                uint16_t,std::unordered_map<int8_t, size_t>>>>& organized_lookup_map)
{
    if(organized_lookup_map.size()<1)
    {
        SPDLOG_WARN("Organize429ICD::IsIndexInLookupMap(): "
            "Empty input map \'organized_lookup_map\'");
        return false;
    }
    if (organized_lookup_map.find(chan_id) == organized_lookup_map.end())
    {
        return false;
    }
    if (organized_lookup_map[chan_id].find(subchan_id) ==
            organized_lookup_map[chan_id].end())
    {
        return false;
    }
    if (organized_lookup_map[chan_id][subchan_id].find(label) ==
            organized_lookup_map[chan_id][subchan_id].end())
    {
        return false;
    }
    if (organized_lookup_map[chan_id][subchan_id][label].find(sdi) ==
            organized_lookup_map[chan_id][subchan_id][label].end())
    {
        return false;
    }

    table_index = organized_lookup_map[chan_id][subchan_id][label][sdi];

    return true;
}

bool Organize429ICD::AddToElementTable(size_t& table_index, std::vector<ICDElement>& word_elements,
                          std::vector<std::vector<std::vector<ICDElement>>>& element_table)
{
    if(word_elements.size()<1)
    {
        SPDLOG_WARN("Organize429ICD::AddToElementTable(): "
            "Empty input vector \'word_elements\'");
        return false;
    }

    if(table_index > element_table.size())
    {
        SPDLOG_WARN("Organize429ICD::AddToElementTable(): "
            "\'table_index\' is not valid.");
        return false;
    }

    if(table_index == element_table.size())
    {
        std::vector<std::vector<ICDElement>> temp_vec;
        temp_vec.push_back(word_elements);
        element_table.push_back(temp_vec);
    }
    else
    {
        element_table[table_index].push_back(word_elements);
    }

    return true;
}
