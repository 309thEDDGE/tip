#ifndef BUSMAP_H
#define BUSMAP_H

#include <string>
#include <utility>
#include <map>
#include <unordered_map>
#include <set>
#include <fstream>
#include <vector>
#include <sstream>
#include <iostream>
#include "spdlog/spdlog.h"
#include "parse_text.h"
#include "iterable_tools.h"
#include "user_input.h"

class BusMap
{
   private:
    std::unordered_map<uint64_t, std::set<std::string>>
        icd_message_key_to_busnames_map_;

    std::unordered_map<uint64_t, std::set<uint64_t>>
        icd_message_key_to_channelids_map_;

    std::map<uint64_t, std::string> tmats_chanid_to_source_map_;
    std::set<std::string> unique_buses_;
    std::set<uint64_t> channel_ids_;
    int64_t key_;
    uint64_t mask_;
    std::map<uint64_t, std::pair<std::string, std::string>>
        final_bus_map_with_sources_;

    std::map<std::string, std::string> tmats_busname_corrections_;
    IterableTools iterable_tools_;
    bool tmats_present_;
    UserInput user_input_;
    std::map<uint64_t, std::string>* final_map_ptr_;
    uint64_t vote_threshold_;
    bool vote_method_checks_tmats_;
    std::map<uint64_t, std::string> excluded_channel_ids_;
    std::set<std::string> upper_case_bus_exclusions_;

    void PrepareFinalMap();
    void PrintVoteMap();
    std::map<uint64_t, std::string> VoteMapping();
    std::map<uint64_t, std::string> TmatsMapping();
    std::string PrintFinalMap();
    void SubmitToFinalBusMap(const std::map<uint64_t,
                                            std::string>& insert_map,
                             std::string source);
    std::unordered_map<uint64_t, std::unordered_map<std::string, uint64_t>> votes_;

   public:
    BusMap() : tmats_present_(false), mask_(UINT64_MAX), vote_threshold_(1) {}
    ~BusMap() {}

    /*
	 Initialize bus map with required maps

	 icd_message_keys_to_busnames -> retrieved from the ICD. Construct a key and a set
										of bus names that correlate to the key.
										In the case of 1553, the key is the transmit
										command word bit shifted left 16 bits
										logical ORd with the recieve command word.

	 channel_ids				  -> set of channel_ids that need to be mapped

	 mask						  -> this will mask the message key allowing the user
										to mask out different portions of the key.
										Example: the user may want to mask out the word
										count portion of the key that correlates to the 
										transmit and recieve word count.
										If the key consists of:

										<32 bits> (not used)
										<16 transmit command word bits>
										<16 recieve command word bits>

										the user would pass the following mask.

										0b11111111111111111111111111111111
										  11111111111000001111111111100000

										Note the last five bits of the transmit
										and recieve command words are zero (word count portions).
										This mask will ensure that keys with consistent
										bits in every bit postition besides the word count
										portions will be considered as the same key.

	 vote_threshold				  -> for a mapping to be made, votes must be >= vote_threshold

	 vote_method_checks_tmats	  -> vote_method_checks_tmats is only used when use_tmats_busmap = false
										Once the voting process is carried out, the results will be compared 
										with TMATS. If the bus name determined by the vote method is a substring
										of the bus name found in TMATS OR the TMATS bus name is a substring
										of the vote method bus name for a given channel ID, it will allow the channel 
										ID to be mapped. If a TMATS substring match is not made, it will not map 
										the channel ID that doesn't have a TMATS bus name match

	 bus_exclusions				  -> if a bus name exists in the final bus map that also
										exists in the bus_exclusions list, remove it from 
										the final map. The bus_exclusion list marks a removal 
										if it is a substring of a final mapped bus. It is also 
										not case sensative when applying matches. 
										If the excluded bus name exists in tmats_chanid_to_source_map,
										the channel ID associated with the excluded bus will
										also be removed even if the vote method suggests
										a different bus mapping for that given channel ID.

	 tmats_chanid_to_source_map	  -> non required map of channel ids to source(bus name)
										from tmats. The information is in metadata
										inside the parsed ch10 parquet file.

	 tmats_busname_corrections    -> non required map of tmats bus name corrections.
										It will correct the tmats source name (busname)
										using the key as the tmats source name and the
										value as the desired correction.
	*/
    void InitializeMaps(
        const std::unordered_map<uint64_t, std::set<std::string>>*
            icd_message_keys_to_busnames,
        std::set<uint64_t> channel_ids,
        uint64_t mask = UINT64_MAX,
        uint64_t vote_threshold = 1,
        bool vote_method_checks_tmats = false,
        std::set<std::string> bus_exclusions = std::set<std::string>(),
        std::map<uint64_t, std::string> tmats_chanid_to_source_map = std::map<uint64_t, std::string>(),
        std::map<std::string, std::string> tmats_busname_corrections = std::map<std::string, std::string>());

