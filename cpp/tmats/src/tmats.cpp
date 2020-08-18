// tmats.cpp
#include "tmats.h"

TMATS::TMATS() : EOF_TS(0x00000A0D), EOF_ALT(0xFFFF0A0D),
EOF_CALCULEX(0xF4F60A0D), EOL(0x0A0D), TMATS_SKIP(28),
EOF_SIZE(sizeof(EOF_TS)), eof_loc(0), tmats_present(false),
EOF_CALCULEX2(0x0D200A0D),
EOF_SEMICOLON_CR_LF_NULL(0x000A0D3B), channel_type_present(false)
{

}

TMATS::TMATS(const TMATS& c) : EOF_TS(0x00000A0D), EOF_ALT(0xFFFF0A0D),
EOF_CALCULEX(0xF4F60A0D), EOL(0x0A0D), TMATS_SKIP(28),
EOF_SIZE(sizeof(EOF_TS)), EOF_CALCULEX2(0x0D200A0D),
eof_loc(0), EOF_SEMICOLON_CR_LF_NULL(0x000A0D3B)
{
	tmats_present = c.tmats_present;
	items.assign(c.items.begin(), c.items.end());
	channel_source_map = c.channel_source_map;
	//channel_bus_map = c.channel_bus_map;
	channel_type_map = c.channel_type_map;
	channel_type_present = c.channel_type_present;
}

void TMATS::operator = (const TMATS& c)
{
	tmats_present = c.tmats_present;
	items.assign(c.items.begin(), c.items.end());
	channel_source_map = c.channel_source_map;
	//channel_bus_map = c.channel_bus_map;
	channel_type_map = c.channel_type_map;
	channel_type_present = c.channel_type_present;
	eof_loc = c.eof_loc;
	//size_t map_size = channel_source_map.size();
	//std::vector<uint32_t> keys(map_size);
	//printf("orig size: %zu, map size: %zu\n", c.channel_source_map.size(), map_size);
}

TMATS::~TMATS()
{

}

uint32_t TMATS::parse(std::ifstream& infile, const uint64_t& total_size, bool use_def_bus_map)
{
	use_default_bus_id_map = use_def_bus_map;
	
	/* Currently without an option to configure TMATS codes of interest,
	   fill the TMATSItems vector with the required codes. */
	TMATSItem chanid("R-x\\\\TK1-n", TMATSDataType::INT);
	TMATSItem sourceid("R-x\\\\DSI-n", TMATSDataType::STR);
	TMATSItem data_type("R-x\\\\CDT-n", TMATSDataType::STR);

	items.push_back(chanid);
	items.push_back(sourceid);
	items.push_back(data_type);

	tmats_present = false;

	// Find one of the EOF markers. 
	uint32_t read_size = 50000;
	//uint8_t buff[read_size];
	// int eof_ts_size = sizeof(EOF_TS);
	// int eof_alt_size = sizeof(EOF_ALT);
	// int eof_calc_size = sizeof(EOF_CALCULEX);
	uint64_t read_pos = 0;

	BinBuff bb;
	bb.Initialize(infile, total_size, read_pos, read_size);
	bb.AdvanceReadPos(TMATS_SKIP);
	uint32_t skip_count = TMATS_SKIP;

	// Search for the TMATS EOF.
	uint32_t search_pattern = EOF_TS;
	eof_loc = bb.FindPattern(search_pattern);
	if (eof_loc == UINT32_MAX)
	{
		// Search for the alternate EOF. 
		search_pattern = EOF_ALT;
		eof_loc = bb.FindPattern(search_pattern);
		if (eof_loc == UINT32_MAX)
		{
			// Search for the final EOF type, calculex.
			search_pattern = EOF_CALCULEX;
			eof_loc = bb.FindPattern(search_pattern);
			if (eof_loc == UINT32_MAX)
			{
				// Search for calculex2.
				search_pattern = EOF_CALCULEX2;
				eof_loc = bb.FindPattern(search_pattern);
				if (eof_loc == UINT32_MAX)
				{
					search_pattern = EOF_SEMICOLON_CR_LF_NULL;
					eof_loc = bb.FindPattern(search_pattern);
					if (eof_loc == UINT32_MAX)
					{
#ifdef DEBUG
						if (DEBUG > 0)
							printf("TMATS EOF not found\n");
#endif

						// If TMATS not found, reset eof_loc to 0.
						eof_loc = 0;
						//return UINT32_MAX;

						if (use_default_bus_id_map)
						{
							//create_channel_bus_map();
							return eof_loc;
						}
						else
						{
							printf("TMATS not present and flag use_default_bus_id_map set to false.\n");
							printf("Set flag to true to parse Ch10 using default bus ID map in absence of TMATS data.\n");
							return UINT32_MAX;
						}
					}
				}
			}
		}
	}

	if (eof_loc != 0)
	{
		#ifdef DEBUG
		if (DEBUG > 0)
			printf("TMATS EOF found at %d\n", eof_loc);
		#endif
		tmats_present = true;
	}

	// Parse TMATS content. If lines can't be parsed correctly, return an 
	// error indicator. 
	if (parse_lines(bb))
	{
		tmats_present = false;
		printf("TMATS parse_lines failure. Using default bus ID map.\n");
		if (use_default_bus_id_map)
		{
			return eof_loc;
		}
		else
		{
			printf("TMATS parse lines failure and flag use_default_bus_id_map set to false.\n");
			printf("Set flag to true to parse Ch10 using default bus ID map in absence of TMATS data.\n");
			return UINT32_MAX;
		}
	}

	// Using parsed tmats lines, map the recorder channel ids (integers) 
	// to source names (strings). If this fails, set the tmats_present flags to 
	// false because the data are unusable. 
	if (map_recorder_channel_to_source())
	{
		tmats_present = false;
		printf("TMATS bus ID map failure\n");

		// If tmats is not present and the flag use_default_bus_id_map is false,
		// then exit with a message. If no tmats, and no default (i.e., hard-coded)
		// bus map, then there is no way to intepret messages. Only use a default
		// bus map if tmats is not present and flag is true. IF tmats is present, 
		// use it to create a bus map regardless of default flag state.
		if (use_default_bus_id_map)
		{
			return eof_loc;
		}
		else
		{
			printf("TMATS map failure and flag use_default_bus_id_map set to false.\n");
			printf("Set flag to true to parse Ch10 using default bus ID map in absence of TMATS data.\n");
			return UINT32_MAX;
		}
	}

	// Create the channel_bus_map *from* the channel_source_map.
	// This function must be called only after channel_source_map
	// has been created.
	//create_channel_bus_map();

	// Create the channel_type_map.
	if (map_recorder_channel_to_type())
	{
		channel_type_present = false;
	}
	else
		channel_type_present = true;


	return eof_loc;
}

