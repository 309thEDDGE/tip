// parse_worker.cpp

#include "parse_worker.h"

ParseWorker::~ParseWorker()
{
	if (delete_alloc)
	{
		delete pkthdr;
		delete tdf;
		delete milstd;
#ifdef VIDEO_DATA
		delete video;
#endif
		#ifdef DEBUG
		if (DEBUG > 2)
			printf("ParseWorker: after delete\n");
		#endif
	}
}

ParseWorker::ParseWorker() : start_position(0), 
	last_position(0), id(UINT16_MAX), complete(false),
	bb_ind(UINT16_MAX), output_fname(""), first_TDP_loc(UINT32_MAX),
	pkt_count(0),
	tdata(), packet_ledger(), packet_error_ledger(),
	tdf(nullptr), milstd(nullptr), delete_alloc(false), continue_parsing(false),
	first_tdp(false), pkthdr(nullptr), read_size(0), retcode(0), use_comet_comm_wrd(false),
	have_generated_file_names(false), final_worker(false), is_scan_worker(false)
{
#ifdef VIDEO_DATA
	video = nullptr;
#endif
}

#ifdef PARQUET
void ParseWorker::initialize(uint16_t ID,
	uint64_t start_pos, uint32_t read, uint16_t binbuff_ind,
	std::map<Ch10DataType, std::filesystem::path>& fsmap,
	TMATS& tmatsdata, bool use_comet_command_words, bool is_final_worker)
{
	id = ID;
	start_position = start_pos;
	read_size = read;
	bb_ind = binbuff_ind;
	tdata = tmatsdata;
	use_comet_comm_wrd = use_comet_command_words;
	final_worker = is_final_worker;
	generate_parquet_file_names(fsmap);
}

void ParseWorker::get_msg_names(std::set<std::string>& output_name_set)
{
	milstd->get_msg_names(output_name_set);
}
#endif

void ParseWorker::append_mode_initialize(uint32_t read, uint16_t binbuff_ind,
	uint64_t start_pos)
{
	read_size = read;
	bb_ind = binbuff_ind;
	start_position = start_pos;
}

