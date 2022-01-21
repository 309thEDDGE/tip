#ifndef ORGANIZE_ICD_H
#define ORGANIZE_ICD_H

#include <string>
#include <unordered_map>
#include "icd_element.h"
#include "yaml-cpp/yaml.h"
#include "spdlog/spdlog.h"

class Organize429ICD
{
   private:
   // string is busname (subchannel name from TMATS), tuple will
   // be in the order (channelid, subchannelid)
    std::unordered_map<std::string, std::tuple<uint16_t, uint16_t>> busname_to_channel_subchannel_ids_;

   public:
    OrganizeICD() {};

    /*
    Perform organization of nested maps to vector<ICDElement>. Resulting map
    structure will be used to find word information when translating ARINC 429
    parsed data.

    Args:
        word_elements:              --> unordered_map of ARINC 429 word name to vector of
                                        all elements associated with it (as ICDElment).
                                        Produced by DTS429()

        md_chanid_to_subchan_node   --> YAML::Node that is expected to the value which
                                        is mapped to the word name/label

        organized_output_map        --> unordered_map - nested maps to the ICDElement vector.
                                        The output map will be structured such that the
                                        ICDElement vector can be reached using the following:
                                        organized_output_map[chanid][subchan_id][label][sdi]

    Return:
        True if map successfully constructed; false otherwise
    */
    bool OrganizeICDMap(std::unordered_map<std::string, std::vector<ICDElement>>& word_elements,
                        YAML::Node& md_chanid_to_subchan_node,
                        std::unordered_map<uint16_t,std::unordered_map<uint16_t, std::unordered_map<
                        uint16_t,std::unordered_map<int8_t, std::vector<ICDElement>>>>>& organized_output_map);




    /*
    Iterate and reorganize tmats_chanid_to_429_subchan_and_name from parsed
    ARINC 429metadata to build busname_to_channel_subchannel_ids_ map.

    Args:
        md_chanid_to_subchan_node   --> YAML::Node that is expected to the value which
                                        is mapped to the word name/label

    Return:
        True if map successfully constructed; false otherwise
    */
    bool BuildBusToSubchannelInfo(YAML::Node& md_chanid_to_subchan_node);


};





#endif

