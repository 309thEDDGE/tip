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
	bb_ind(UINT16_MAX), first_TDP_loc(UINT32_MAX),
	pkt_count(0),
	packet_ledger(), packet_error_ledger(),
	tdf(nullptr), milstd(nullptr), delete_alloc(false), continue_parsing(false),
	first_tdp(false), pkthdr(nullptr), read_size(0), retcode(0),
	final_worker(false), is_scan_worker(false)
#ifdef LIBIRIG106
	, i106_handle_(0), i106_status_(I106Status::I106_OK), i106_offset_(0), found_tmats_(false),
	temp_buffer_vec_(temp_buffer_size_)
#endif
{
#ifdef VIDEO_DATA
	video = nullptr;
#endif
}

#ifdef PARQUET
void ParseWorker::initialize(uint16_t ID,
	uint64_t start_pos, uint32_t read, uint16_t binbuff_ind,
	std::map<Ch10DataType, ManagedPath>& file_path_map,
	bool is_final_worker)
{
	id = ID;
	start_position = start_pos;
	read_size = read;
	bb_ind = binbuff_ind;
	final_worker = is_final_worker;
	output_file_paths_ = file_path_map;
#ifdef LIBIRIG106
	if (ID == 0)
		found_tmats_ = false;
	else
		found_tmats_ = true;

#endif
}
#endif

void ParseWorker::append_mode_initialize(uint32_t read, uint16_t binbuff_ind,
	uint64_t start_pos)
{
	read_size = read;
	bb_ind = binbuff_ind;
	start_position = start_pos;
#ifdef LIBIRIG106
	found_tmats_ = true;
#endif
}

#ifdef PARSER_REWRITE
void ParseWorker::operator()(BinBuff& bb, bool append_mode, 
	std::vector<std::string>& tmats_body_vec, std::map<Ch10PacketType, bool> ch10_packet_type_map)
{
	if (append_mode)
		SPDLOG_INFO("({:02d}) APPEND MODE ParseWorker now active", id);
	else
		SPDLOG_INFO("({:02d}) ParseWorker now active", id);
	
	SPDLOG_DEBUG("({:02d}) Beginning of shift, absolute position: {:d}", id, start_position);

	// Initialize Ch10Context object. Note that this Ch10Context instance
	// created in the ParseWorker constructor is persistent until the 
	// ParseWorker instance is garbage-collected to maintain file writer
	// (read: Parquet file writer) state.
	ctx.Initialize(start_position, id);
	ctx.SetSearchingForTDP(!append_mode);

	if (!ctx.IsConfigured())
	{
		// Configure packet parsing.
		ctx.SetPacketTypeConfig(ch10_packet_type_map);

		// Configure output file paths.
		std::map<Ch10PacketType, ManagedPath> output_paths = {
			{Ch10PacketType::MILSTD1553_F1, output_file_paths_[Ch10DataType::MILSTD1553_DATA_F1]},
			{Ch10PacketType::VIDEO_DATA_F0, output_file_paths_[Ch10DataType::VIDEO_DATA_F0]}
		};

		// Check configuration. Are the packet parse and output paths configs
		// consistent?
		std::map<Ch10PacketType, ManagedPath> enabled_paths;
		bool config_ok = ctx.CheckConfiguration(ctx.pkt_type_config_map,
			output_paths, enabled_paths);
		if (!config_ok)
		{
			complete = true;
			return;
		}

		// For each packet type that is enabled and has an output path specified,
		// create a file writer object that is owned by Ch10Context to maintain
		// state between regular and append mode calls to this worker's operator().
		// Pass a pointer to the file writer to the relevant parser for use in 
		// writing data to disk.
		ctx.InitializeFileWriters(enabled_paths);
	}

	// Instantiate Ch10Packet object
	Ch10Packet packet(&bb, &ctx, tmats_body_vec);

	// Parse packets until error or end of buffer.
	bool continue_parsing = true;
	Ch10Status status;
	while (continue_parsing)
	{
		status = packet.ParseHeader();
		if (status == Ch10Status::BAD_SYNC || status == Ch10Status::PKT_TYPE_NO)
		{
			continue;
		}
		else if (status == Ch10Status::PKT_TYPE_EXIT || status == Ch10Status::BUFFER_LIMITED)
		{
			continue_parsing = false;
			continue;
		}

		// Parse body if the header is parsed and validated.
		packet.ParseBody();
	}

	// Update last_position;
	last_position = ctx.absolute_position;

	// Close all file writers if append_mode is true or
	// this is the final worker which has no append mode.
	if(append_mode || final_worker)
		ctx.CloseFileWriters();
	
	SPDLOG_INFO("({:02d}) End of worker's shift", id);
	SPDLOG_DEBUG("({:02d}) End of shift, absolute position: {:d}", id, last_position);
	complete = true;
}
#endif