uint8_t TMATS::parse_lines(BinBuff& bb)
{
	// Find all semicolons, which mark the end of a TMATS entry,
	// and confirm that CR,LF follow each semicolon. If so, parse 
	// the line further into code name and data item, then further 
	// if there are multiple levels indices in the code name.
	uint8_t semicolon_val = char(';');
	bb.SetReadPos(TMATS_SKIP);
	std::vector<uint64_t> semicolon_locs = bb.FindAllPattern(semicolon_val);
	#ifdef DEBUG
	if (DEBUG > 1)
		printf("TMATS Semi-colons found: %zu\n", semicolon_locs.size());
	#endif

	if (semicolon_locs.size() == 0)
	{
		#ifdef DEBUG
		if (DEBUG > 0)
			printf("Semi-colons not found in TMATS data. TMATS data not present\n");
		#endif
		tmats_present = false;
		return 1;
	}

	// Pick out each TMATS line in order and submit for further parsing.
	uint32_t temp_pos = TMATS_SKIP;
	uint32_t line_length = 0;
	char* tmats_line;
	uint8_t cr = 13;
	uint8_t lf = 10;
	uint8_t false_semicolon_count = 0;
	for (auto semicolon_pos : semicolon_locs)
	{
		// Get reference to the bytes in memory pointed to by the semicolon
		// position and confirm that the following characters are carriage return
		// followed by line feed. 
		bb.SetReadPos(temp_pos);
		tmats_line = (char*)bb.Data();
		line_length = semicolon_pos - temp_pos;
		if ((tmats_line[line_length + 1] == cr && tmats_line[line_length + 2] == lf) ||
			(tmats_line[line_length+1] == 0x20))
		{
			std::string s(tmats_line, line_length);
			interpret_tmats_line(s);
		}
		else
		{
			false_semicolon_count++;
#ifdef DEBUG
#if DEBUG > 1
			printf("False semicolon count: %hhu\n", false_semicolon_count);
			std::string s(tmats_line, line_length + uint32_t(10));
			printf("Failed line (line_length+10 chars): %s\n", s.c_str());
#endif
#endif
		}

		// position of semicolon + 2 (CR,LF) + 1 (go to next line) = 3
		temp_pos = semicolon_pos + 3;

		// Occasionally there will be two sets of (CR,LF) characters.
		// Check if the current position is CR and if so skip another
		// two chars.
		if (tmats_line[line_length + 3] == cr)
		{
			temp_pos += 2;
		}
		else if (tmats_line[line_length + 3] == 0x20)
		{
			// And yet another format possibility found in a real flight
			// Calculex-recorded TMATS file: There can be multiple spaces
			// (value 0x20) before (CR,LF). Skip spaces until the (CR,LF)
			// is found.
			uint16_t counter = 4;
			temp_pos++;
			while (tmats_line[line_length + counter] == 0x20)
			{
				counter++;
				temp_pos++;
			}
			if (tmats_line[line_length + counter] == cr && tmats_line[line_length + counter + 1] == lf)
			{
				// Increment temp_pos to the first character past (CR,LF).
				temp_pos += 2;
			}
		}

		// Sometimes semicolons are found throughout the Ch10 file. 
		// If so, then non-TMATS portions of the Ch10 will be treated
		// as TMATS and the program will crash. To avoid this, check 
		// how many times a semicolon is not followed by carriage return,
		// then line feed. Somewhat arbitrarily, break if this occurs
		// some number of times. Maybe I should break if this occurs once.
		// I've never seen a Ch10 file which had a single semicolon that was
		// not followed by cr,lf. 
		if (false_semicolon_count > 1)
			break;
	}
	return 0;
}

