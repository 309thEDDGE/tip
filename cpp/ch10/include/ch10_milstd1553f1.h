// ch10_milstd1553f1.h

// TODO: Use base class (ParseContext) members for time data storage and 
// calculation. E.g., abstime -> absolute_pkt_TS, etc. See ch10_videodataf0.cpp initialize
// function and time calculations.

#ifndef CH10MILSTD1553F1_H
#define CH10MILSTD1553F1_H

#include "parse_context.h"
#include "MilStd1553F1Format.h"
#include "ch10_milstd1553f1stats.h"
#include "ch10_packet_stats.h"
#include "tmats.h"
#include <ctime>
#include <cstring>

#ifdef LOCALDB
#ifdef PARQUET
#include "parquet_milstd1553f1.h"
#endif
#endif

#ifdef LIBIRIG106
extern "C" {
#include "i106_decode_1553f1.h"
}
#endif

class Ch10MilStd1553F1 : public ParseContext<MilStd1553F1ChanSpecFormat, MilStd1553F1Status>
{
private:
#ifdef LIBIRIG106
	I106Status i106_status_;
	MS1553F1_Message i106_1553msg_;
#endif
	Ch10MilStd1553F1Stats stats;
	const MilStd1553F1Msg* msg;
	const MilStd1553F1MsgCommWord* msg_commword;
	uint8_t busid;
	int8_t command_word_payload_count_;
	uint8_t is_payload_incomplete_;
	/*const CommandWord* comm_word;
	const CommandWord* comm_word2;*/
	uint32_t msg_hdr_size;
	uint32_t intrapkt_hdr_size;
	uint32_t msg_size;
	uint16_t msg_index;
	uint16_t channel_id;
	std::string bus_name;
	const uint16_t* datum;
	int datum_shift;
	int8_t datum_count;
	uint8_t RTtoBC;
	uint16_t zero;
	bool check_word_count;
	bool use_comet_command_words;
	bool use_selected_msg_list;
	std::vector<std::string> selected_msg_names;
	std::vector<std::string>::iterator msg_names_start;
	std::vector<std::string>::iterator msg_names_end;
	std::vector<std::string>::iterator msg_names_itr;
	std::map<uint32_t, std::set<uint16_t>>* chanid_remoteaddr1_ptr;
	std::map<uint32_t, std::set<uint16_t>>* chanid_remoteaddr2_ptr;
	void debug_info();
	//void msg_debug_info();
	void msg_debug_info2();
	//uint8_t parse_payload();
	uint8_t parse_payload_new();
	//uint8_t parse_payload_without_comet_command();
	//uint8_t parse_payload_without_comet_command_improved();

	TMATS& tdata;
	//uint8_t write_data();

#ifdef LOCALDB
#ifdef PARQUET
	ParquetMilStd1553F1 db;
#endif
#endif
	
public:
	char* msg_name;
	uint8_t* word_count;
#ifdef LOCALDB
#ifdef PARQUET
	Ch10MilStd1553F1(BinBuff& buff, uint16_t ID, TMATS& tmats,
		bool wc_check, bool use_comet_comm_wrd, std::string out_path, bool msg_selection,
		std::vector<std::string> sorted_msg_selection_names) : ParseContext(buff, ID), msg(nullptr),
		use_selected_msg_list(msg_selection),
		selected_msg_names(sorted_msg_selection_names),
		datum(nullptr), channel_id(0), command_word_payload_count_(0),
		msg_hdr_size(14), msg_size(0), datum_count(0), msg_commword(nullptr),
		RTtoBC(0), msg_index(0), msg_name(nullptr), word_count(nullptr),
		tdata(tmats), zero(0), datum_shift(0), stats(), check_word_count(wc_check),
		intrapkt_hdr_size(sizeof(*msg)), use_comet_command_words(use_comet_comm_wrd),
		db(out_path, ID, true), is_payload_incomplete_(0),
		msg_names_start(selected_msg_names.begin()), msg_names_end(selected_msg_names.end()),
		chanid_remoteaddr1_ptr(nullptr), chanid_remoteaddr2_ptr(nullptr), busid(UINT8_MAX)
	{	}
	void get_msg_names(std::set<std::string>& output_name_set);
#endif
#endif

	Ch10MilStd1553F1(BinBuff& buff, uint16_t ID, TMATS& tmats, bool wc_check, bool use_comet_comm_wrd,
		bool msg_selection) : ParseContext(buff, ID), msg(nullptr), tdata(tmats),
		use_selected_msg_list(msg_selection), command_word_payload_count_(0),
		datum(nullptr), channel_id(0), is_payload_incomplete_(0),
		msg_hdr_size(14), msg_size(0), datum_count(0), msg_commword(nullptr),
		RTtoBC(0), msg_index(0), msg_name(nullptr), word_count(nullptr),
		zero(0), datum_shift(0), stats(), check_word_count(wc_check),
		intrapkt_hdr_size(sizeof(*msg)), use_comet_command_words(use_comet_comm_wrd),
		chanid_remoteaddr1_ptr(nullptr), chanid_remoteaddr2_ptr(nullptr), busid(UINT8_MAX) {}

	~Ch10MilStd1553F1();
	void Initialize(const Ch10TimeData* ch10td, const Ch10HeaderData* ch10hd) override;
	//void scan_initialize(uint32_t chanID);
	uint8_t Parse() override;
	//uint8_t parse_minimal();
	void set_channelid_remoteaddress_output(std::map<uint32_t, std::set<uint16_t>>* map_remoteaddr1, 
		std::map<uint32_t, std::set<uint16_t>>* map_remoteaddr2);
	void set_truncate(bool state);
	void close();
	Ch10MilStd1553F1Stats& get_stats();
#ifdef LIBIRIG106
	uint8_t UseLibIRIG106(I106C10Header* i106_header, void* buffer);
	uint8_t IngestLibIRIG106Msg();
	void ParsePayloadLibIRIG106();
#endif
};

#endif