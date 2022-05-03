#ifndef ORGANIZE_429_ICD_H
#define ORGANIZE_429_ICD_H

#include <string>
#include <unordered_map>
#include <vector>
#include <tuple>
#include "icd_element.h"
#include "yaml-cpp/yaml.h"
#include "spdlog/spdlog.h"

class Organize429ICD
{
   private:
   // string is busname (subchannel name from TMATS), tuple will
   // be in the order (channelid, subchannelid)
    std::unordered_map<std::string, std::tuple<uint16_t, uint16_t>> busname_to_channel_subchannel_ids_;

    // storage for subchannel names not found in
    // busname_to_channel_subchannel_ids_ lookup.
    std::vector<std::string> subchannel_name_lookup_misses_;

    // map providing the arinc word name at a given table_index and vector index matching the
    // arinc word data vector, vector<vector<ICDElement>>, found in element_table created by
    // Organize429ICD
    std::unordered_map<size_t,std::vector<std::string>> arinc_word_names_;

    // count of valid 429 words in ICD added to lookup map
    size_t valid_arinc_word_count_;

   public:
    Organize429ICD(){}
    virtual ~Organize429ICD(){}

    virtual std::unordered_map<std::string, std::tuple<uint16_t, uint16_t>> GetBusNameToChannelSubchannelMap()
    {
        return busname_to_channel_subchannel_ids_;
    }

    virtual std::vector<std::string> GetSubchannelNameLookupMisses()
    {
        return subchannel_name_lookup_misses_;
    }

    virtual std::unordered_map<size_t,std::vector<std::string>> GetArincWordNames()
    {
        return arinc_word_names_;
    }

    virtual size_t GetValidArincWordCount() {return valid_arinc_word_count_;}

    /*
    Perform organization of nested maps to vector<ICDElement>. Resulting map
    structure will be used to find word information when translating ARINC 429
    parsed data.

    Args:
        word_elements:              --> unordered_map of ARINC 429 word name to vector of
                                        all elements associated with it (as ICDElment).
                                        Produced by DTS429()

        md_chanid_to_subchan_node   --> YAML::Node that is found in the ARINC429 parsing
                                        metadata otuput under tmats_chanid_to_429_subchan_and_name.

        organized_lookup_map        --> unordered_map - nested maps to size_t which provides
                                        the index of a vector of ICDElement vectors in
                                        element_table. These vectors of ICDElements have
                                        all of the ICDElements associated with words which
                                        have common features: chanid, subchan_id, label, sdi.
                                        The output map will be structured such that the
                                        size_t index can be reached using the following:
                                        organized_output_map[chanid][subchan_id][label][sdi]

        element_table               --> At the index provided by ourganized_lookup_map a
                                        vector<vector<ICDElement>> is returned bearing all
                                        the ICDElements of the 429 words associated with
                                        a given combination of chanid, subchan_id, label, sdi.

    Return:
        True if map successfully constructed; false otherwise
    */
    virtual bool OrganizeICDMap(std::unordered_map<std::string, std::vector<ICDElement>>& word_elements_map,
                        YAML::Node& md_chanid_to_subchan_node,
                        std::unordered_map<uint16_t,std::unordered_map<uint16_t, std::unordered_map<
                        uint16_t,std::unordered_map<int8_t, size_t>>>>& organized_lookup_map,
                        std::vector<std::vector<std::vector<ICDElement>>>& element_table);

    /*
    Perform validation of inputs to OrganizeICDMap(). Check that inputs are properly formatted.

    Args:
        word_elements:              --> unordered_map of ARINC 429 word name to vector of
                                        all elements associated with it (as ICDElment).
                                        Produced by DTS429()

        md_chanid_to_subchan_node   --> YAML::Node that is found in the ARINC429 parsing
                                        metadata otuput under tmats_chanid_to_429_subchan_and_name.

    Return:
        True if inputs pass; false otherwise
    */
    bool ValidateInputs(std::unordered_map<std::string, std::vector<ICDElement>>& word_elements_map,
                        YAML::Node& md_chanid_to_subchan_node);

    /*
    Iterate and reorganize tmats_chanid_to_429_subchan_and_name from parsed
    ARINC 429metadata to build busname_to_channel_subchannel_ids_ map.

    Args:
        md_chanid_to_subchan_node   --> YAML::Node that is found in the ARINC429 parsing
                                        metadata otuput where md_chanid_to_subchan_node.first
                                        == tmats_chanid_to_429_subchan_and_name.

    Return:
        True if map successfully constructed; false otherwise
    */
    bool BuildBusNameToChannelAndSubchannelMap(YAML::Node& md_chanid_to_subchan_node);