void TMATS::interpret_tmats_line(std::string& s)
{
	#ifdef DEBUG
	if (DEBUG > 2)
		printf("\nInterpreting TMATS line: %s\n", s.c_str());
	#endif

	// Split line into code name and data.
	const char colon = ':';
	size_t pos = s.find(colon);
	std::string code;
	std::string data;
	//uint32_t val = 0;
	if (pos != std::string::npos)
	{
		code = s.substr(0, pos);
		data = s.substr(pos+1);
		
		#ifdef DEBUG
		if (DEBUG > 2)
			printf("code: %s, data: %s\n", code.c_str(), data.c_str());
		#endif

		// Attempt to match the code with each of the TMATS items.
		for(size_t i = 0; i < items.size(); i++)
		{
			items[i].match_item(code, data);
		}
	}
}

uint32_t TMATS::eof_location()
{
	return eof_loc;
}

uint32_t TMATS::post_tmats_location()
{
	if(tmats_present)
		return eof_loc + EOF_SIZE;
	else
		return eof_loc;
}

uint8_t TMATS::map_recorder_channel_to_source()
{
	// TMATS must be present.
	if (tmats_present)
	{
		// Determine if appropriate TMATS items are present.
		bool codes_present = true;
		std::string chan_id_code = "R-x\\\\TK1-n";
		std::string source_id_code = "R-x\\\\DSI-n";
		uint16_t chan_id_item_index = 0;
		uint16_t source_id_item_index = 0;

		chan_id_item_index = tmats_item_present(chan_id_code);
		if (chan_id_item_index == UINT16_MAX)
			codes_present = false;

		source_id_item_index = tmats_item_present(source_id_code);
		if (source_id_item_index == UINT16_MAX)
			codes_present = false;

		if(!codes_present)
		{
			#ifdef DEBUG
			if (DEBUG > 2)
				printf("TMATS::map_recorder_to_channel_source(): codes not present\n");
			#endif
			return 1;
		}

		// In the TMATS files I've seen so far, there is only one source
		// recorder ID, value 1. At this time, I will assume there is only
		// one source ID present and check to see if it is 1. That may need
		// need to be amended.

		// For each channel ID (check if it's equal to one), find the subid and the 
		// value to which it's mapped, which I'll label the mappedID. Find the 
		// source subid equal to the channel subid and map: mappedid -> source data.
		std::vector<uint16_t> chan_id_match_inds = items[chan_id_item_index].match_ids(1);
		size_t n_chan_id_match_inds = chan_id_match_inds.size();
		std::vector<uint16_t> source_id_match_inds = items[source_id_item_index].match_ids(1);
		size_t n_source_id_match_inds = source_id_match_inds.size();

		// If there are no matches, then there is no point to continue. 
		if (n_chan_id_match_inds == 0 || n_source_id_match_inds == 0)
		{
			#ifdef DEBUG
			if (DEBUG > 2)
			{
				printf("TMATS::map_recorder_to_channel_source(): zero ids matched\n");
				printf("n_chan_id_match_inds = %zu\n", n_chan_id_match_inds);
				printf("n_source_id_match_inds = %zu\n", n_source_id_match_inds);
			}
			#endif
			return 1;
		}

		// The quantity of subids must be the same because there ought to be a 1:1 matching
		// of chan ID and source value.
		if (n_chan_id_match_inds != n_source_id_match_inds)
		{
			#ifdef DEBUG
			if (DEBUG > 2)
			{
				printf("TMATS::map_recorder_to_channel_source(): match inds quantities unequal\n");
			}
			#endif
			return 1;
		}

		// We know that there is only one subid, because both TMATS items have one "n" and no
		// other variables. 
		uint16_t chan_subid = 0;
		uint16_t source_subid = 0;
		for (size_t match_ind = 0; match_ind < n_chan_id_match_inds; match_ind++)
		{
			// Check if subids match.
			chan_subid = items[chan_id_item_index].subids(chan_id_match_inds[match_ind])[0];
			for (size_t i = 0; i < n_source_id_match_inds; i++)
			{
				source_subid = items[source_id_item_index].subids(source_id_match_inds[i])[0];
				if (chan_subid == source_subid)
				{
					int32_t chanid = 0;
					std::string sourceid = "";
					items[chan_id_item_index].data(chan_id_match_inds[match_ind], chanid);
					items[source_id_item_index].data(source_id_match_inds[i], sourceid);
					
						channel_source_map.insert(std::pair<uint16_t, std::string>(chanid, sourceid));

					#ifdef DEBUG
					if (DEBUG > 2)
						printf("TMATS::map_recorder_to_channel_source(): mapped (%hu, %s)\n", chanid, channel_source_map[chanid].c_str());
					#endif
				}
			}
		}

	}

	return 0;
}