void ParseWorker::operator()(BinBuff& bb, bool append_mode, bool check_milstd1553_word_count, bool milstd1553_msg_selection,
	std::vector<std::string> milstd1553_sorted_selected_msgs)
{
#ifdef DEBUG
	if (DEBUG > 0)
	{
		if (append_mode)
		{
			printf("\n(%03u) APPEND MODE ParseWorker now active!\n", id);
		}
		else
		{
			printf("\n(%03u) ParseWorker now active!\n", id);
		}

	}
	if (DEBUG > 1)
		printf("(%03u) Absolute position: %llu\n", id, start_position);
#endif

	first_tdp = true;
	continue_parsing = true;
	uint16_t n_milstd_messages = 0;
	retcode = 0;
	pkt_count = 0;

	// Create objects for parsing Ch10 data packets. 
	const Ch10PacketHeaderFormat* pkthdr_data;
	if (append_mode)
	{
		pkthdr->SetBinbuff(&bb);
		tdf->SetBinbuff(&bb);
		milstd->SetBinbuff(&bb);
		milstd->set_truncate(false);
#ifdef VIDEO_DATA
		video->SetBinbuff(&bb);
		video->set_truncate(false);
#endif
		
	}
	else
	{
		pkthdr = new Ch10PacketHeader(bb, id);
		tdf = new Ch10TDF1(bb, id);
		milstd = new Ch10MilStd1553F1(bb, id, tdata, check_milstd1553_word_count,
			use_comet_comm_wrd, output_file_names[Ch10DataType::MILSTD1553_DATA_F1],
			milstd1553_msg_selection, milstd1553_sorted_selected_msgs);
		milstd->set_channelid_remoteaddress_output(&chanid_remoteaddr1_map, &chanid_remoteaddr2_map);
#ifdef VIDEO_DATA
		printf("\n(%03u) ParseWorker parsing VIDEO\n", id);
		video = new Ch10VideoDataF0(bb, id, tdata, output_file_names[Ch10DataType::VIDEO_DATA_F0]);
#endif
		delete_alloc = true;
	}

	// Find time data packet and collect locations of sync bytes.
	// Note that in append_mode, this function only finds the sync 
	// byte locations and returns when the first error free packet
	// header is found, unlike the case when NOT in append_mode, when
	// the function only returns a non UINT32_MAX when a time data
	// packet is found. 
	first_TDP_loc = pkthdr->find_first_time_data_packet(append_mode);

	// Continue with parsing only if a time data packet is found. 
	if (first_TDP_loc < UINT32_MAX)
	{
		// Iterate over packets, switch on packet type and parse. 
		while (continue_parsing)
		{
			pkt_count++;
			/*if(pkt_count > 3)
				break;*/
				// Assume binary buffer index is at the first byte of the next
				// Ch10 packet body and the packet header has been parsed by 
				// the Ch10PacketHeader object. 

				// Get the pointer to the packet header data.
			pkthdr_data = pkthdr->Data();
			// Parse the packet body. 
			switch (pkthdr_data->data_type)
			{
				case static_cast<uint8_t>(Ch10DataType::TIME_DATA_F1) :

					// If in append mode, the first occurrence of a time
					// data packet indicates the end of parsing because this
					// time data packet would have been picked up as the first
					// TDP of the next worker's parse run. Set status as necessary
					// and return.

					/*
					Note: It may be better to be aware of which time data packet has
					been found, for non-append mode workers. The first time packet
					cannot have errors otherwise the worker should return. I don't know
					how to handle the case of errors in the first time packet encountered.
					If no errors, all subsequent packets can have errors in the case that
					the relative time counter doesn't roll over. This all needs to be
					handled carefully and none of it is implemented at this point.
					*/
					if (append_mode)
					{
#ifdef DEBUG
#if DEBUG > 1
						printf("(%03u) Found time data packet in append mode, returning.\n", id);
#endif
#endif
						retcode = 0;
						last_position = start_position + pkthdr->start_position();
						continue_parsing = false;
						continue;
					}
					else
					{
						tdf->Initialize(nullptr, pkthdr->GetCh10HeaderDataPtr());
						retcode = tdf->Parse();
						if (retcode)
						{
							packet_error_ledger.time_data_count++;
#ifdef DEBUG
							if (DEBUG > -1)
								printf("\n(%03u) Error at time data packet, returning.\n", id);
#endif
							if (tdf->Status() == TDF1Status::PARSE_FAIL)
							{
								continue_parsing = false;
								last_position = start_position + pkthdr->start_position();
								continue;
							}
						}
						else
						{
							packet_ledger.time_data_count++;
							if (first_tdp)
							{
#ifdef DEBUG
								if (DEBUG > 2)
								{
									printf("(%03u) First time data packet.\n", id);
									printf("Absolute position: %llu\n", start_position + pkthdr->start_position());
								}
#endif
								first_tdp = false;
							}
						}
					}
				break;

				case static_cast<uint8_t>(Ch10DataType::MILSTD1553_DATA_F1) :
				{
					// Insert the new channel ID element if it's not 
					// already present. 
					if (chanid_remoteaddr1_map.count(pkthdr_data->chanID) == 0)
					{
						std::set<uint16_t> temp_set;
						chanid_remoteaddr1_map[pkthdr_data->chanID] = temp_set;
						chanid_remoteaddr2_map[pkthdr_data->chanID] = temp_set;
					}

					// This is the new way to initialize a ch10 packet parser that inherits
					// from Parsecontext.
					milstd->Initialize(tdf->GetCh10TimeDataPtr(), pkthdr->GetCh10HeaderDataPtr());

					retcode = milstd->Parse();
					if (retcode)
					{
						packet_error_ledger.milstd_1553pkt_count++;
					}
					else
						packet_ledger.milstd_1553pkt_count++;

					break;
				}
#ifdef VIDEO_DATA
				case static_cast<uint8_t>(Ch10DataType::VIDEO_DATA_F0) :
				{
					/*video->initialize(tdf->absolute_time, tdf->relative_time_counter, tdf->day_of_year,
						pkthdr->relative_time_counter(),
						pkthdr_data->intrapkt_ts_source, pkthdr_data->time_format, pkthdr_data->chanID,
						pkthdr_data->data_size);*/
					video->Initialize(tdf->GetCh10TimeDataPtr(), pkthdr->GetCh10HeaderDataPtr());
					retcode = video->Parse();
					if (retcode)
						packet_error_ledger.video_data_count++;
					else
						packet_ledger.video_data_count++;
					break;
				}
#endif
#ifdef DISCRETE_DATA
				case static_cast<uint8_t>(Ch10DataType::DISCRETE_DATA_F1) :
#ifdef DEBUG
#if DEBUG > 3
					printf("\n(%03u) -- Discrete Data, Format 1 UNDEFINED --\n", id);
#endif
#endif
					break;
#endif
#ifdef ANALOG_DATA
				case static_cast<uint8_t>(Ch10DataType::ANALOG_DATA_F1) :
#ifdef DEBUG
#if DEBUG > 3
					printf("\n(%03u) -- Analog Data, Format 1 UNDEFINED --\n", id);
#endif
#endif
					break;
#endif
				default:
#ifdef DEBUG
#if (DEBUG > 3)
					printf("\n(%03u) Data type 0x%x unknown - skipping\n", id, pkthdr_data->data_type);
#endif
#endif
					break;
			}

			// Move buffer data location to the beginning of the next packet.
			// Attempt to parse it. If there is a failure, make a record of the error and 
			// move the read location to the next found sync location.
			retcode = pkthdr->advance_to_next();
			if (retcode)
			{
				// Failed to move binary buffer position. 
#ifdef DEBUG
				if (DEBUG > 0)
					printf("(%03u) Failed to move binary buffer position to (assumed) position of next packet\n", id);
#endif
				retcode = 0;
				last_position = start_position + pkthdr->start_position();
				continue_parsing = false;
				continue;
			}

			parse_and_validate_header();

		} // end while(continue_parsing)
	} // end if(first_TDP_loc < UINT32_MAX)

	complete = true;
#ifdef DEBUG
#if DEBUG > 0
	printf("(%03u) End of worker's shift\n", id);
#endif
#if DEBUG > 1
	printf("(%03u) Absolute position: %llu\n\n", id, last_position);
#endif
#endif

	// Close/commit to database.
#ifdef LOCALDB
#ifdef PARQUET
	// For Parquet writing, values from each parsed message or packet are appended 
	// to arrays that are written to Parquet file only when they are full or reach
	// a certain value (see, e.g., max_temp_element_count_ in ParquetMilStd1553F1 in
	// which max_temp_element_count_ is both the array size and the pre-determined 
	// threshold at which the arrays are written to Parquet file, comprising a row group).
	// After dangling packets are appended in append mode, there will likely be some
	// quantity of rows in the vectors that have been filled but not written as a row group
	// because the quantity of filled rows hasn't reached the pre-determined value
	// (max_temp_element_count_ in the case of 1553 data in ParquetMilStd1553F1 class). 
	// The same is true of the last worker which reaches the end of the Ch10 file and 
	// will likely not have reached the threshold, so the data need to be committed.
	// In both cases, the following causes the remaining rows which have not been 
	// written as a row group to be written. 
	if (append_mode || final_worker)
	{
		milstd->close();
#ifdef VIDEO_DATA
		printf("(%03hu) Closing Video Data Parquet database\n", id);
		video->close();
#endif 
	}
#endif
#endif
}

