#ifndef PARSEWORKER_H
#define PARSEWORKER_H

/*
Execute ch10 parsing on a chunk of binary data using
parser_rewrite lib.
*/

#include <string>
#include <cstdio>
#include <set>
#include <atomic>
#include "ch10_packet_type.h"
#include "ch10_context.h"
#include "ch10_packet.h"
#include "binbuff.h"
#include "iterable_tools.h"
#include "managed_path.h"
#include "spdlog/spdlog.h"

class ParseWorker
{
private:

	// True if worker has completed parsing, false otherwise
	std::atomic<bool> complete_;

	// Track Ch10 state and manipulate metadata
	Ch10Context ctx_;

	// The 0-indexed ParseWorker index
	uint16_t worker_index_;

	// 0-indexed buffer index from which ch10 binary data are read
	uint16_t buffer_index_;

	// Absolute position in ch10 at which ParseWorker begins parsing
	uint64_t start_position_;

	// Absolute position in ch10 at which ParseWorker ends parsing
	uint64_t last_position_;
	
	// True if worker is the final worker used to parse a Ch10
	bool final_worker_;
	
	// Output paths for each Ch10 packet type, specific to the 
	// ParseWorker for which the map is created.
	std::map<Ch10PacketType, ManagedPath> output_file_paths_;

	
public:
	ParseWorker();
	std::atomic<bool>& completion_status();
	void reset_completion_status();

	void initialize(uint16_t worker_index,
		uint64_t start_pos, uint32_t read, uint16_t buffer_index,
		std::map<Ch10PacketType, ManagedPath>& file_path_map, bool is_final_worker);
	void append_mode_initialize(uint32_t read, uint16_t buffer_index, uint64_t start_pos);
	uint16_t get_binbuff_ind();
	
	void operator()(BinBuff& bb, bool append_mode, std::vector<std::string>& tmats_body_vec,
		std::map<Ch10PacketType, bool> ch10_packet_type_map);
	
	uint64_t& get_last_position();
	void append_chanid_remoteaddr_maps(std::map<uint32_t, std::set<uint16_t>>& out1, std::map<uint32_t, std::set<uint16_t>>& out2);
	void append_chanid_comwmwords_map(std::map<uint32_t, std::set<uint32_t>>& out);
	const std::map<uint16_t, uint64_t>& GetChannelIDToMinTimeStampMap();
};

#endif 