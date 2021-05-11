#ifndef PARSEWORKER_H
#define PARSEWORKER_H

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
	Ch10Context ctx;
	std::map<uint16_t, uint64_t> chanid_minvideotimestamp_map_;
	bool final_worker;
	uint16_t id;
	uint16_t bb_ind;
	std::atomic<bool> complete;
	uint64_t start_position;
	uint64_t last_position;
	std::map<Ch10PacketType, ManagedPath> output_file_paths_;

	
	public:
	ParseWorker();
	~ParseWorker();
	std::atomic<bool>& completion_status();
	void reset_completion_status();

	void initialize(uint16_t ID,
		uint64_t start_pos, uint32_t read, uint16_t binbuff_ind,
		std::map<Ch10PacketType, ManagedPath>& file_path_map, bool is_final_worker);
	void append_mode_initialize(uint32_t read, uint16_t binbuff_ind, uint64_t start_pos);
	uint16_t get_binbuff_ind();
	
	void operator()(BinBuff& bb, bool append_mode, std::vector<std::string>& tmats_body_vec,
		std::map<Ch10PacketType, bool> ch10_packet_type_map);
	
	uint64_t& get_last_position();
	void append_chanid_remoteaddr_maps(std::map<uint32_t, std::set<uint16_t>>& out1, std::map<uint32_t, std::set<uint16_t>>& out2);
	void append_chanid_comwmwords_map(std::map<uint32_t, std::set<uint32_t>>& out);
	const std::map<uint16_t, uint64_t>& GetChannelIDToMinTimeStampMap();
};

#endif 