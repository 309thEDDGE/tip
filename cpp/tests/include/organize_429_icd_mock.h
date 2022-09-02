#ifndef ORGANIZE_429_ICD_MOCK_H_
#define ORGANIZE_429_ICD_MOCK_H_

#include <vector>
#include <string>
#include <unordered_map>

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "organize_429_icd.h"
#include "yaml-cpp/yaml.h"

class MockOrganize429ICD : public Organize429ICD
{
   public:
    MockOrganize429ICD() : Organize429ICD() {}

    MOCK_METHOD4(OrganizeICDMap, bool(std::unordered_map<std::string, std::vector<ICDElement>>& word_elements_map,
                        YAML::Node& md_chanid_to_subchan_node,
                        std::unordered_map<uint16_t,std::unordered_map<uint16_t, std::unordered_map<
                        uint16_t,std::unordered_map<int8_t, size_t>>>>& organized_lookup_map,
                        std::vector<std::vector<std::vector<ICDElement>>>& element_table));

   MOCK_METHOD0(GetSubchannelNameLookupMisses, std::vector<std::string>());
   MOCK_METHOD0(GetArincWordNames, std::unordered_map<size_t,std::vector<std::string>>());
   MOCK_METHOD0(GetValidArincWordCount, size_t());
};


#endif  //ORGANIZE_429_ICD_MOCK_H_