#ifndef PARSER_REWRITE
#ifndef LIBIRIG106
void ParseWorker::operator()(BinBuff& bb, bool append_mode)
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
		milstd = new Ch10MilStd1553F1(bb, id, output_file_paths_[Ch10DataType::MILSTD1553_DATA_F1]);
		milstd->set_channelid_remoteaddress_output(&chanid_remoteaddr1_map, &chanid_remoteaddr2_map);
#ifdef VIDEO_DATA
		printf("\n(%03u) ParseWorker parsing VIDEO\n", id);
		video = new Ch10VideoDataF0(bb, id, output_file_paths_[Ch10DataType::VIDEO_DATA_F0]);
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
#ifdef DEBUG
#if DEBUG > 2
		printf("(%03hu) Closing 1553 data Parquet database\n", id);
#endif
#endif
		milstd->close();
#ifdef VIDEO_DATA
		printf("(%03hu) Closing Video Data Parquet database\n", id);
		video->close();
#endif 
	}
#endif
#endif
}
#endif
#endif

#ifdef LIBIRIG106
void ParseWorker::operator()(BinBuff& bb, bool append_mode, std::vector<std::string>& tmats_body_vec)
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

	first_tdp = false;
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
		milstd = new Ch10MilStd1553F1(bb, id, output_file_paths_[Ch10DataType::MILSTD1553_DATA_F1]);
		milstd->set_channelid_remoteaddress_output(&chanid_remoteaddr1_map, &chanid_remoteaddr2_map);
		milstd->set_channelid_commwords_output(&chanid_commwords_map);
#ifdef VIDEO_DATA
		printf("\n(%03u) ParseWorker parsing video packets\n", id);
		video = new Ch10VideoDataF0(bb, id, output_file_paths_[Ch10DataType::VIDEO_DATA_F0]);
#endif
#ifdef ETHERNET_DATA
		printf("\n(%03hu) ParseWorker parsing Ethernet packets\n", id);
		i106_ethernetf0_.Initialize(id, &ch10md_, 
			output_file_paths_[Ch10DataType::ETHERNET_DATA_F0]);
		if (!i106_ethernetf0_.InitializeWriter())
		{
			printf("\n(%03hu) ParseWorker failed to initialize Ethernet writer\n", id);
			complete = true;
			return;
		}
