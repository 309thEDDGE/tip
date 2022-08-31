#include "arinc429_data.h"

bool ARINC429Data::IdentifyWord(size_t& table_index, uint16_t& channelid, uint16_t& subchan_id,
                        uint16_t& label, int8_t& sdi)
{
    // validity checks.
    if(organized_lookup_map_.empty())
    {
        SPDLOG_WARN("ARINC429Data::IdentifyWord(): Empty lookup map");
        return false;
    }

    if(element_table_.empty())
    {
        SPDLOG_WARN("ARINC429Data::IdentifyWord(): Empty element table");
        return false;
    }

    // if channel id not in map
    if(organized_lookup_map_.find(channelid)
        ==organized_lookup_map_.end()) return false;


    // if subchannel_id not in map
    if(organized_lookup_map_[channelid].find(subchan_id+1)
        ==organized_lookup_map_[channelid].end()) return false;

    // if label not in map
    if(organized_lookup_map_[channelid][subchan_id+1].find(label)
        ==organized_lookup_map_[channelid][subchan_id+1].end()) return false;

    // if sdi not in map
    if(organized_lookup_map_[channelid][subchan_id+1][label].find(sdi)
        ==organized_lookup_map_[channelid][subchan_id+1][label].end())
    {
        // check for SDI of -1, indicating the DTS doesn't care what the SDI is
        if(organized_lookup_map_[channelid][subchan_id+1][label].find(-1)
            ==organized_lookup_map_[channelid][subchan_id+1][label].end())
        {
            return false;
        }
        else
        {
            table_index =organized_lookup_map_[channelid][subchan_id+1][label][-1];
            return true;
        }
    }

    table_index = organized_lookup_map_[channelid][subchan_id+1][label][sdi];

    return true;
}

bool ARINC429Data::GetWordElements(size_t& table_index, std::vector<std::vector<ICDElement>>& arinc_elems)
{
    // validity checks.
    if(organized_lookup_map_.empty())
    {
        SPDLOG_WARN("ARINC429Data::GetWordElements(): Empty lookup map");
        return false;
    }

    if(element_table_.empty())
    {
        SPDLOG_WARN("ARINC429Data::GetWordElements(): Empty element table");
        return false;
    }

    if(table_index >= element_table_.size())
    {
        return false;
    }

    arinc_elems = element_table_[table_index];

    return true;
}

bool ARINC429Data::GetArincWordNames(size_t& table_index, size_t& vector_index, std::string& word_name)
{
    if(arinc_word_names_.empty())
    {
        SPDLOG_WARN("ARINC429Data::GetArincWordNames(): Empty word name map");
        return false;
    }

    if(arinc_word_names_.find(table_index)
        ==arinc_word_names_.end()) return false;

    if(vector_index >= arinc_word_names_[table_index].size())
        return false;

    word_name = arinc_word_names_[table_index][vector_index];

    return true;
}

std::string ARINC429Data::LookupMapToString()
{
    std::string map_string = "ARINC 429 LOOKUP MAP\n";

    // iterate channel ids
    for (auto& it: organized_lookup_map_) {
        //append channel id
        map_string.append("channelid: ");
        map_string.append(std::to_string(it.first));

        // iterate subchannel id
        for (auto& it2: it.second) {
            //append subchannel id
            map_string.append("\n|---- subchannelid: ");
            map_string.append(std::to_string(it2.first));

            // iterate labels
            for (auto& it3: it2.second) {
                // append labels
                map_string.append("\n|----|---- label: ");
                map_string.append(std::to_string(it3.first));

                // iterate sdi
                for (auto& it4: it3.second) {

                    //append sdi
                    map_string.append("\n|----|----|---- sdi: ");
                    map_string.append(std::to_string(it4.first));

                    // append arinc word name
                    size_t index = organized_lookup_map_[it.first][ it2.first][ it3.first][ it4.first];

                    std::vector<std::string> element_name_vec = arinc_word_names_[index];
                    for(int i=0; i < element_name_vec.size(); i++){
                        map_string.append("\n|----|----|----|---- 429WordName: ");
                        map_string.append(element_name_vec[i]);
                        map_string.append("\n|----|----|----|----|---- ICDElement.elem_name_: ");

                        // get element and append element name + info
                        std::vector<ICDElement> elem_vec = element_table_[index][i];
                        for(int j = 0; j < elem_vec.size(); j++)
                        {
                            map_string.append(elem_vec[j].elem_name_);
                            map_string.append("\n|----|----|----|----|---- ICDElement.description_: ");
                            map_string.append(elem_vec[j].description_);
                            map_string.append("\n|----|----|----|----|---- ICDElement.bcd_partial_: ");
                            map_string.append(std::to_string(elem_vec[j].bcd_partial_));
                            map_string.append("\n|----|----|----|----|---- ICDElement.is_bitlevel_: ");
                            map_string.append(std::to_string(elem_vec[j].is_bitlevel_));
                            map_string.append("\n|----|----|----|----|---- ICDElement.msb_val_: ");
                            map_string.append(std::to_string(elem_vec[j].msb_val_));
                            map_string.append("\n|----|----|----|----|---- ICDElement.bitlsb_: ");
                            map_string.append(std::to_string(elem_vec[j].bitlsb_));
                            map_string.append("\n|----|----|----|----|---- ICDElement.bitmsb_: ");
                            map_string.append(std::to_string(elem_vec[j].bitmsb_));
                            map_string.append("\n|----|----|----|----|---- ICDElement.bit_count_: ");
                            map_string.append(std::to_string(elem_vec[j].bit_count_));
                            map_string.append("\n|----|----|----|----|---- ICDElement.uom_: ");
                            map_string.append(elem_vec[j].uom_);
                            map_string.append("\n|----|----|----|----|---- ICDElement.classification_: ");
                            map_string.append(std::to_string(elem_vec[j].classification_));
                            map_string.append("\n|----|----|----|----|---- ICDElement.offset_: ");
                            map_string.append(std::to_string(elem_vec[j].offset_));
                            map_string.append("\n|----|----|----|----|---- ICDElement.elem_word_count_: ");
                            map_string.append(std::to_string(elem_vec[j].elem_word_count_));

                        }
                    }


                }
            }

        }
        map_string.append("\n");
    }

    return map_string;
}


bool ARINC429Data::SSMSignForBCD(uint8_t ssm, int8_t& sign)
{
    if (ssm == 1) // no computed data
    {
        sign = 0;
        return false; 
    }
    else if (ssm == 2) // functional test
    {
        sign = 0;
        return false;
    }
    else if(ssm == 0)
        sign = 1;
    else
        sign = -1;
    
    return true;
}