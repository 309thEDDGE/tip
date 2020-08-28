#ifndef PARQUET_MILSTD1553F1_H
#define PARQUET_MILSTD1553F1_H

#include "parquet_context.h"
#include "MilStd1553F1Format.h"
#include <set>

const int DEFAULT_ROW_GROUP_COUNT = 10000;
const int DEFAULT_BUFFER_SIZE_MULTIPLIER = 10;
const int DATA_PAYLOAD_LIST_COUNT = 32;

class ParquetMilStd1553F1 : public ParquetContext
{
private:
	int max_temp_element_count_;
	int temp_element_count_;
	uint16_t id_;
	uint16_t* commword_ptr_;

	// Set of msg names.
	std::set<std::string> name_set_;

	// Arrays of data to be written to the Parquet table.
	std::vector<uint64_t> time_stamp_;  // save as int64
	std::vector<uint8_t> doy_;  // save as int16
	std::vector<int8_t> ttb_;  // save as int8
	std::vector<uint8_t> WE_;  // save as single bit
	std::vector<uint8_t> SE_;  // save as single bit
	std::vector<uint8_t> WCE_;  // save as single bit
	std::vector<uint8_t> TO_;  // save as single bit
	std::vector<uint8_t> FE_;  // save as single bit
	std::vector<uint8_t> RR_;  // save as single bit
	std::vector<uint8_t> ME_;  // save as single bit
	std::vector<uint8_t> gap1_;  // save as int16
	std::vector<uint8_t> gap2_;  // save as int16
	std::vector<uint8_t> mode_code_;  // save as single bit
	std::vector<uint16_t> data_;  // for all data payloads, save as int16
	//std::vector<int8_t> word_count_;  // save as int8
	std::vector<uint16_t> comm_word1_; 
	std::vector<uint16_t> comm_word2_;
	std::vector<int8_t> rtaddr1_;
	std::vector<uint8_t> tr1_; // save as single bit
	std::vector<int8_t> subaddr1_;
	std::vector<int8_t> wrdcnt1_; 
	std::vector<int8_t> rtaddr2_;
	std::vector<uint8_t> tr2_; // save as single bit
	std::vector<int8_t> subaddr2_;
	std::vector<int8_t> wrdcnt2_;
	std::vector<uint16_t> channel_id_;
	std::vector<int8_t> payload_count_;
	//std::vector<int16_t> msglen_;

public:
	ParquetMilStd1553F1();
	ParquetMilStd1553F1(std::string outfile, uint16_t ID, bool truncate);
	void append_data(const uint64_t& time_stamp, uint8_t doy, const char* name, 
		const MilStd1553F1ChanSpecFormat* chan_spec,
		const MilStd1553F1MsgCommWord* msg, const uint16_t* data, const uint16_t& chanid, 
		const uint16_t& payload_count);
	void commit();
	void add_names_to_set(std::set<std::string>& output_set);
};

#endif