uint8_t TMATS::map_recorder_channel_to_type()
{
	/*
	This function is copied from map_recorder_channel_to_source() and modified slightly.
	*/

	// TMATS must be present.
	if (tmats_present)
	{
		// Determine if appropriate TMATS items are present.
		bool codes_present = true;
		std::string chan_id_code = "R-x\\\\TK1-n";
		std::string type_id_code = "R-x\\\\CDT-n";
		uint16_t chan_id_item_index = 0;
		uint16_t type_id_item_index = 0;

		chan_id_item_index = tmats_item_present(chan_id_code);
		if (chan_id_item_index == UINT16_MAX)
		{
			codes_present = false;
			//printf("chan_id_code not present!\n");
		}

		type_id_item_index = tmats_item_present(type_id_code);
		if (type_id_item_index == UINT16_MAX)
		{
			codes_present = false;
			//printf("type_id_code not present!\n");
		}

		if (!codes_present)
		{
#ifdef DEBUG
			if (DEBUG > 2)
				printf("TMATS::map_recorder_channel_to_type(): codes not present\n");
#endif
			return 1;
		}

		std::vector<uint16_t> chan_id_match_inds = items[chan_id_item_index].match_ids(1);
		size_t n_chan_id_match_inds = chan_id_match_inds.size();
		std::vector<uint16_t> type_id_match_inds = items[type_id_item_index].match_ids(1);
		size_t n_type_id_match_inds = type_id_match_inds.size();

		// If there are no matches, then there is no point to continue. 
		if (n_chan_id_match_inds == 0 || n_type_id_match_inds == 0)
		{
#ifdef DEBUG
			if (DEBUG > 2)
			{
				printf("TMATS::map_recorde_channel_to_type(): zero ids matched\n");
				printf("n_chan_id_match_inds = %zu\n", n_chan_id_match_inds);
				printf("n_type_id_match_inds = %zu\n", n_type_id_match_inds);
			}
#endif
			return 1;
		}

		// The quantity of subids must be the same because there ought to be a 1:1 matching
		// of chan ID and source value.
		if (n_chan_id_match_inds != n_type_id_match_inds)
		{
#ifdef DEBUG
			if (DEBUG > 2)
			{
				printf("TMATS::map_recorder_channel_to_type(): match inds quantities unequal\n");
			}
#endif
			return 1;
		}

		// We know that there is only one subid, because both TMATS items have one "n" and no
		// other variables. 
		uint16_t chan_subid = 0;
		uint16_t type_subid = 0;
		ChannelDataType dt;
		for (size_t match_ind = 0; match_ind < n_chan_id_match_inds; match_ind++)
		{
			// Check if subids match.
			chan_subid = items[chan_id_item_index].subids(chan_id_match_inds[match_ind])[0];
			for (size_t i = 0; i < n_type_id_match_inds; i++)
			{
				type_subid = items[type_id_item_index].subids(type_id_match_inds[i])[0];
				if (chan_subid == type_subid)
				{
					int32_t chanid = 0;
					std::string type_id = "";
					items[chan_id_item_index].data(chan_id_match_inds[match_ind], chanid);
					items[type_id_item_index].data(type_id_match_inds[i], type_id);

					if (type_id == "PCMIN")
						dt = ChannelDataType::PCMIN;
					else if (type_id == "ANAIN")
						dt = ChannelDataType::ANAIN;
					else if (type_id == "DISIN")
						dt = ChannelDataType::DISIN;
					else if (type_id == "TIMEIN")
						dt = ChannelDataType::TIMEIN;
					else if (type_id == "VIDIN")
						dt = ChannelDataType::VIDIN;
					else if (type_id == "UARTIN")
						dt = ChannelDataType::UARTIN;
					else if (type_id == "1553IN")
						dt = ChannelDataType::IN1553;
					else if (type_id == "429IN")
						dt = ChannelDataType::IN429;
					else if (type_id == "MSGIN")
						dt = ChannelDataType::MSGIN;
					else if (type_id == "IMGIN")
						dt = ChannelDataType::IMGIN;
					else if (type_id == "1394IN")
						dt = ChannelDataType::IN1394;
					else if (type_id == "PARIN")
						dt = ChannelDataType::PARIN;
					else if (type_id == "ETHIN")
						dt = ChannelDataType::ETHIN;
					else
						dt = ChannelDataType::NONE;

					channel_type_map.insert(std::pair<uint32_t, ChannelDataType>(chanid, dt));
					channel_type_as_string_map.insert(std::pair<uint32_t, std::string>(chanid, type_id));

#ifdef DEBUG
					if (DEBUG > 1)
						printf("TMATS::map_recorder_channel_to_type(): mapped (%u, %hhu)\n", chanid, static_cast<uint8_t>(channel_type_map[chanid]));
#endif
				}
			}
		}

	}

	return 0;
}

