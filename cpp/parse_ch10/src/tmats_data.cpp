#include "tmats_data.h"

TMATSData::TMATSData() : chanid_to_source_map(chanid_to_source_map_), 
    chanid_to_type_map(chanid_to_type_map_),
    chanid_to_429_format(chanid_to_429_format_),
    chanid_to_429_subchans(chanid_to_429_subchans_),
    chanid_to_429_subchan_and_name(chanid_to_429_subchan_and_name_)
{}


bool TMATSData::Parse(const std::string& tmats_data)
{
    TMATSParser parser(tmats_data);

    // General 
    RETFAIL(parser.MapAttrs("R-x\\TK1-n", "R-x\\DSI-n", chanid_to_source_map_), "chanid_to_source");
    RETFAIL(parser.MapAttrs("R-x\\TK1-n", "R-x\\CDT-n", chanid_to_type_map_), "chanid_to_type");

    // ARINC 429
    RETFAIL(parser.MapAttrs("R-x\\TK1-n", "R-x\\ABTF-n", chanid_to_429_format_), "chanid_to_429_format");
    RETFAIL(parser.MapAttrs("R-x\\TK1-n", "R-x\\ASN-n-m", chanid_to_429_subchans_), "chanid_to_429_subchans");

    cmapmap chanid_to_429index_and_subchan;
    cmapmap chanid_to_429index_and_name;
    RETFAIL(parser.MapAttrs("R-x\\TK1-n", "R-x\\ASN-n-m", chanid_to_429index_and_subchan), "chanid_to_429index_and_subchan");
    RETFAIL(parser.MapAttrs("R-x\\TK1-n", "R-x\\ANM-n-m", chanid_to_429index_and_name), "chanid_to_429index_and_name");
 
    CombineMaps(chanid_to_429index_and_subchan, chanid_to_429index_and_name,
        chanid_to_429_subchan_and_name_);

    return true;
}


void TMATSData::CombineMaps(const cmapmap& map1, const cmapmap& map2, 
    cmapmap& outmap)
{
    for(cmapmap::const_iterator it1 = map1.cbegin(); it1 != map1.cend(); ++it1)
    {
        if(map2.find(it1->first) != map2.cend())
        {
            cmap temp_map;
            cmap map1sub = it1->second;
            cmap map2sub = map2.at(it1->first);
            for(cmap::const_iterator it2 = map1sub.cbegin(); it2 != map1sub.cend(); ++it2) 
            {
                if(map2sub.find(it2->first) != map2sub.cend())
                {
                    temp_map[it2->second] = map2sub.at(it2->first);
                }
            }

            outmap[it1->first] = temp_map;
        }
    }
}