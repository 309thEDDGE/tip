// parse_manager.h

/* ParseManager is called by the main() ch10parse entry point.
   It has functions and data members for managing multiple threads
   which in turn are each represented by a ParseWorker instance.
*/ 

#ifndef PARSEMANAGER_H
#define PARSEMANAGER_H

#include <cstdint>
#include <cstdio>
#include <string>
#include <thread>
#include <vector>
#include "iterable_tools.h"
#include "parse_text.h"
#include "parse_worker.h"
#include "ch10.h"
#include "ch10_milstd1553f1stats.h"
#include "ch10_packet_stats.h"
#include <chrono>
#include "parser_config_params.h"
#include "metadata.h"
#include "tmats_parser.h"
#include "managed_path.h"
#include "spdlog/spdlog.h"
#include "ch10_packet_type.h"

class ParseManager
{
	private:
#ifdef PARQUET
	std::set<std::string> name_set;
#endif

	// TMATS processing
	std::vector<std::string> tmats_body_vec_;
	void ProcessTMATS();
	std::map<std::string, std::string> TMATsChannelIDToSourceMap_;
	std::map<std::string, std::string> TMATsChannelIDToTypeMap_;

	ManagedPath input_path;
	ManagedPath output_path;
	uint32_t read_size;
	uint32_t append_read_size;
	uint64_t total_size;
	uint64_t total_read_pos;
	uint8_t n_threads;
	uint16_t n_reads;
	//bool tmats_present;
	bool error_set;
	bool check_word_count;
	std::ifstream ifile;
	ParseWorker* workers;
	BinBuff* binary_buffers;
	std::thread* threads;
	bool workers_allocated;
	const ParserConfigParams * const config_;
	
	std::map<Ch10DataType, ManagedPath> output_dir_map_;
	std::vector<std::map<Ch10DataType, ManagedPath>> output_file_path_vec_;
	bool milstd1553_msg_selection;
	std::vector<std::string> milstd1553_sorted_msg_selection;
	std::string chanid_to_lruaddrs_metadata_string_;
	std::map<Ch10PacketType, bool> packet_type_config_map_;

	std::streamsize activate_worker(uint16_t binbuff_ind, uint16_t ID,
		uint64_t start_pos, uint32_t n_read);
	std::streamsize activate_append_mode_worker(uint16_t binbuff_ind, uint16_t ID,
		uint32_t n_read);
	void collect_stats();

	// worker automation
	std::vector<uint16_t> active_workers;
	std::chrono::milliseconds worker_wait;
	std::chrono::milliseconds worker_start_offset;
	void worker_queue(bool append_mode);
	void worker_retire_queue();
	void create_output_dirs();
	void create_output_file_paths();
	void collect_chanid_to_lruaddrs_metadata(
		std::map<uint32_t, std::set<uint16_t>>& output_chanid_remoteaddr_map);
	void collect_chanid_to_commwords_metadata(
		std::map<uint32_t, std::vector<std::vector<uint32_t>>>& output_chanid_commwords_map);
	
#ifdef VIDEO_DATA
	void CollectVideoMetadata(std::map<uint16_t, uint64_t>& channel_id_to_min_timestamp_map);
#endif

	public:

	//ParseManager(ACPlatform plat, std::string fname, std::string output_path, ConfigManager& cm_parse, ConfigManager& cm_1553);
	ParseManager(ManagedPath fname, ManagedPath output_path, const ParserConfigParams * const config);
	bool error_state();
	void start_workers();
	~ParseManager();

	// Used for unit tests
	void ProcessTMATsTest(const std::vector<std::string>& input)
	{
		tmats_body_vec_ = input;
		ProcessTMATS();
	};
	std::map<std::string, std::string> GetTMATsChannelIDToSourceMap() { return TMATsChannelIDToSourceMap_; };
	std::map<std::string, std::string> GetTMATsChannelIDToTypeMap() { return TMATsChannelIDToTypeMap_; };

	/*
	Convert the ch10_packet_type configuration map that is read from the
	parse_conf.yaml as a map<std::string, std::string> to a map<Ch10PacketType, bool>.

	Args: 
		input_map	--> map<string, string> of the raw ch10_packet_type data 
						structure found in the parse_conf.yaml file
		output_map	--> map<Ch10PacketType, bool> passed by reference

	Return:
		True if the conversion is successful, false otherwise.
	*/
	bool ConvertCh10PacketTypeMap(const std::map<std::string, std::string>& input_map,
		std::map<Ch10PacketType, bool>& output_map);

	/*
	Log the ch10_packet_type_map_. This is a convenience function to clean up
	the clutter that this code introduces.

	Args:
		pkt_type_config_map	--> Input map of Ch10PacketType to bool that 
								represents the enable state of ch10 packet types
	*/
	void LogPacketTypeConfig(const std::map<Ch10PacketType, bool>& pkt_type_config_map);

};

#endif