uint16_t TMATS::tmats_item_present(std::string code)
{
	for (uint16_t i = 0; i < items.size(); i++)
	{
		if (items[i].code() == code)
			return i;
	}
	return UINT16_MAX;
}

std::string TMATS::get_mapped_source_from_channel(uint32_t& channel_id)
{
	return channel_source_map[channel_id];
}

void TMATS::print_mapped_source_from_channel()
{
	size_t n_entries = channel_source_map.size();
	std::string bus = "";
	printf("channel_source_map: %zu entries\n", n_entries);
	//std::vector<uint32_t> keys;
	for (auto const& elem : channel_source_map)
	{
		bus = elem.second;
		printf("Key: %06u, Value: %s\n", elem.first, bus.c_str());
	}
} 

std::map<uint32_t, std::string> TMATS::map_channel_to_source_by_type(ChannelDataType cdt)
{
	std::map<uint32_t, std::string> temp_map;
	// channel type data must be present => channel_type_present = true
	if (tmats_present && channel_type_present)
	{
		// Iterate over channel_type_map and look for entries that correspond
		// to the type in the query.
		std::map<uint32_t, ChannelDataType>::iterator it;
		for (it = channel_type_map.begin(); it != channel_type_map.end(); ++it)
		{
			if (it->second == cdt)
			{
				// Map the corresponding channel id to the channel_source_map mapped
				// value.
				temp_map.insert(std::pair<uint32_t, std::string>(it->first, channel_source_map[it->first]));
			}
		}
	}
	return temp_map;
}

void TMATS::replace_channel_source_map(const std::map<uint32_t, std::string>& input_map)
{
	channel_source_map = input_map;
	print_mapped_source_from_channel();
}

bool TMATS::using_tmats_data()
{
	return tmats_present;
}

std::map<uint32_t, std::string> TMATS::get_channel_source_map()
{
	return channel_source_map;
}

std::map<uint32_t, ChannelDataType> TMATS::get_channel_type_map()
{
	return channel_type_map;
}

std::map<uint32_t, std::string> TMATS::get_channel_type_as_string_map()
{
	return channel_type_as_string_map;
}