#endif
		delete_alloc = true;
	}
	
	i106_status_ = I106C10OpenBuffer(&i106_handle_, (void*)bb.Data(), (int)bb.Size(), I106C10Mode::READ);
	if (i106_status_ != I106Status::I106_OK)
	{
		printf("\n(%03u) I106C10OpenBuffer failure: %s\n", 
			id, I106ErrorString(i106_status_));
		complete = true;
		return;
	}

	// Iterate over packets, switch on packet type and parse. 
	void* temp_buffer_ptr = (void*)temp_buffer_vec_.data();
	while (continue_parsing)
	{
		//printf("before read next header\n");
		i106_status_ = I106C10ReadNextHeader(i106_handle_, &i106_header_);
		//printf("immed after read next header\n");
		if (i106_status_ != I106Status::I106_OK)
		{
			if (i106_status_ == I106Status::I106_EOF)
			{
				printf("\n(%03u) I106C10ReadNextHeader: %s\n", id,
					I106ErrorString(i106_status_));
				retcode = 0;

				// Get the position/offset of the I106 internal reader to
				// calculate and record the position from which the 
				// append mode worker should start parsing.
				i106_status_ = I106C10GetPos(i106_handle_, &i106_offset_);
				if (i106_status_ != I106Status::I106_OK)
				{
					printf("\n(%03u) I106C10GetPos Failed at last_position calculation!: %s\n",
						id, I106ErrorString(i106_status_));
					continue_parsing = false;

					// If GetPos fails at this point then last_position can't be correctly set.
					// I believe this constitutes a major error in which append mode
					// for this worker is not started or, if started, checks to see if the 
					// retcode is 1 and immediately exits. This is not implemented.
					last_position = start_position;
					retcode = 1;
					continue;
				}

				// During normal functionality, I106C10ReadNextHeader leaves the internal
				// position at the end of the Ch10 packet header. If this is the case even
				// when EOF status is returned then the calculation ought to be:
				last_position = start_position + i106_offset_ - GetHeaderLength(&i106_header_);
				continue_parsing = false;
				continue;
			}
			else
			{
				printf("\n(%03u) I106C10ReadNextHeader: %s\n",
					id, I106ErrorString(i106_status_));
			}
		}

		// Get the position/offset of the I106 internal reader to
		// calculate and record the position from which the 
		// append mode worker should start parsing.
		i106_status_ = I106C10GetPos(i106_handle_, &i106_offset_);
		if (i106_status_ != I106Status::I106_OK)
		{
			printf("\n(%03u) I106C10GetPos Failed at last_position calculation!: %s\n",
				id, I106ErrorString(i106_status_));
			continue_parsing = false;

			// If GetPos fails at this point then last_position can't be correctly set.
			// I believe this constitutes a major error in which append mode
			// for this worker is not started or, if started, checks to see if the 
			// retcode is 1 and immediately exits. This is not implemented.
			last_position = start_position;
			retcode = 1;
			continue;
		}

		// During normal functionality, I106C10ReadNextHeader leaves the internal
		// position at the end of the Ch10 packet header. If this is the case even
		// when EOF status is returned then the calculation ought to be:
		last_position = start_position + i106_offset_ - GetHeaderLength(&i106_header_);


		//printf("after read next header, type = %hhu\n", i106_header_.DataType);
		// In append mode, exit as soon as the first TDP is found. The assumption is 
		// that the previous
		// iteration of a worker in the part of the Ch10 file in which the buffer has been
		// set already parsed all packets starting from the TDP and continuing to end
		// of the buffer. 
		if (append_mode)
		{
			if (i106_header_.DataType == I106CH10_DTYPE_IRIG_TIME)
			{
				printf("\n(%03u) Append mode and found first TDP!\n", id);
				first_tdp = true;
				continue_parsing = false;
				continue;
			}
		}
		else if(!first_tdp)
		{
			if (!found_tmats_)
			{
				if (i106_header_.DataType == I106CH10_DTYPE_TMATS)
					found_tmats_ = true;
			}
			else if (i106_header_.DataType == I106CH10_DTYPE_IRIG_TIME)
			{
				first_tdp = true;
				printf("\n(%03u) Found first TDP!\n", id);
			}
			else
				continue;
		}

		// No need to get the header length because ReadNextHeader sets
		// its internal pointer to the end of the ch10 packet header. Now 
		// a call to GetPos	will return the beginning of the packet body,
		// which can be used to set the position of the BinBuff buffer.

		// In the future when the packet body will also be parsed by LibIRIG106
		// I believe there is no need to use I106C10ReadData to read data into
		// a buffer, instead I can pass the BinBuff::Data() return directly into
		// I106_Decode_First1553F1, for example, or any other packet parser
		// based on which packet is indicated by informatin in the ch10 packet header.
		// Ex: I106_Decode_First1553F1(&i106_header_, (char*)bb->Data(), &msg). The 
		// important point here is that we do bb.SetReadPos(i106_offset_) first, where
		// the offset is the current value of the return of GetPos immediately after having
		// used ReadNextHeader.
		i106_status_ = I106C10ReadData(i106_handle_, GetDataLength(&i106_header_),
			temp_buffer_ptr);
		if (i106_status_ != I106Status::I106_OK)
		{
			// If the error prevents further packet parsing, exit. Note that the
			// absolute position of the beginning of this packet was already recorded
			// in last_position so this value does not need to be calculated.
			// Currently the assumption is that any error indicates that further parsing
			// is not possible. 
			printf("\n(%03u) I106C10ReadData: %s\n", id, I106ErrorString(i106_status_));
			continue_parsing = false;
			continue;
		}

		// Do time calculations and set the BinBuff to the beginning of the data payload.
		pkthdr->UseLibIRIG106PktHdrStruct(&i106_header_, i106_offset_, GetHeaderLength(&i106_header_));
		if (bb.SetReadPos(uint64_t(i106_offset_)) == 1)
		{
			printf("\n(%03u) BinBuff::SetReadPos failure: requested offset = %lld\n",
				id, i106_offset_);
			continue_parsing = false;
			continue;
		}
		pkthdr_data = pkthdr->Data();
		pkt_count++;

		// Parse the packet body. 
		switch (i106_header_.DataType)
		{
			case I106CH10_DTYPE_TMATS:
			{
				/*i106_status_ = I106_Decode_TMATS(&i106_header_, (void*)temp_buffer_vec_.data(),
					&i106_tmats_info_);*/
				char* tmats_cstring;
				int tmats_length = 0;
				I106GetRawTMATS(&i106_header_, temp_buffer_ptr, &tmats_cstring,
					&tmats_length);
				std::string tmats_str(tmats_cstring, tmats_length);
				tmats_body_vec.push_back(tmats_str);
				break;
			}

			case I106CH10_DTYPE_IRIG_TIME:
			{
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
					continue_parsing = false;
					continue;
				}
				else
				{
					// Parse the time data packet, format 1 using LibIRIG106.
					i106_status_ = I106_Decode_TimeF1(&i106_header_, temp_buffer_ptr, &i106_time_);
					if (i106_status_ != I106Status::I106_OK)
					{
						printf("\n(%03u) I106_Decode_TimeF1: %s\n", id, I106ErrorString(i106_status_));
						continue_parsing = false;
						continue;
					}

					// Initialize the time data packet parser object and 
					// pass in the pre-parsed data necessary to calculate the absolute time.
					tdf->Initialize(nullptr, pkthdr->GetCh10HeaderDataPtr());
					tdf->UseI106Time(&i106_time_);

					//retcode = tdf->Parse();
					/*if (retcode)
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
						}
					}*/
				}
				break;
			} // end IRIG time parsing (TDF1)

			case I106CH10_DTYPE_1553_FMT_1:
			{
				// Insert the new channel ID element if it's not 
				// already present. 
				if (chanid_remoteaddr1_map.count(i106_header_.ChannelID) == 0)
				{
					std::set<uint16_t> temp_set;
					chanid_remoteaddr1_map[i106_header_.ChannelID] = temp_set;
					chanid_remoteaddr2_map[i106_header_.ChannelID] = temp_set;

					std::set<uint32_t> temp_set2;
					chanid_commwords_map[i106_header_.ChannelID] = temp_set2;
				}

				// This is the new way to initialize a ch10 packet parser that inherits
				// from Parsecontext.
				milstd->Initialize(tdf->GetCh10TimeDataPtr(), pkthdr->GetCh10HeaderDataPtr());

				// Parse the 1553 Ch10 packet and the 1553 messages using LibIRIG106.
				retcode = milstd->UseLibIRIG106(&i106_header_, temp_buffer_ptr);
				if (retcode == 1)
				{
					continue_parsing = false;
					continue;
				}

				/*if (retcode)
				{
					packet_error_ledger.milstd_1553pkt_count++;
				}
				else
					packet_ledger.milstd_1553pkt_count++;*/

				break;
			}
#ifdef ETHERNET_DATA
			case I106CH10_DTYPE_ETHERNET_FMT_0:
			{
				// The following lines are a stop-gap during the transition to a cleaner
				// LibIRIG106 integration. Here I record the data stored in Ch10TimeData and
				// Ch10HeaderData in the Ch10MetaData object in preparation for passing it 
				// to the I106Ch10EthernetF0::Ingest() method.
				const Ch10TimeData* ch10td = tdf->GetCh10TimeDataPtr();
				const Ch10HeaderData* ch10hd = pkthdr->GetCh10HeaderDataPtr();
				ch10md_.timedatapkt_rtc_ = ch10td->timedatapkt_rtc_;
				ch10md_.doy_ = ch10td->doy_;
				ch10md_.timedatapkt_abstime_ = ch10td->timedatapkt_abstime_;
				ch10md_.intrapkt_ts_source_ = ch10hd->intrapkt_ts_source_;
				ch10md_.time_format_ = ch10hd->time_format_;
				ch10md_.header_rtc_ = ch10hd->header_rtc_;

				if (i106_ethernetf0_.Ingest(&i106_header_, temp_buffer_ptr) == 1)
				{
					printf("\n(%03hu) ParseWorker EthernetF0 Ingest failed!\n", id);
					continue_parsing = false;
					continue;
				}
				break;
			}
#endif
#ifdef VIDEO_DATA
			case I106CH10_DTYPE_VIDEO_FMT_0:
			{
				video->Initialize(tdf->GetCh10TimeDataPtr(), pkthdr->GetCh10HeaderDataPtr());
				//retcode = video->Parse();
				retcode = video->UseLibIRIG106(&i106_header_, temp_buffer_ptr);
				if (retcode == 1)
				{
					continue_parsing = false;
					continue;
				}

				/*if (retcode)
					packet_error_ledger.video_data_count++;
				else
					packet_ledger.video_data_count++;
				break;*/
			}
#endif
#ifdef DISCRETE_DATA
			case I106CH10_DTYPE_DISCRETE:
			{
#ifdef DEBUG
#if DEBUG > 3
				printf("\n(%03u) -- Discrete Data, Format 1 UNDEFINED --\n", id);
#endif
#endif
				break;
			}
#endif
#ifdef ANALOG_DATA
			case I106CH10_DTYPE_ANALOG:
			{
#ifdef DEBUG
#if DEBUG > 3
				printf("\n(%03u) -- Analog Data, Format 1 UNDEFINED --\n", id);
#endif
#endif
				break;
			}
#endif
			default:
			{
#ifdef DEBUG
#if (DEBUG > 3)
				printf("\n(%03u) Data type 0x%x unknown - skipping\n", id, i106_header_.DataType);
#endif
#endif
				break;
			}
		} // end switch (i106_header_.DataType)

	} // end while(continue_parsing)
	
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
#ifdef DEBUG
#if DEBUG > 2
		printf("(%03hu) Closing 1553 data Parquet database\n", id);
