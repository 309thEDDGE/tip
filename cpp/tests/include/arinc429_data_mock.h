#ifndef ARINC429_DATA_MOCK_H_
#define ARINC429_DATA_MOCK_H_

#include <vector>
#include <string>
#include <unordered_map>

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "arinc429_data.h"
#include "yaml-cpp/yaml.h"

class MockARINC429Data : public ARINC429Data
{
   public:
    MockARINC429Data() : ARINC429Data() {}

    MockARINC429Data(std::unordered_map<uint16_t,std::unordered_map<uint16_t, std::unordered_map<
                uint16_t,std::unordered_map<int8_t, size_t>>>>& organized_lookup_map,
                std::vector<std::vector<std::vector<ICDElement>>>& element_table,
                std::unordered_map<size_t,std::vector<std::string>>& arinc_word_names,
                size_t& valid_arinc_word_count) : ARINC429Data(organized_lookup_map,
                element_table,arinc_word_names,valid_arinc_word_count) {}


    MOCK_METHOD5(IdentifyWord, bool(size_t& table_index, uint16_t& channelid, uint16_t& subchan_id,
                        uint16_t& label, int8_t& sdi));

   MOCK_METHOD2(GetWordElements, bool(size_t& table_index, std::vector<std::vector<ICDElement>>& arinc_elems));


   MOCK_METHOD3(GetArincWordNames, bool(size_t& table_index, size_t& vector_index, std::string& word_name));

   MOCK_METHOD0(GetValidArincWordCount, size_t());
};

#endif // ARINC429_DATA_MOCK_H_