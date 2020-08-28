#ifndef PARSEWORKER_H
#define PARSEWORKER_H

//#include <thread>
#include <string>
#include <cstdio>
#include <set>
#include "binbuff.h"
#include <atomic>
#include <filesystem>
#include "tmats.h"
#include "ch10.h"
#include "ch10_packet_header.h"
#include "ch10_packet_stats.h"
#include "ch10_tdf1.h"
#include "ch10_milstd1553f1.h"
#ifdef VIDEO_DATA
#include "ch10_videodataf0.h"
#endif

#ifdef LIBIRIG106
extern "C" {
#include "i106_decode_tmats.h"
#include "i106_decode_time.h"
#include "i106_decode_1553f1.h"
}
#endif

class ParseWorker
{
	
	private:
		// libirig106
#ifdef LIBIRIG106
		int i106_handle_;
		I106C10Header i106_header_;
		I106Status i106_status_;
		int64_t i106_offset_;
		TMATS_Info i106_tmats_info_;
		bool found_tmats_;
		const size_t temp_buffer_size_ = 10e6;
		std::vector<uint8_t> temp_buffer_vec_;
		I106Time i106_time_;
		MS1553F1_Message i106_1553msg_;

#endif
	uint8_t retcode;
	bool continue_parsing;
	bool delete_alloc;
	bool first_tdp;
	bool use_comet_comm_wrd;
	bool final_worker;
	bool is_scan_worker;
	uint16_t id;
	//BinBuff bb;
	std::atomic<bool> complete;
	uint64_t start_position;
	uint64_t last_position;
	uint32_t read_size;
	//BinBuff* bb;
	uint16_t bb_ind;
	std::string output_fname;
	std::map<Ch10DataType, std::string> output_file_names;
	uint32_t first_TDP_loc;
	std::vector<Ch10PacketHeaderStatus> hdr_err;
	uint32_t pkt_count;
	TMATS tdata;
	Ch10PacketHeader* pkthdr;
	Ch10TDF1* tdf;
	Ch10MilStd1553F1* milstd;
	std::map<uint32_t, std::set<uint16_t>> chanid_remoteaddr1_map;
	std::map<uint32_t, std::set<uint16_t>> chanid_remoteaddr2_map;
#ifdef VIDEO_DATA
	Ch10VideoDataF0* video;
#endif
	PacketStats packet_ledger;
	PacketStats packet_error_ledger;
	void parse_and_validate_header();

#ifdef PARQUET
	void generate_parquet_file_names(std::map<Ch10DataType, std::filesystem::path>& fsmap);
#endif
	bool have_generated_file_names;
	
	public:
	ParseWorker();
	~ParseWorker();
	std::atomic<bool>& completion_status();
	void reset_completion_status();

#ifdef PARQUET
	void initialize(uint16_t ID,
		uint64_t start_pos, uint32_t read, uint16_t binbuff_ind,
		std::map<Ch10DataType, std::filesystem::path>& fsmap, 
		TMATS& tmatsdata, bool use_comet_command_words, bool is_final_worker);
	void get_msg_names(std::set<std::string>& output_name_set);
#endif
	void append_mode_initialize(uint32_t read, uint16_t binbuff_ind, uint64_t start_pos);
	uint16_t get_binbuff_ind();
	void operator()(BinBuff& bb, bool append_mode, bool word_count_check, bool milstd1553_msg_selection,
		std::vector<std::string> milstd1553_sorted_selected_msgs);
#ifdef LIBIRIG106
	void operator()(BinBuff& bb, bool append_mode, bool word_count_check, bool milstd1553_msg_selection,
		std::vector<std::string> milstd1553_sorted_selected_msgs, std::vector<std::string>& tmats_body_vec);
#endif
	//void operator()(BinBuff& bb, uint16_t ID, TMATS& tmatsdata);
	uint64_t& get_last_position();
	void time_info(uint64_t&&, uint64_t&&);
	PacketStats* get_packet_ledger();
	PacketStats* get_packet_error_ledger();
	Ch10MilStd1553F1Stats* milstd1553_stats();
	std::string output_file_path();
	std::string output_file_path(Ch10DataType dt);
	void append_chanid_remoteaddr_maps(std::map<uint32_t, std::set<uint16_t>>& out1, std::map<uint32_t, std::set<uint16_t>>& out2);
};

#endif 