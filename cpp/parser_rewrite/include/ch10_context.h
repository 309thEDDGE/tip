
#ifndef CH10_CONTEXT_H_
#define CH10_CONTEXT_H_

#include <cstdint>
#include <cstdio>
#include <map>
#include <unordered_map>
#include <set>
#include <cmath>
#include <memory>
#include "managed_path.h"
#include "ch10_status.h"
#include "ch10_header_format.h"
#include "ch10_1553f1_msg_hdr_format.h"
#include "parquet_milstd1553f1.h"
#include "ch10_videof0_header_format.h"
#include "parquet_videodataf0.h"
#include "spdlog/spdlog.h"

enum class Ch10PacketType : uint8_t
{
	/*
	WARNING! Update CreatePacketTypeConfigReference with
	any packet types that are added to this enum! Also update
	relevant unit tests in ch10_context_u.cpp.
	*/
	NONE					   = 0xFF,
	COMPUTER_GENERATED_DATA_F1 = 0x01,
	TIME_DATA_F1               = 0x11,
	MILSTD1553_F1              = 0x19,
	VIDEO_DATA_F0              = 0x40,
};

class Ch10Context
{
private:

	// ID to control or generate thread-specific log output.
	uint16_t thread_id_;

	// Key components of Ch10 context.
	uint64_t absolute_position_;
	uint64_t tdp_rtc_; // nanosecond
	uint64_t tdp_abs_time_; // nanosecond
	uint64_t rtc_;  // nanosecond
	uint32_t pkt_size_;
	uint32_t data_size_;

	// Temporary RTC time in nanosecond
	uint64_t temp_rtc_;

	// Conversion factor for relative time counter (RTC)
	// to nanoseconds.
	// Unit: count/ns
	const uint64_t rtc_to_ns_;
	
	// Reference map for packet type on/off state.
	// This map must include all elements of Ch10PacketType to
	// ensure that all types can be configured for on/off correctly.
	std::unordered_map<Ch10PacketType, bool> pkt_type_config_map_;
	
	// Store status of enabled file writers.
	std::unordered_map<Ch10PacketType, bool> pkt_type_file_writers_enabled_map_;

	bool searching_for_tdp_;
	bool found_tdp_;

	// Indicates that tdp data are relevant. This is only
	// valid after found_tdp_ is set to true. If this var 
	// is false, then tdp data are not available, absolute time
	// can't be computed, and rtc time ought to be used in 
	// place of absolute time.
	bool tdp_valid_;

	// 1 if IRIG time (day of year) is found in the TDP packet.
	// 0 if otherwise (day, month, year). This value is only
	// relevant after the TDP has been parsed, i.e., found_tdp_
	// is set to true.
	uint8_t tdp_doy_; 

	// Intra-packet time stamp source, from Ch10PacketHeaderFmt::intrapkt_ts_source.
	// 0 = intra-packet TS source is the header RTC and should be parsed as RTC.
	// 1 = intra-packet TS source is the header secondary time stamp and should
	// be parsed as indicated by Ch10PacketHeaderFmt::time_fmt.
	uint8_t intrapkt_ts_src_;

	// Indicates the ch10 packet header secondary header time format, 
	// if present (indicated by Ch10PacketHeaderFmt::secondary_hdr = 1)
	uint8_t time_format_;

	// Channel ID from Ch10PacketHeaderFmt (chanID), identifies data
	// source for the contents of the packet.
	uint32_t channel_id_;

	// Record mapping of 1553 message channel ID to lru addresses.
	std::map<uint32_t, std::set<uint16_t>> chanid_remoteaddr1_map_;
	std::map<uint32_t, std::set<uint16_t>> chanid_remoteaddr2_map_;

	// Record a 32-bit integer calculated from the command word(s).
	// Used for bus mapping. Map channel id to set of these command
	// word values.
	std::map<uint32_t, std::set<uint32_t>> chanid_commwords_map_;

	// Reference command words 1 and 2 in the 1553
	// message data header.
	const uint16_t* command_word1_;
	const uint16_t* command_word2_;

	// Set to true if CheckConfiguration has been executed
	// and its return value was true. 
	bool is_configured_;

	// 
	// File writers are owned by Ch10Context to maintain state
	//
	std::unique_ptr<ParquetMilStd1553F1> milstd1553f1_pq_writer_;
	std::unique_ptr<ParquetVideoDataF0> videof0_pq_writer_;


public:
	const uint16_t& thread_id;
	const uint64_t& absolute_position;
	const uint64_t& tdp_rtc;
	const uint64_t& tdp_abs_time;
	const uint64_t& rtc;
	const uint32_t& pkt_size;
	const uint32_t& data_size;
	const bool& tdp_valid;
	const uint8_t& tdp_doy;
	const bool& found_tdp;
	const uint8_t& intrapkt_ts_src;
	const uint8_t& time_format;
	const uint32_t& channel_id;
	const std::unordered_map<Ch10PacketType, bool>& pkt_type_config_map;
	const std::map<uint32_t, std::set<uint16_t>>& chanid_remoteaddr1_map;
	const std::map<uint32_t, std::set<uint16_t>>& chanid_remoteaddr2_map;
	const std::map<uint32_t, std::set<uint32_t>>& chanid_commwords_map;
	ParquetMilStd1553F1* milstd1553f1_pq_writer;
	const ParquetVideoDataF0* videof0_pq_writer;
	const uint32_t intrapacket_ts_size_ = 2*sizeof(uint64_t);

	Ch10Context(const uint64_t& abs_pos, uint16_t id = 0);
	Ch10Context();
	void Initialize(const uint64_t& abs_pos, uint16_t id);
	~Ch10Context();