void ParseWorker::parse_and_validate_header()
{
	retcode = pkthdr->Parse();
	while (retcode)
	{
		#ifdef DEBUG
		if (DEBUG > 1)
			printf("(%03u) Pkt Hdr Err: %s, pkt %u\n", id, pkthdr->status_desc().c_str(), pkt_count);
		#endif

		// If the header or body exceeds the remaining bytes in the binary buffer,
		// end parsing. Otherwise move the packet header 
		if (pkthdr->Status() == Ch10PacketHeaderStatus::HEADER_EXCEEDS ||
			pkthdr->Status() == Ch10PacketHeaderStatus::BODY_EXCEEDS)
		{
			last_position = start_position + pkthdr->Position();
			continue_parsing = false;
			retcode = 0;
		}
		else
		{
			hdr_err.push_back(pkthdr->Status());
			retcode = pkthdr->advance_to_next_sync_location();
			if (retcode)
			{
				#ifdef DEBUG
				if (DEBUG > 0)
					printf("(%03u) Failed to move binary buffer position to next sync location\n", id);
				#endif
				retcode = 0;
				last_position = start_position + pkthdr->Position();
				continue_parsing = false;
			}
			else
				retcode = pkthdr->Parse();
		}
	}
}

std::atomic<bool>& ParseWorker::completion_status()
{
	return complete;
}