#endif
#endif
		milstd->close();
#ifdef VIDEO_DATA
		printf("(%03hu) Closing Video Data Parquet database\n", id);
		video->close();
#endif 
#ifdef ETHERNET_DATA
		printf("(%03hu) Closing Ethernet Data Parquet database\n", id);
		i106_ethernetf0_.Finalize();
#endif

		
	}
#endif
#endif

	complete = true;
	// Close I106 Buffer
	i106_status_ = I106C10Close(i106_handle_);
	if (i106_status_ != I106Status::I106_OK)
	{
		printf("\n(%03u) I106C10Close failure: %s\n",
			id, I106ErrorString(i106_status_));
	}	
	else
	{
		printf("\nDeleting Handle: %i\n",i106_handle_);
	}
}
#endif

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

void ParseWorker::append_chanid_remoteaddr_maps(std::map<uint32_t, std::set<uint16_t>>& out1,
	std::map<uint32_t, std::set<uint16_t>>&out2)
{
	IterableTools it;
#ifdef PARSER_REWRITE
	out1 = it.CombineCompoundMapsToSet(out1, ctx.chanid_remoteaddr1_map);
	out2 = it.CombineCompoundMapsToSet(out2, ctx.chanid_remoteaddr2_map);
#else
	out1 = it.CombineCompoundMapsToSet(out1, chanid_remoteaddr1_map);
	out2 = it.CombineCompoundMapsToSet(out2, chanid_remoteaddr2_map);
#endif
}

void ParseWorker::append_chanid_comwmwords_map(std::map<uint32_t, std::set<uint32_t>>& out)
{
	IterableTools it;
#ifdef PARSER_REWRITE
	out = it.CombineCompoundMapsToSet(out, ctx.chanid_commwords_map);
#else
	out = it.CombineCompoundMapsToSet(out, chanid_commwords_map);
#endif
}

#ifdef VIDEO_DATA
const std::map<uint16_t, uint64_t>& ParseWorker::GetChannelIDToMinTimeStampMap()
{
#ifdef PARSER_REWRITE
	return ctx.chanid_minvideotimestamp_map;
#else
	return video->GetChannelIDToMinTimeStampMap();
#endif
}
#endif