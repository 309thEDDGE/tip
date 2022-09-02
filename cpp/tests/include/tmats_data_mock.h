#ifndef TMATS_DATA_MOCK_H_
#define TMATS_DATA_MOCK_H_

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "tmats_data.h"

class MockTMATSData: public TMATSData
{
   public:
    MockTMATSData() : TMATSData() {}
    MOCK_CONST_METHOD3(FilterTMATSType, bool(const cmap& type_map, 
        Ch10PacketType type_enum, cmap& filtered_map));
    MOCK_CONST_METHOD2(FilterByChannelIDToType, cmap(const cmap& type_map, 
        const cmap& input_map));
    MOCK_CONST_METHOD0(GetChannelIDToTypeMap, const cmap&());
    MOCK_CONST_METHOD0(GetChannelIDToSourceMap, const cmap&());
    MOCK_CONST_METHOD0(GetChannelIDTo429Format, const cmap&());
    MOCK_CONST_METHOD0(GetChannelIDTo429Subchans, const cmapvec&());
    MOCK_CONST_METHOD0(GetChannelIDTo429SubchanAndName, const cmapmap&());
};

#endif  // TMATS_DATA_MOCK_H_ 