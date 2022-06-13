#ifndef PARSER_METADATA_MOCK_H_
#define PARSER_METADATA_MOCK_H_

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "parser_metadata.h"

class MockParserMetadataFunctions : public ParserMetadataFunctions
{
    public:
        MockParserMetadataFunctions() : ParserMetadataFunctions() {}

        MOCK_METHOD4(RecordProvenanceData, void(TIPMDDocument* md, 
            const ManagedPath& input_ch10_file_path, const std::string& packet_type_label, 
            const ProvenanceData& prov_data));

        MOCK_METHOD2(RecordUserConfigData, void(std::shared_ptr<MDCategoryMap> config_category, 
        	const ParserConfigParams& user_config));

        MOCK_METHOD3(ProcessTMATSForType, bool(const TMATSData* tmats_data, 
            TIPMDDocument* md, Ch10PacketType pkt_type));

        MOCK_METHOD2(WriteStringToFile, bool(const ManagedPath& outpath, 
            const std::string& outdata));

        MOCK_METHOD5(RecordCh10PktTypeSpecificMetadata, bool(Ch10PacketType pkt_type, 
            const std::vector<const Ch10Context*>& context_vec, MDCategoryMap* runtime_metadata, 
            const TMATSData* tmats, Ch10PacketTypeSpecificMetadata* spec_md));

        MOCK_METHOD4(RecordCh10PktTypeSpecificMetadata, bool(Ch10PacketType pkt_type, 
            const std::vector<const Ch10Context*>& context_vec, MDCategoryMap* runtime_metadata, 
            const TMATSData* tmats));

};

#endif  // PARSER_METADATA_MOCK_H_