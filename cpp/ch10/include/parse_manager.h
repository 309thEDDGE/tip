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
#include <chrono>
#include <sstream>
#include <iomanip>
#include "iterable_tools.h"
#include "parse_text.h"
#include "parse_worker.h"
#include "parser_config_params.h"
#include "metadata.h"
#include "tmats_parser.h"
#include "managed_path.h"
#include "worker_config.h"
#include "spdlog/spdlog.h"
#include "ch10_packet_type.h"

class ParseManager
{
	private:

	// TMATS raw data
	std::vector<std::string> tmats_body_vec_;

	// Metadata manipulation
	IterableTools it_;
	
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
	bool check_word_count;
	std::ifstream ifile;
	ParseWorker* workers;
	//BinBuff* binary_buffers;
	WorkerConfig* worker_config_;
	std::thread* threads;
	bool workers_allocated;
	const ParserConfigParams * const config_;
	
	std::map<Ch10PacketType, ManagedPath> output_dir_map_;
	std::vector<std::map<Ch10PacketType, ManagedPath>> output_file_path_vec_;
	bool milstd1553_msg_selection;
	std::vector<std::string> milstd1553_sorted_msg_selection;
	std::string chanid_to_lruaddrs_metadata_string_;

	std::map<Ch10PacketType, bool> packet_type_config_map_;

	/*std::streamsize activate_worker(uint16_t binbuff_ind, uint16_t ID,
		uint64_t start_pos, uint32_t n_read);
	std::streamsize activate_append_mode_worker(uint16_t binbuff_ind, uint16_t ID,
		uint32_t n_read);*/

	std::streamsize new_activate_worker(ParseWorker* worker_vec, WorkerConfig* worker_config,
		uint16_t worker_index, uint64_t& read_pos, uint32_t& read_size);
	std::streamsize new_activate_append_mode_worker(ParseWorker* worker_vec, 
		WorkerConfig* worker_config, uint16_t worker_index, uint32_t& read_size);

	// worker automation
	std::vector<uint16_t> active_workers;
	std::chrono::milliseconds worker_wait;
	std::chrono::milliseconds worker_start_offset;
	//void worker_queue(bool append_mode);
	void new_worker_queue(bool append_mode);
	//void worker_retire_queue();
	void new_worker_retire_queue();
	void create_output_file_paths();
	void collect_chanid_to_lruaddrs_metadata(
		std::map<uint32_t, std::set<uint16_t>>& output_chanid_remoteaddr_map);
	void collect_chanid_to_commwords_metadata(
		std::map<uint32_t, std::vector<std::vector<uint32_t>>>& output_chanid_commwords_map);
	
	void CollectVideoMetadata(std::map<uint16_t, uint64_t>& channel_id_to_min_timestamp_map);
	void ProcessTMATS();

	public:

	ParseManager(ManagedPath fname, ManagedPath output_path, const ParserConfigParams * const config);
	void start_workers();
	~ParseManager();

	/*
	Collect misc setup functions for readability

	Return:
		True if no errors, false if errors occur and
		execution ought to stop.
	*/
	bool Setup();

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

	/*
	Create and verify output directories for enabled ch10 packet types. 

	Args:
		output_dir			--> ManagedPath object giving the output directory into 
								which packet type-specific dirs will be created
		base_file_name		--> ManagedPath object with the base file name on which
								to build the output directory name. May be a complete 
								path to a file, in which case the parent path will be 
								stripped and only the file name used, or a file name
								only.
		packet_enabled_map	--> Map of Ch10PacketType to boolean. True = enabled,
								False = disabled
		append_str_map		--> Map of Ch10PacketType to string. The mapped string is
								appended to the base_file_name prior and ought to
								include an extension if necessary, such as ".parquet"
								in the case of Parquet files. Map must contain at 
								least all of the keys in the packet_enabled_map which
								are also mapped to true.
		pkt_type_output_dir_map --> Map of Ch10PacketType to specific output directory,
									the product of this function
		create_dir				--> True if the directory ought to be created, false
									otherwise

	Return:
		True if successful and all output directories were created; false otherwise.
	*/
	bool CreateCh10PacketOutputDirs(const ManagedPath& output_dir,
		const ManagedPath& base_file_name,
		const std::map<Ch10PacketType, bool>& packet_enabled_map,
		const std::map<Ch10PacketType, std::string>& append_str_map,
		std::map<Ch10PacketType, ManagedPath>& pkt_type_output_dir_map, bool create_dir);

	/*
	Generate a vector of maps of Ch10PacketType to ManagedPath. The path object
	is a file path to which data for the given Ch10PacketType
	ought to be written by the worker associated with the index of the vector
	from which the map was retrieved. 

	Args:
		total_worker_count			--> Count of workers expected to be
										created to parse according to 
										configuration settings
		pkt_type_output_dir_map		--> Map of Ch10PacketType to base output
										directory to which files associated
										with the packet type ought to be written.
										This is the pkt_type_output_dir_map from
										CreateCh10PacketOutputDirs.
		output_vec_mapped_paths		--> Vector of maps in which the index in
										the vector is the same as the worker which
										ought to utilize the mapped output file
										paths.
		file_extension				--> String not including the '.'.
										Ex: file_extension = 'txt'
	*/
	void CreateCh10PacketWorkerFileNames(const uint16_t& total_worker_count,
		const std::map<Ch10PacketType, ManagedPath>& pkt_type_output_dir_map,
		std::vector< std::map<Ch10PacketType, ManagedPath>>& output_vec_mapped_paths,
		std::string file_extension);

};

#endif