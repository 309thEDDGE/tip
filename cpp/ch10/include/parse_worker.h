#ifndef PARSEWORKER_H
#define PARSEWORKER_H

//#include <thread>
#include <string>
#include <cstdio>
#include <set>
#include "binbuff.h"
#include <atomic>
#include <filesystem>
#include "ch10.h"
#include "ch10_packet_header.h"
#include "ch10_packet_stats.h"
#include "ch10_tdf1.h"
#include "ch10_milstd1553f1.h"
#include "iterable_tools.h"
#include "managed_path.h"
#ifdef VIDEO_DATA
#include "ch10_videodataf0.h"
#endif

#ifdef LIBIRIG106
#include "i106_parse_context.h"

#ifdef ETHERNET_DATA
#include "i106_ch10_ethernetf0.h"
#endif

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

		// Native TIP adaptation to LibIRIG106
		Ch10MetaData ch10md_;
#ifdef ETHERNET_DATA
		I106Ch10EthernetF0 i106_ethernetf0_;
#endif

#endif
	uint8_t retcode;
	bool continue_parsing;
	bool delete_alloc;
	bool first_tdp;
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
	std::map<Ch10DataType, ManagedPath> output_file_paths_;
	uint32_t first_TDP_loc;
	std::vector<Ch10PacketHeaderStatus> hdr_err;
	uint32_t pkt_count;
	Ch10PacketHeader* pkthdr;
	Ch10TDF1* tdf;
	Ch10MilStd1553F1* milstd;
	std::map<uint32_t, std::set<uint16_t>> chanid_remoteaddr1_map;
	std::map<uint32_t, std::set<uint16_t>> chanid_remoteaddr2_map;
	std::map<uint32_t, std::set<uint32_t>> chanid_commwords_map;
#ifdef VIDEO_DATA
	Ch10VideoDataF0* video;
#endif
	PacketStats packet_ledger;
	PacketStats packet_error_ledger;
	void parse_and_validate_header();
	bool have_generated_file_names;
	
	public:
	ParseWorker();
	~ParseWorker();
	std::atomic<bool>& completion_status();
	void reset_completion_status();

#ifdef PARQUET
	void initialize(uint16_t ID,
		uint64_t start_pos, uint32_t read, uint16_t binbuff_ind,
		std::map<Ch10DataType, ManagedPath>& file_path_map, bool is_final_worker);
#endif
	void append_mode_initialize(uint32_t read, uint16_t binbuff_ind, uint64_t start_pos);
	uint16_t get_binbuff_ind();
	void operator()(BinBuff& bb, bool append_mode);
#ifdef LIBIRIG106
	void operator()(BinBuff& bb, bool append_mode, std::vector<std::string>& tmats_body_vec);
#endif
	//void operator()(BinBuff& bb, uint16_t ID, TMATS& tmatsdata);
	uint64_t& get_last_position();
	void time_info(uint64_t&&, uint64_t&&);
	PacketStats* get_packet_ledger();
	PacketStats* get_packet_error_ledger();
	Ch10MilStd1553F1Stats* milstd1553_stats();
	void append_chanid_remoteaddr_maps(std::map<uint32_t, std::set<uint16_t>>& out1, std::map<uint32_t, std::set<uint16_t>>& out2);
	void append_chanid_comwmwords_map(std::map<uint32_t, std::set<uint32_t>>& out);
#ifdef VIDEO_DATA
	const std::map<uint16_t, uint64_t>& GetChannelIDToMinTimeStampMap();
#endif
};

#endif 