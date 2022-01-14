#ifndef SUBCHANNEL_MAP_H
#define SUBCHANNEL_MAP_H

#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include "yaml-cpp/yaml.h"
#include "tip_md_document.h"
#include "spdlog/spdlog.h"
#include "yaml_reader.h"


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
    unordered_map<uint64_t, unordered_map<uint16_t, string>> channel_id_to_subchannel_map_;

   public:
    SubchannelMap();

    unordered_map<uint64_t, unordered_map<uint16_t, string>> GetChanIDToSubchanMap() { return channel_id_to_subchannel_map_;}

    /*
      Ingest429ParserMDDoc

		  parser_md_doc:	--> TIPMDDocument which read in string representation of
                          the ARINC 429 metadata file, containing new line
                          character at the end of each line.

      return:	        --> True if success, otherwise false.

	  */
    bool Ingest429ParserMDDoc(TIPMDDocument& parser_md_doc);

    /*
      MapSubchannelNameAndNumberToChannelID

      tmats_chanid_to_429_subchan_and_name:   -->	Node built from metadata genereated from TIPs
                                                      parsing of ARINC 429 channels. Node contains a
                                                      map of channelid to subchannel numbers and names,
                                                      in the following format:

                                                              31: {1: ARBusName1}

              return:							    --> True if success, otherwise false.

	  */
    bool MapSubchannelNameAndNumberToChannelID(unordered_map<uint64_t, unordered_map<uint16_t, string>>>& tmats_chanid_to_429_subchan_and_name);

    /*
      GetNameOfARINC429Bus

      channelid:          --> Integer representing the channelid associated with the ARINC 429
                                  bus.

          subchannel_number:  --> The subchannel id (number) assocaited with the name of the ARINC 429
                                  bus. This subchannel id association is made in TMATS and is stored
                                  in the ARINC 429 parsed data parquet.

          bus_name:           --> If found, bus name string will be stored in bus_name.

      return:				--> True if success, otherwise false.

    */
    bool GetNameOfARINC429Bus(uint64_t channelid, uint16_t subchannel_number, string& bus_name);

}


#endif
