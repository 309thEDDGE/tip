#ifndef DATE_H
#define DATE_H
//#include <iostream>
//#include <fstream>

//#include <stdlib.h>
#include <string>
#include <cstdint>
#include <cstdio>
#include <map>
#include <set>
#include <fstream>
#include "binbuff.h"
#include "tmats_item.h"

enum class ChannelDataType : uint8_t
{
	PCMIN = 0,  // PCM input
	ANAIN = 1,  // Analog input
	DISIN = 2,  // Discrete input
	TIMEIN = 3,  // IRIG time input
	VIDIN = 4,  // video input
	UARTIN = 5,  // UART input
	IN1553 = 6,  // 1553 input (Note: actual text descriptor is '1553IN')
	IN429 = 7,  // ARINC 429 input (see note above)
	MSGIN = 8,  // message data input
	IMGIN = 9,  // Image data input 
	IN1394 = 10,  // IEEE-1394 input (see note above)
	PARIN = 11,  // Parallel input
	ETHIN = 12,  // Ethernet Input
	NONE = 13,
};

class TMATS
{

	private:
	const uint64_t TMATS_SKIP;
	uint32_t eof_loc;
	bool tmats_present;
	bool channel_type_present;
	uint8_t parse_lines(BinBuff&);
	void interpret_tmats_line(std::string&);
	std::vector<TMATSItem> items;
	uint8_t map_recorder_channel_to_source();
	std::map<uint32_t, std::string> channel_source_map;
	std::map<uint32_t, ChannelDataType> channel_type_map;
	std::map<uint32_t, std::string> channel_type_as_string_map;
	uint16_t tmats_item_present(std::string);
	bool use_default_bus_id_map;
	uint8_t map_recorder_channel_to_type();

	public:
	const uint32_t EOF_TS;
	const uint32_t EOF_ALT;
	const uint32_t EOF_CALCULEX;
	const uint32_t EOF_CALCULEX2;
	const uint32_t EOF_SEMICOLON_CR_LF_NULL;
	const uint16_t EOL;
	const uint32_t EOF_SIZE;
	
	TMATS();
	TMATS(const TMATS& c);
	void operator = (const TMATS& c);
	~TMATS();
	uint32_t parse(std::ifstream&, const uint64_t& total_size, bool use_def_bus_map);
	
	uint32_t eof_location();
	uint32_t post_tmats_location();
	std::string get_mapped_source_from_channel(uint32_t&);
	void print_mapped_source_from_channel();
	std::map<uint32_t, std::string> map_channel_to_source_by_type(ChannelDataType cdt);
	void replace_channel_source_map(const std::map<uint32_t, std::string>& input_map);
	std::map<uint32_t, std::string> get_channel_source_map();
	std::map<uint32_t, ChannelDataType> get_channel_type_map();
	std::map<uint32_t, std::string> get_channel_type_as_string_map();
	bool using_tmats_data();
};
#endif