    /*
    Called from within BuildBusNameToChannelAndSubchannelMap() to handle a the mapping
    to a single subchannel associated with a channelid, as found in the
    YAML::Node md_chanid_to_subchan_node.

    Args:
        channelid       --> YAML::Node representing single chanid with mapped subchannel
                            info from tmats_chanid_to_429_subchan_and_name in metadata.

        subchan_number  --> uint16_t value which is the number used to map an ARINC
                            429 subchannel ('bus' in ARINC 429 spec) to a channelid in
                            the chapter 10 recorder.

        subchan_name    --> string name of the subchannel, which is the name of an
                            ARINC429 bus. This name should match the 'bus' discription
                            as seen in the 429 DTS schema

    Return:
        True if tuples successfully constructed and added to map; false otherwise
    */
    bool AddSubchannelToMap(uint16_t& channelid, uint16_t& subchan_number,
                            std::string& subchan_name);


    /*
    Input a string subchannel name from an ICDElment and find the associated channelid
    and subchannel_number from busname_to_channel_subchannel_ids_. If the subchan_name
    is not found in map, the name will be stored in subchannel_name_lookup_misses_

    Args:
        subchan_name    --> string name of the subchannel, which is the name of an
                            ARINC429 bus. This name should match the 'bus' discription
                            as seen in the 429 DTS schema

        channelid       --> YAML::Node representing single chanid with mapped subchannel
                            info from tmats_chanid_to_429_subchan_and_name in metadata.

        subchan_number  --> uint16_t value which is the number used to map an ARINC
                            429 subchannel ('bus' in ARINC 429 spec) to a channelid in
                            the chapter 10 recorder.

        bus_to_ids_mapm --> Expected input is the private variable,
                            busname_to_channel_subchannel_ids_

    Return:
        True if subchan_name found; otherwise false and subchan_name added
        to subchannel_name_lookup_misses_
    */
    bool GetChannelSubchannelIDsFromMap(std::string& subchan_name,
                    uint16_t& channelid, uint16_t& subchan_number,
                    std::unordered_map<std::string, std::tuple<uint16_t,
                    uint16_t>> bus_to_ids_map);

    /*
    Input a vector of ICDElements from word_elements map. Use the first ICDElement
    in the vector to obtain label, bus_name, and sdi.

    Args:
        elements    --> Vector of ICD elements all of which were mapped to a
                        single ARINC 429 word name (in word_elements map)

        label       --> std::string for storing the ARINC 429 word's label

        bus_name    --> string name of the subchannel, which is the name of an
                        ARINC429 bus. This name should match the 'bus' discription
                        as seen in the 429 DTS schema

        sdi         --> ARINC 429 word's sdi value, as defined in 429 dts schema

    Return:
        True label, bus_name, and sdi are found and assigned; false otherwise.
    */
    bool GetICDElementComponents(std::vector<ICDElement>& elements,
                                 uint16_t& label, std::string& bus_name,
                                 int8_t& sdi);

    /*
    Search organized_lookup_map to see if there is a table_index stored
    using the lookups chan_id, subchan_id, label, and sdi. If found in
    map, assigns table_index and returns true. Else, return false.

    Args:
        chan_id     --> uint16_t id of channel in ch10 recording.

        subchan_id  --> uint16_t ARINC 429 bus subchannel id.

        label       --> std::string for storing the ARINC 429 word's label

        sdi         --> int8_t ARINC 429 word's sdi value, as defined in
                        429 dts schema

        table_index --> size_t value providing the index where a vector
                        of vector<ICDElement> is stored in element_table,
                        found in OrganizeICDMap().

        organized_lookup_map    --> map storing a table_index using the
                                    keys: chan_id, subchan_id, label, and
                                    sdi.

    Return:
        True if index is found in map and assigned to table_index; false otherwise.
    */
    bool IsIndexInLookupMap(uint16_t& chan_id, uint16_t& subchan_id,
                          uint16_t& label,int8_t& sdi, size_t& table_index,
                          std::unordered_map<uint16_t,std::unordered_map<uint16_t, std::unordered_map<
                                uint16_t,std::unordered_map<int8_t, size_t>>>>& organized_lookup_map);


    /*
    Determins if a vector already exists at table_index, or if a new
    new vector needs to be created and added to the back of element_table.
    Inserts vector<ICDElement> to existing or new vector at position
    table_index.

    Args:
        table_index     --> size_t value providing the index where a vector
                            of vector<ICDElement> is stored in element_table,
                            found in OrganizeICDMap().

        word_elements   --> A vector of ICDElement that are all associated with
                            a single ARINC 429 word defintion.

        element_table   --> At the index provided by ourganized_lookup_map a
                            vector<vector<ICDElement>> is returned bearing all
                            the ICDElements of the 429 words associated with
                            a given combination of chanid, subchan_id, label, sdi.

    Return:
        True if vector<ICDElement> successfully added to element_table; false otherwise.
    */
    bool AddToElementTable(size_t& table_index, std::vector<ICDElement>& word_elements,
                          std::vector<std::vector<std::vector<ICDElement>>>& element_table);
};

#endif