    /*
		SubmitMessages

		Should be called each time new messages are submitted for  
		voting. This is specific to 1553 bus mapping. It will iterate
		over each vector entry, recreate a key and attempt to match the key
		with one found in the icd_message_key_to_busnames_map after applying
		the mask. The key = transmit_cmd << 16 | recieve_cmd.
		Each vector should be the same length.

		transmit_cmd	-> vector of transmit command words

		recieve_cmd		-> vector of recieve command words

		channel_ids		-> vector of channel ids associated with each
							transmit and recieve command word

		submission_size	-> optional argument to specify the amount of 
							of entries from the vectors that should
							be submitted

		returns:			True if all vectors are the same size and count is
							less than or equal to the vector sizes.
							False otherwise	
	*/
    bool SubmitMessages(
        const std::vector<uint64_t>& transmit_cmd,
        const std::vector<uint64_t>& recieve_cmd,
        const std::vector<uint64_t>& channel_ids,
        size_t submission_size = -1);

    /*
		SubmitMessage

		Used to submit one message at a time for voting. 
		This is specific to 1553 bus mapping. It will use the 
		transmit and recieve command word and recreate a key. 
		The key = transmit_cmd << 16 | recieve_cmd.
		It will then attempt to match the key with one found
		in the icd_message_key_to_busnames_map after applying
		the mask. 

		transmit_cmd	-> transmit command word

		recieve_cmd		-> recieve command word

		channel_id		-> channel id associated with the
							transmit and recieve command word
	*/
    void SubmitMessage(
        const uint64_t& transmit_cmd,
        const uint64_t& recieve_cmd,
        const uint64_t& channel_id);

    /*
		 Finalize

		 final_map					-> channel ID to bus name map passed by reference and
										filled out by bus map

		 use_tmats_busmap			-> specified in the config file (true or false).
										If true, the tool will use the tmats mapping
										else it will use the default vote method.

		 prompt_user				-> True:  Prompt the user for help with bus mapping
									   False: Don't Prompt the user for help bus mapping

		 user_test_input			-> Used to bypass user input for unit tests

		 Returns: True -> If all channel_ids are mapped AND prompt_user = false
						  |OR| at least one	channel id is mapped 
							AND prompt_user = false
						  |OR| prompt_user is set to true and the user
							selects option 1 to continue with bus map 
				  False-> If zero channel_ids are
							mapped and prompt_user is false
						  |OR| prompt_user is set to true and the user
							selects option "q" to quit translation	
	*/
    bool Finalize(std::map<uint64_t, std::string>& final_map,
                  bool use_tmats_busmap,
                  bool prompt_user,
                  std::vector<std::string>* test_options = NULL);

    /*
		Prints the current vote statistics along with
		the final bus map
	*/
    void Print();

    ///////////////////////// Utilities used for unit tests

    std::unordered_map<uint64_t, std::set<std::string>>
    GetICD_MessageKeyToBusNamesMap()
    {
        return icd_message_key_to_busnames_map_;
    }

    std::unordered_map<uint64_t, std::set<uint64_t>>
    GetICD_MessageKeyToChannelIDSMap()
    {
        return icd_message_key_to_channelids_map_;
    }

    std::set<std::string> GetUniqueBuses()
    {
        return unique_buses_;
    }

    std::set<uint64_t> GetChannelIDs()
    {
        return channel_ids_;
    }

    bool TmatsPresent()
    {
        return tmats_present_;
    }

    const std::map<uint64_t, std::string>&
    GetTMATSchannelidToSourceMap()
    {
        return tmats_chanid_to_source_map_;
    }

    std::map<uint64_t, std::string> TestVoteMapping(
        std::unordered_map<uint64_t, std::set<uint64_t>>
            icd_message_key_to_channelids_map)
    {
        icd_message_key_to_channelids_map_ = icd_message_key_to_channelids_map;
        return VoteMapping();
    }

    const std::map<uint64_t, std::pair<std::string, std::string>>&
    GetFinalBusMap_withSource()
    {
        return final_bus_map_with_sources_;
    }

    std::map<uint64_t, std::string> GetExcludedChannelIDs()
    {
        return excluded_channel_ids_;
    }

    bool UserAdjustments(std::vector<std::string>* test_options = NULL);

    /////////////////////////
};

#endif