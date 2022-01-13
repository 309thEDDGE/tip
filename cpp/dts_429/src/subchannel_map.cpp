#include "subchannel_map.h"

bool SubchannelMap::Ingest429ParserMDDoc(TIPMDDocument& parser_md_doc)
{
    // // build yaml nodes from parser_md_doc
    // std::unordered_map<uint64_t, std::unordered_map<uint16_t, string>>> tmats_chanid_to_429_subchan_and_name;
    // if(!YamlReader::GetMapNodeParameter(parser_md_doc.runtime_category_->node,
    //     "tmats_chanid_to_429_subchan_and_name", tmats_chanid_to_429_subchan_and_name))
    // {
    //     SPDLOG_ERROR(
    //         "ManageParseMetadata(): Failed to get"
    //         " tmats_chanid_to_429_subchan_and_name map from metadata!");
    //     return false;
    // }

    // // if no data, return false
    // if(tmats_chanid_to_429_subchan_and_name.empty())
    // {
    //     SPDLOG_ERROR(
    //         "ManageParseMetadata(): tmats_chanid_to_429_subchan_and_name"
    //         " map from metadata contains no subchannel mappings!");
    //     return false;
    // }
    return false;
}

bool SubchannelMap::MapSubchannelNameAndNumberToChannelID(
    unordered_map<uint64_t, unordered_map<uint16_t, string>>>& tmats_chanid_to_429_subchan_and_name)
{
    return false;
}

bool SubchannelMap::GetNameOfARINC429Bus(uint64_t channelid, uint16_t subchannel_number, string& bus_name)
{
    return false;
}