#ifndef TMATS_DATA_H_
#define TMATS_DATA_H_

#include <map>
#include <vector>
#include <set>
#include <string>
#include "spdlog/spdlog.h"
#include "iterable_tools.h"
#include "tmats_parser.h"
#include "ch10_packet_type.h"

#define RETFAIL(x, y) if(!(x)) {SPDLOG_WARN("TMATSData::Parse: Failed to MapAttrs: " y); return false; }

using cmap = std::map<std::string, std::string>;
using cmapvec = std::map<std::string, std::vector<std::string>>;
using cmapmap = std::map<std::string, std::map<std::string, std::string>>;

class TMATSData
{
private:
	cmap chanid_to_source_map_;
    cmap chanid_to_type_map_;
	cmap chanid_to_429_format_;
    cmapvec	chanid_to_429_subchans_;
	cmapmap chanid_to_429_subchan_and_name_;

public:
	const cmap& chanid_to_source_map;
    const cmap& chanid_to_type_map;
	const cmap& chanid_to_429_format;
	const cmapvec& chanid_to_429_subchans;
	const cmapmap& chanid_to_429_subchan_and_name;

    static const std::map<Ch10PacketType, std::string> TMATS_channel_data_type_map_;
    TMATSData();
    virtual ~TMATSData() {}



    /*
    Parse TMATS matter passed in the form of a string. 

    Args:
        tmats_data      --> String representation of entire
                            TMATs blob. Includes formatting
                            chars such as newlines.
    	parsed_pkt_types--> Set of Ch10PacketType that contains only the
							present and parsed types

    Return:
        False if one of the attributes fails to map; true
        otherwise.
    */
   bool Parse(const std::string& tmats_data, const std::set<Ch10PacketType>& parsed_pkt_types);


    //////////////////////////////////////////////////////////////////////////
    //                  internal functions
    //////////////////////////////////////////////////////////////////////////

    /*
    Combine two cmapmaps under the assumption that the key of the submaps
    ought to be matched.
    */
    void CombineMaps(const cmapmap& map1, const cmapmap& map2, cmapmap& outmap) const;



    /*
    Filter channel ID to type map by the Ch10PacketType.

    Ex channel ID to type map:
    23: 1553IN
    24: 429IN
    27: 1553IN

    FilterTMATSType(map, Ch10PacketType::MILSTD1553_F1) yields:
    23: 1553IN
    27: 1553IN
    
    Args:
        type_map    --> Channel ID to type map 
                        (parser.MapAttrs("R-x\\TK1-n", "R-x\\CDT-n", chanid_to_type_map_))
        type_enum   --> Ch10PacketType
        filtered_map--> Filtered type map including only entries relevant to type_enum


    Return:
        True if no errors occur; false otherwise 
    */
    bool FilterTMATSType(const cmap& type_map, Ch10PacketType type_enum,
        cmap& filtered_map) const;



    /*
    Filter an input map in which the key is the channel ID by the channel 
    ID to type map. To be used with a filtered map generated by 
    FilterTMATSType.

    Ex input map:
    24: SOME_VALUE

    Ex channel ID to type map:
    23: 1553IN
    24: 429IN
    27: 1553IN

    FilterByChannelIDToType(type_map, input_map) yields:
    24: SOME_VALUE
    
    Args:
        type_map    --> Channel ID to type map 
                        (parser.MapAttrs("R-x\\TK1-n", "R-x\\CDT-n", chanid_to_type_map_))
        input_map   --> Input map in which the key is a channel ID.

    Return:
        Filtered input_map
    */
    cmap FilterByChannelIDToType(const cmap& type_map, const cmap& input_map) const;
};


#endif  // TMATS_DATA_H_