void ParseWorker::reset_completion_status()
{
	complete = false;
}

uint16_t ParseWorker::get_binbuff_ind()
{ return bb_ind; }

void ParseWorker::time_info(uint64_t&& abstime, uint64_t&& rtc)
{
	abstime = tdf->GetCh10TimeDataPtr()->timedatapkt_abstime_;
	rtc = tdf->GetCh10TimeDataPtr()->timedatapkt_rtc_;
}

uint64_t& ParseWorker::get_last_position()
{
	return last_position;
}

PacketStats* ParseWorker::get_packet_ledger()
{
	return &(packet_ledger);
}

PacketStats* ParseWorker::get_packet_error_ledger()
{
	return &(packet_error_ledger);
}

Ch10MilStd1553F1Stats* ParseWorker::milstd1553_stats()
{
	return &(milstd->get_stats());
}

std::string ParseWorker::output_file_path()
{
	return output_fname;
}

std::string ParseWorker::output_file_path(Ch10DataType dt)
{
	return output_file_names[dt];
}

#ifdef PARQUET
void ParseWorker::generate_parquet_file_names(std::map<Ch10DataType, std::filesystem::path>& fsmap)
{
	if (!have_generated_file_names)
	{
		have_generated_file_names = true;
		std::filesystem::path dirpath = fsmap[Ch10DataType::MILSTD1553_DATA_F1];
		std::filesystem::path outpath = dirpath / dirpath.stem();
		char buff[20];
		sprintf(buff, "__%03u.parquet", id);
		std::string ext(buff);
		outpath += std::filesystem::path(ext);
		output_file_names[Ch10DataType::MILSTD1553_DATA_F1] = outpath.string();
		printf("Worker output path: %s\n", output_file_names[Ch10DataType::MILSTD1553_DATA_F1].c_str());

#ifdef VIDEO_DATA
		dirpath = fsmap[Ch10DataType::VIDEO_DATA_F0];
		outpath = dirpath / dirpath.stem();
		outpath += std::filesystem::path(ext);
		output_file_names[Ch10DataType::VIDEO_DATA_F0] = outpath.string();
#endif

	}
}
#endif

void ParseWorker::append_chanid_remoteaddr_maps(std::map<uint32_t, std::set<uint16_t>>& out1,
	std::map<uint32_t, std::set<uint16_t>>&out2)
{
	for (std::map<uint32_t, std::set<uint16_t>>::iterator it = chanid_remoteaddr1_map.begin(); it != chanid_remoteaddr1_map.end(); ++it)
	{
		// If the key is not present in the output map, insert the entire set. 
		if (out1.count(it->first) == 0)
		{
			out1[it->first] = it->second;
		}
		// Otherwise attempt to insert each of the set elements.
		else
		{
			out1[it->first].insert(it->second.begin(), it->second.end());
		}
		/*printf("Remote addresses1 found in packets with channel ID %02u:\n", it->first);
		for (std::set<uint16_t>::iterator it2 = out1[it->first].begin(); it2 != out1[it->first].end(); ++it2)
		{
			printf("%02hu ", *it2);
		}
		printf("\n");*/
	}

	for (std::map<uint32_t, std::set<uint16_t>>::iterator it = chanid_remoteaddr2_map.begin(); it != chanid_remoteaddr2_map.end(); ++it)
	{
		// If the key is not present in the output map, insert the entire set. 
		if (out2.count(it->first) == 0)
		{
			out2[it->first] = it->second;
		}
		// Otherwise attempt to insert each of the set elements.
		else
		{
			out2[it->first].insert(it->second.begin(), it->second.end());
		}
		/*printf("Remote addresses2 found in packets with channel ID %02u:\n", it->first);
		for (std::set<uint16_t>::iterator it2 = out2[it->first].begin(); it2 != out2[it->first].end(); ++it2)
		{
			printf("%02hu ", *it2);
		}
		printf("\n");*/
	}
}