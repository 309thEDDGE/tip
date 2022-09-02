#ifndef ARINC429DATA_H
#define ARINC429DATA_H

#include <sstream>
#include <vector>
#include <map>
#include <unordered_map>
#include <string>
#include "icd_data.h"
#include "icd_element.h"
#include "spdlog/spdlog.h"

// ARINC429Data
//
// This class manages access to the paremeter element table/map that
// is produced by OrganizeICDMap.

class ARINC429Data
{
   private:
    std::unordered_map<uint16_t,std::unordered_map<uint16_t, std::unordered_map<
                        uint16_t,std::unordered_map<int8_t, size_t>>>> organized_lookup_map_;
    std::vector<std::vector<std::vector<ICDElement>>> element_table_;

    // map providing the arinc word name at a given table_index and index equal to the
    // arinc word data vector, vector<vector<ICDElement>>, found in element_table_
    std::unordered_map<size_t,std::vector<std::string>> arinc_word_names_;

    // count of valid 429 words in ICD
    size_t valid_arinc_word_count_;
    std::unordered_map<std::string, size_t> arinc_word_name_to_unique_index_map_;


   public:
    ARINC429Data() {}
    virtual ~ARINC429Data() {}

    ARINC429Data(std::unordered_map<uint16_t,std::unordered_map<uint16_t, std::unordered_map<
                  uint16_t,std::unordered_map<int8_t, size_t>>>>& organized_lookup_map,
                  std::vector<std::vector<std::vector<ICDElement>>>& element_table)
    {
        organized_lookup_map_ = organized_lookup_map;
        element_table_ = element_table;
    }

    ARINC429Data(std::unordered_map<uint16_t,std::unordered_map<uint16_t, std::unordered_map<
                uint16_t,std::unordered_map<int8_t, size_t>>>>& organized_lookup_map,
                std::vector<std::vector<std::vector<ICDElement>>>& element_table,
                std::unordered_map<size_t,std::vector<std::string>>& arinc_word_names)
    {
        organized_lookup_map_ = organized_lookup_map;
        element_table_ = element_table;
        arinc_word_names_ = arinc_word_names;
        SetNamesToUniqueIndexMap(arinc_word_names_,arinc_word_name_to_unique_index_map_);
    }

    ARINC429Data(std::unordered_map<uint16_t,std::unordered_map<uint16_t, std::unordered_map<
                uint16_t,std::unordered_map<int8_t, size_t>>>>& organized_lookup_map,
                std::vector<std::vector<std::vector<ICDElement>>>& element_table,
                std::unordered_map<size_t,std::vector<std::string>>& arinc_word_names,
                size_t& valid_arinc_word_count)
    {
        organized_lookup_map_ = organized_lookup_map;
        element_table_ = element_table;
        arinc_word_names_ = arinc_word_names;
        valid_arinc_word_count_ = valid_arinc_word_count;
        SetNamesToUniqueIndexMap(arinc_word_names_,arinc_word_name_to_unique_index_map_);
    }

    size_t GetTableSize() { return element_table_.size(); }
    std::unordered_map<size_t,std::vector<std::string>> GetArincWordNamesMap()
    {
        return arinc_word_names_;
    }

    // build  arinc_word_name_to_unique_index_map_ from arinc_word_names.
    // returns true if map built; false otherwise.
    bool SetNamesToUniqueIndexMap(
        const std::unordered_map<size_t, std::vector<std::string>>& word_names,
        std::unordered_map<std::string, size_t>& word_name_to_unique_index_map)
    {
        if(word_names.size() == 0)
            return false;

        size_t unique_index = 0;
        for(std::unordered_map<size_t, std::vector<std::string>>::const_iterator it =
            word_names.cbegin(); it != word_names.cend(); ++it)
        {
            std::vector<std::string> names_vec = it->second;
            for(size_t i = 0; i < names_vec.size(); i++)
            {
                word_name_to_unique_index_map[names_vec.at(i)] = unique_index;
                unique_index++;
            }
        }
        return true;
    }

    std::unordered_map<std::string, size_t> GetNamesToUniqueIndexMap()
    {
        return arinc_word_name_to_unique_index_map_;
    }

    virtual size_t GetValidArincWordCount() {return valid_arinc_word_count_;}

   /*
    Identify word. Provides the index to a vector of ICDElements vectors.
    Locates index from organized_lookup_map_ with the inputs channelid,
    subchannel_id, label, and sdi. If found, stores in table_index.

    Args:
        table_index --> size_t index where 429 word related information
                        is located in element_table_

        chan_id     --> uint16_t id of channel in ch10 recording.

        subchan_id  --> uint16_t ARINC 429 bus subchannel id.

        label       --> uint16_t for storing the ARINC 429 word's label

        sdi         --> int8_t ARINC 429 word's sdi value, as defined in 429 dts schema

    Return:
        true if table_index located in map with inputs, false otherwise
    */
    virtual bool IdentifyWord(size_t& table_index, uint16_t& channelid, uint16_t& subchan_id,
                        uint16_t& label, int8_t& sdi);

   /*
    Find vector vector<ICDElement> which are associated with an ARINC429
    word, and are stored in element_table_ at given table index

    Args:
        table_index --> size_t index where 429 word related information
                        is located in element_table_

        arinc_elems --> ICDElement information related to a set of ARINC 429
                        identifiers (channelid, subchannel_id, label, sdi).
                        This vector of vectors is found at index, table_index,
                        in element_table_.

    Return:
        true if index found in elem_vec, false otherwise
    */
    virtual bool GetWordElements(size_t& table_index, std::vector<std::vector<ICDElement>>& arinc_elems);

    /*
    Identify the name of and ARINC 429 word, as given in DTS429, which is stored in the
    arinc_word_name_ map at a given key (table_index) and position in the vector (vector_index)


    Args:
        table_index     --> size_t index where 429 word related information
                            is located in element_table_

        vector_index    --> size_t index where 429 word related information
                            is located in element_table_

        word_name       --> std:string variable to store a arinc word name,
                            if located in a vector in the arinc_word_names map

    Return:
        true if arinc word name found in elem_vec, false otherwise
    */
    virtual bool GetArincWordNames(size_t& table_index, size_t& vector_index, std::string& word_name);

    /*
    Identify the name of and ARINC 429 word, as given in DTS429, which is stored in the
    arinc_word_name_ map at a given key (table_index) and position in the vector (vector_index)

    Return:
        String which when output depicts the contents of organized_lookup_map_ and
        element_table_
    */
    std::string LookupMapToString();


    /*
    Convert SSM to sign for the case of BCD

    Args:
        ssm     --> Integer value of SSM
        sign    --> -1 or +1 depending on interpretation
                    of SSM

    Return:
        True if -1 or +1 can be returned based on SSM value,
        false if some other condition occurs such as "No computed
        data" or "functional test".
    */
    static bool SSMSignForBCD(uint8_t ssm, int8_t& sign);
};

#endif