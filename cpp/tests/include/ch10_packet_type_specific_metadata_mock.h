#ifndef CH10_PACKET_TYPE_SPECIFIC_METADATA_MOCK_H_
#define CH10_PACKET_TYPE_SPECIFIC_METADATA_MOCK_H_

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ch10_packet_type_specific_metadata.h"

class MockCh10PacketTypeSpecificMetadataFunctions : public Ch10PacketTypeSpecificMetadataFunctions
{
    public:
        MockCh10PacketTypeSpecificMetadataFunctions() : Ch10PacketTypeSpecificMetadataFunctions() {}
        MOCK_CONST_METHOD3(CombineChannelIDToLRUAddressesMetadata, bool(
            std::map<uint32_t, std::set<uint16_t>>& output_chanid_lruaddr_map,
            const std::vector<std::map<uint32_t, std::set<uint16_t>>>& chanid_lruaddr1_maps,
            const std::vector<std::map<uint32_t, std::set<uint16_t>>>& chanid_lruaddr2_maps));
        MOCK_CONST_METHOD2(CombineChannelIDToCommandWordsMetadata, bool(
            std::map<uint32_t, std::vector<std::vector<uint32_t>>>& output_chanid_commwords_map,
            const std::vector<std::map<uint32_t, std::set<uint32_t>>>& chanid_commwords_maps));
        MOCK_CONST_METHOD2(CreateChannelIDToMinVideoTimestampsMetadata, void(
            std::map<uint16_t, uint64_t>& output_chanid_to_mintimestamp_map,
            const std::vector<std::map<uint16_t, uint64_t>>& chanid_mintimestamp_maps));
        MOCK_CONST_METHOD2(CombineChannelIDToLabelsMetadata, bool(
            std::map<uint32_t, std::set<uint16_t>>& output_chanid_labels_map,
            const std::vector<std::map<uint32_t, std::set<uint16_t>>>& chanid_labels_maps));
        MOCK_CONST_METHOD2(CombineChannelIDToBusNumbersMetadata, bool(
            std::map<uint32_t, std::set<uint16_t>>&  output_chanid_busnumbers_map,
            const std::vector<std::map<uint32_t, std::set<uint16_t>>>& chanid_busnumbers_maps));
};

class MockCh10PacketTypeSpecificMetadata : public Ch10PacketTypeSpecificMetadata
{
    public:
        MockCh10PacketTypeSpecificMetadata() : Ch10PacketTypeSpecificMetadata() {}

        MOCK_METHOD2(RecordMilStd1553F1SpecificMetadata, bool(
            std::vector<const Ch10Context*> context_vec, MDCategoryMap* runtime_metadata));

        MOCK_METHOD2(RecordVideoDataF0SpecificMetadata, bool(
            std::vector<const Ch10Context*> context_vec, MDCategoryMap* runtime_metadata));

        MOCK_METHOD3(RecordARINC429F0SpecificMetadata, bool(
            std::vector<const Ch10Context*> context_vec, MDCategoryMap* runtime_metadata, 
            const TMATSData* tmats));
};


#endif  // CH10_PACKET_TYPE_SPECIFIC_METADATA_MOCK_H_