	/*
	Return is_configured_. This value is set during call to CheckConfiguration.
	If true, then CheckConfiguration has been called, presumably with the 
	maps set during calls to SetPacketTypeConfig and SetOutputPathsMap and
	all necessary data are present and ready for parsing.
	*/
	bool IsConfigured();

	void SetSearchingForTDP(bool should_search);
	Ch10Status ContinueWithPacketType(uint8_t data_type);

	/*
	Update the members that are of primary importance for conveyance
	to the packet body parsers, including re-calculation of the current
	packet absolute time based on TDP abs time, RTC, and the current packet
	RTC.

	Use the chanID in Ch10PacketHeaderFmt to insert an empty set
	in the chanid to remote lru addresses maps.

	Args:

		abs_pos			--> absolute byte position within the Ch10
		hdr_fmt_ptr_	--> pointer to Ch10PacketHeaderFmt

	*/
	void UpdateContext(const uint64_t& abs_pos, const Ch10PacketHeaderFmt* const hdr_fmt_ptr_);


	void CreateDefaultPacketTypeConfig(std::unordered_map<Ch10PacketType, bool>& input);

	/*
	Use a user-input map of Ch10PacketType to bool to assemble the 
	pkt_type_config_map_.

	Args:

		user_config --> map of Ch10PacketType to bool. For the user-submitted
		example map, 
			{Ch10PacketType::MILSTD1553_F1 --> true},
			{Ch10PacketType::VIDEO_DATA_F0 --> false}
		1553 will be parsed and video (type f0) will not be parsed. TMATS
		(computer generated data, format 1) and time data packets (time data f1)
		cannot be turned off. Data types that are not configured will default
		to true.
	*/
	void SetPacketTypeConfig(const std::map<Ch10PacketType, bool>& user_config);

	/*
	Update tdp_rtc_, tdp_abs_time_, tdp_doy_, tdp_valid_ and found_tdp_,
	member vars necessary
	for absolute time calculation for other data packets. 

	Args:
		tdp_abs_time	--> absolute time in nanoseconds since the epoch
							as calculated from data retrieved from the 
							TDP packet
		tdp_doy			--> bit value, 1 = day-of-year time (IRIG time),
							0 = year-mth-day time
		tdp_valid		--> true if tdp csdw time_fmt or src are none,
							false otherwise. Set to false if tdp data is invalid.

	*/
	void UpdateWithTDPData(const uint64_t& tdp_abs_time, uint8_t tdp_doy,
		bool tdp_valid);

	/*
	Calculate absolute time from RTC time format components. This context
	must have been updated with UpdateWithTDPData prior to this call
	in order for absolute time to be calculated correctly.

	Args:
		rtc1	--> first 32-bit component of rtc counter
		rtc2	--> second 32-bit component of rtc counter

	Return:
		absolute time in nanoseconds since the epoch
	*/
	uint64_t CalculateAbsTimeFromRTCFormat(const uint64_t& rtc1, const uint64_t& rtc2);

	/*
	Caculate this packet's absolute time
	
	Return:
		absolute time this packet was received in nanoseconds since the epoch
	*/
	uint64_t GetPacketAbsoluteTime();
	
	/*
	Update maps of channel ID to remote addresses 1 and 2 as obtained from
	the 1553 intra-packet data headers.

	Update the channel ID to command words map. The integer inserted into
	the set of uint32_t is the upshifted command word1 + command word2.
	*/
	void UpdateChannelIDToLRUAddressMaps(const uint32_t& chanid,
		const MilStd1553F1DataHeaderCommWordFmt* data_header);

	/*
	Check if the configurations for packet type and output paths are consistent.
	Return false if an enabled packet type does not have an output file specified.
	Generate a Ch10PacketType to ManagedPath map that is consistent with enabled 
	packet types and the selection of types for which a mapped ManagedPath exists
	in the pkt_type_paths_config input map. The generated map is stored in map
	that's passed by reference, 

	Args:
		pkt_type_enabled_config	--> Map of Ch10PacketType to bool. Use map set by call
									to SetPacketTypeConfig.
		pkt_type_paths_config	--> Map of Ch10PacketType to ManagedPath. Use map set 
									by call to SetOutputPathsMap
		enabled_paths			--> Reference to map of Ch10PacketType to ManagedPath,
									the output of the configuration check. Only valid
									if the return of this function is true.

	Return:
		True if configuration is valid and key-value pairs inserted in enabled_paths
		map are to be used. False otherwise.
	*/
	bool CheckConfiguration(const std::unordered_map<Ch10PacketType, bool>& pkt_type_enabled_config,
		const std::map<Ch10PacketType, ManagedPath>& pkt_type_paths_config,
		std::map<Ch10PacketType, ManagedPath>& enabled_paths);

	/*
	Initialize file writers for the various enabled packet types. This function
	exists to avoid initializing file writers by default in the various parser
	constructors, which makes testing difficult.

	Instantiate file writers for each of the mapped types using the ManagedPath
	values.

	Args:
		enabled_paths	--> Const ref to map of Ch10PacketType to ManagedPath
							containing only types that have been enabled (SetPacketTypeConfig)
							by the user. This map is to be generated by CheckConfiguration.
	*/
	void InitializeFileWriters(const std::map<Ch10PacketType, ManagedPath>& enabled_paths);

	/*
	Close file writers for the various enabled packet types. Uses 
	pkt_type_file_writers_enabled_map_, which is created during the call to 
	InitializeFileWriters and stored as a private member var.
	*/
	void CloseFileWriters();
};


#endif