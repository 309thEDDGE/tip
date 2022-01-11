#ifndef SUBCHANNEL_MAP_H
#define SUBCHANNEL_MAP_H

#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include "yaml-cpp/yaml.h"
// #include "icd_data.h"
// #include "managed_path.h"


// The pupose of this class is to build nested maps. The class will map
// the channel id to a map which is used to map the subchannel number (integer)
// to the subchannel name (string). The map provides a way to search for an
// ARINC 429 bus name using the channel id and subchannel number.

// Note: the subchannel number is parsed out of an ARINC 429 word IPDH's
// bus field.

// A lookup of the ARINC 429 bus name (subchannel name), can be performed using
// subchannel_map[channelid][subchannel_number]


class SubchannelMap
{
   private:
    unordered_map<uint32_t, unordered_map<uint16_t, string>> channel_id_to_subchannel_map;

   public:
    SubchannelMap();

    // TODO - find out the input format that has most synergy with rest of project
    bool MapSubchannelNameAndNumberToChannelID(YAML::Node& tmats_chanid_to_429_subchan_and_name);

    string GetNameOfARINC429Bus(uint32_t channelid, uint16_t subchannel_number);

}


#endif
