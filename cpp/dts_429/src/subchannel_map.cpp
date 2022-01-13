#include "subchannel_map.h"

SubchannelMap::Ingest429ParserMDDoc(TIPMDDocument& parser_md_doc)
{
    return false;
}

SubchannelMap::MapSubchannelNameAndNumberToChannelID(
    unordered_map<uint64_t, unordered_map<uint16_t, string>>>& tmats_chanid_to_429_subchan_and_name)
{
    return false;
}

SubchannelMap::GetNameOfARINC429Bus(uint32_t channelid, uint16_t subchannel_number, string& bus_name)
{
    return false;
}