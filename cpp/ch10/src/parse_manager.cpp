// parse_manager.cpp

#include "parse_manager.h"

ParseManager::ParseManager(std::string fname, std::string output_path, const ParserConfigParams * const config) : 
	config_(config),
	input_fname(fname), 
	output_path(output_path), 
	read_size(10000), 
	append_read_size(10000000), 
	total_size(0), 
	total_read_pos(0),
	n_threads(1), 
	tmats(), 
	error_set(false),
	workers_allocated(false),
	worker_wait(config->worker_shift_wait_ms_),
	worker_start_offset(config->worker_offset_wait_ms_), 
	ifile(),
	use_comet_command_words(false), 
	binary_buffers(nullptr), 
	check_word_count(true),
	n_reads(0), 
	threads(nullptr), 
	tmats_present(false),
	workers(nullptr), 
	milstd1553_msg_selection(false)
	,parser_md_()
{
#ifdef DEBUG
#if DEBUG > 0
	printf("Ch10 file path: %s\n", input_fname.c_str());

#endif
#endif

	// Use MilStd1553 conf file data to create a vector of sorted message
	// names if required.
#ifdef XDAT
	create_milstd1553_sorted_msgs();
#endif

	if (!error_set)
	{
		// Open file and determine size.
		ifile.open(input_fname.c_str(), std::ios::binary | std::ios::ate);
		if (!(ifile.is_open()))
		{
			printf("Error opening file: %s\n", input_fname.c_str());
			error_set = true;
		}
		else
		{
			total_size = ifile.tellg();
			printf("total size is %llu\n", total_size);
			ifile.seekg(0);
#ifdef DEBUG
			if (DEBUG > 0)
				printf("File size: %llu MB\n\n", total_size / (1024 * 1024));
#endif

#ifndef LIBIRIG106
	// Parse TMATS.
	bool use_default_bus_id_map = true;
	error_set = parse_tmats(use_default_bus_id_map);
#endif

			// Create file output paths based on the input file name.
			create_paths();
		}
	}
}

void ParseManager::create_paths()
{

#ifdef PARQUET
	// Create parquet output file directory.
	std::filesystem::path input_path(input_fname);
	std::filesystem::path out_path(output_path);
	std::filesystem::path parquet_1553_path = out_path / input_path.stem();
	parquet_1553_path += std::filesystem::path("_1553.parquet");
	printf("Parquet 1553 output path: %s\n", parquet_1553_path.string().c_str());

	if (!std::filesystem::exists(parquet_1553_path))
	{
		//printf("parquet path DOES NOT EXIST!\n");
		bool create_dir_success = false;
		create_dir_success = std::filesystem::create_directory(parquet_1553_path);
		if (!create_dir_success)
		{
			error_set = true;
			printf("Creation of directory %s failed\n", parquet_1553_path.string().c_str());
		}
	}
	fspath_map[Ch10DataType::MILSTD1553_DATA_F1] = parquet_1553_path;

#ifdef VIDEO_DATA
	std::filesystem::path parquet_vid_path = out_path / input_path.stem();
	parquet_vid_path += std::filesystem::path("_video.parquet");
	printf("Parquet video output path: %s\n", parquet_vid_path.string().c_str());
	fspath_map[Ch10DataType::VIDEO_DATA_F0] = parquet_vid_path;

	if (!std::filesystem::exists(parquet_vid_path))
	{
		bool create_dir_success = false;
		create_dir_success = std::filesystem::create_directory(parquet_vid_path);
		if (!create_dir_success)
		{
			error_set = true;
			printf("Creation of directory %s failed\n", parquet_vid_path.string().c_str());
		}
	}
#endif
#ifdef ETHERNET_DATA
	std::filesystem::path parquet_eth_path = out_path / input_path.stem();
	parquet_eth_path += std::filesystem::path("_ethernet.parquet");
	printf("Parquet video output path: %s\n", parquet_eth_path.string().c_str());
	fspath_map[Ch10DataType::ETHERNET_DATA_F0] = parquet_eth_path;

	if (!std::filesystem::exists(parquet_eth_path))
	{
		bool create_dir_success = false;
		create_dir_success = std::filesystem::create_directory(parquet_eth_path);
		if (!create_dir_success)
		{
			error_set = true;
			printf("Creation of directory %s failed\n", parquet_eth_path.string().c_str());
		}
	}
#endif
#endif
}

bool ParseManager::error_state()
{
	return error_set;
}

bool ParseManager::parse_tmats(bool use_def_bus_map)
{
	uint32_t tmats_eof = UINT32_MAX;
	tmats_eof = tmats.parse(ifile, total_size, use_def_bus_map);
	if(tmats_eof == 0)
	{
		#ifdef DEBUG
		if (DEBUG > 2)
			printf("TMATS not present\n");
		#endif
		tmats_present = false;
	}
	else if(tmats_eof < UINT32_MAX)
	{
		#ifdef DEBUG
		if (DEBUG > 2)
			printf("TMATS EOF at %d\n", tmats_eof);
		#endif
		tmats_present = true;
	}
	else if (tmats_eof == UINT32_MAX)
	{
		return true;
	}
	return false;
}

void ParseManager::start_workers()
{
	read_size = config_->parse_chunk_bytes_ * 1e6;
	n_threads = config_->parse_thread_count_;
	uint16_t max_workers = config_->max_chunk_read_count_;
	
	check_word_count = true;
	total_read_pos = 0;

	// Calculate the number of reads necessary to parse the entire file.
	n_reads = int(ceil(float(total_size) / float(read_size)));
	if (max_workers < n_reads)
		n_reads = max_workers;
	workers = new ParseWorker[n_reads];
	threads = new std::thread[n_reads];
	binary_buffers = new BinBuff[n_reads];
	workers_allocated = true;

	printf("\nInput file: %s\n", input_fname.c_str());
	printf("Using %u threads\n", n_threads);
#ifdef DEBUG
	if (DEBUG > 0)
	{
		printf("Created %u workers\n", n_reads);
		printf("Created %u binary buffers\n", n_reads);
	}
#endif	

	// Advance the total read position by the tmats end position.
	uint32_t tmats_end = tmats.post_tmats_location();

	// If libirig106 is defined, let the value of total_read_pos remain
	// at its initialization value of 0. Do this so libirig106 can 
	// parse the TMATS Ch10 packet. Otherwise, assume that TMATS has 
	// already been found and read, and set the total_read_pos to 
	// the end of TMATS to skip it.
#ifndef LIBIRIG106
	total_read_pos = tmats_end;
#endif

	// Start queue to activate all workers, limiting the quantity of 
	// concurrent threads to n_threads.
	bool append = false;
	worker_queue(append);
	printf("after worker_queue\n");

	// Wait for all active workers to finish.
	worker_retire_queue();

	// After all workers have finished and joined, each worker except the
	// last will parse the dangling packets that occur at the end of each 
	// worker's section of raw data and at the beginning of the following 
	// worker's section of data that occurs at the beginning of it's data
	// payload before it finds the time data packet to establish absolute
	// time. 

	// Delete current threads, then allocate new threads for each worker.
	// Quantity of threads is n_reads-1 because the last worker, which 
	// reaches the end of the file, does not leave dangling packets and thus
	// doesn't require another thread to be run again. 
	delete[] threads;
	threads = new std::thread[n_reads];

#ifdef DEBUG
	if (DEBUG > 0)
		printf("\n-- Parsing dangling packets --\n");
#endif
	append = true;
	worker_queue(append);

	// Wait for all active workers to finish.
	worker_retire_queue();

#ifdef COLLECT_STATS
	collect_stats();
#endif

#ifdef PARQUET
#endif

	collect_chanid_to_lruaddrs_metadata();
#ifdef LIBIRIG106
	ProcessTMATS();
#else
	collect_tmats_metadata();
#endif
#ifdef VIDEO_DATA
	CollectVideoMetadata();
#endif
	write_metadata();

}

#ifdef VIDEO_DATA
void ParseManager::CollectVideoMetadata()
{
	// Gather the maps from each worker and combine them into one, 
	//keeping only the lowest time stamps for each channel ID.
	std::map<uint16_t, uint64_t> final_map;
	for (int i = 0; i < n_reads; i++)
	{
		std::map<uint16_t, uint64_t> temp_map = workers[i].GetChannelIDToMinTimeStampMap();
		for (std::map<uint16_t, uint64_t>::const_iterator it = temp_map.begin();
			it != temp_map.end(); ++it)
		{
			if (final_map.count(it->first) == 0)
				final_map[it->first] = it->second;
			else if (it->second < final_map[it->first])
				final_map[it->first] = it->second;

		}
	}
	parser_md_.WriteVideoMetadataToYaml(fspath_map[Ch10DataType::VIDEO_DATA_F0],
		final_map);
}
#endif

void ParseManager::collect_chanid_to_lruaddrs_metadata()
{
	std::map<uint32_t, std::set<uint16_t>> chanid_remoteaddr_map1;
	std::map<uint32_t, std::set<uint16_t>> chanid_remoteaddr_map2;
	for (uint16_t read_ind = 0; read_ind < n_reads; read_ind++)
	{
		workers[read_ind].append_chanid_remoteaddr_maps(chanid_remoteaddr_map1, chanid_remoteaddr_map2);
	}
	parser_md_.create_chanid_to_lruaddrs_metadata_strings(chanid_remoteaddr_map1, chanid_remoteaddr_map2);

}

void ParseManager::collect_tmats_metadata()
{
	parser_md_.create_tmats_channel_source_metadata_string(tmats.get_channel_source_map());
	parser_md_.create_tmats_channel_type_metadata_string(tmats.get_channel_type_as_string_map());
}

void ParseManager::write_metadata()
{
	parser_md_.Write1553metadataToYaml(fspath_map[Ch10DataType::MILSTD1553_DATA_F1]);

	// Test reading metadata and reconstructing maps.
	/*std::string parquet_dir = fspath_map[Ch10DataType::MILSTD1553_DATA_F1].string();
	parser_md_.read_metadata(parquet_dir);*/
}

std::streamsize ParseManager::activate_worker(uint16_t binbuff_ind, uint16_t ID,
	uint64_t start_pos, uint32_t n_read)
{
	uint64_t read_count = binary_buffers[binbuff_ind].Initialize(ifile, total_size, start_pos, n_read);
#ifdef PARQUET
	bool is_final_worker = false;
	if (ID == n_reads - 1)
	{
		is_final_worker = true;
	}
	workers[ID].initialize(ID, start_pos, n_read, binbuff_ind, fspath_map,
		tmats, use_comet_command_words, is_final_worker);
#endif

	#ifdef DEBUG
	if (DEBUG > 1)
	{
		printf("Init. worker %u: start = %llu, read size = %u, bb ind = %u\n", 
			ID, start_pos, n_read, binbuff_ind);
		printf("Output file name: %s\n", worker_outfile_name(ID).c_str());
	}
	#endif
		
	// Start this instance of ParseWorker. Append mode false.
#ifdef LIBIRIG106
	threads[ID] = std::thread(std::ref(workers[ID]), std::ref(binary_buffers[binbuff_ind]),
		false, check_word_count, milstd1553_msg_selection, milstd1553_sorted_msg_selection, 
		std::ref(tmats_body_vec_));
#else
	threads[ID] = std::thread(std::ref(workers[ID]), std::ref(binary_buffers[binbuff_ind]),
		false, check_word_count, milstd1553_msg_selection, milstd1553_sorted_msg_selection);
#endif
	
	return read_count;
}

std::streamsize ParseManager::activate_append_mode_worker(uint16_t binbuff_ind, uint16_t ID,
	uint32_t n_read)
{
	// Reset parse worker completion status.
	uint64_t last_pos = workers[ID].get_last_position();
	workers[ID].reset_completion_status();

	#ifdef DEBUG
	if (DEBUG > 1)
	{
		printf("Init. append mode worker %u: start = %llu, read size = %u, bb ind = %u\n",
			ID, last_pos, n_read, binbuff_ind);
	}
	#endif

	uint64_t read_count = binary_buffers[binbuff_ind].Initialize(ifile,
		total_size, last_pos, n_read);

	workers[ID].append_mode_initialize(n_read, binbuff_ind, last_pos);

	// Start this instance of ParseWorker. Append mode true.
#ifdef LIBIRIG106
	threads[ID] = std::thread(std::ref(workers[ID]), std::ref(binary_buffers[binbuff_ind]),
		true, check_word_count, milstd1553_msg_selection, milstd1553_sorted_msg_selection, 
		std::ref(tmats_body_vec_));
#else
	threads[ID] = std::thread(std::ref(workers[ID]), std::ref(binary_buffers[binbuff_ind]),
		true, check_word_count, milstd1553_msg_selection, milstd1553_sorted_msg_selection);
#endif
	return read_count;
}

std::string ParseManager::worker_outfile_name(uint16_t worker_ind)
{
	std::string key = ".ch10";
	std::string output_fname = input_fname;
	char buff[10];
	sprintf(buff, "__%03u.h5", worker_ind);
	std::string s(buff);
	size_t pos = output_fname.rfind(key);
	if(pos == std::string::npos)
	{
		printf("Substring not found: %s\n", key.c_str());
		std::string def = "default.h5";
		return def;
	}
	output_fname.replace(pos, key.length(), s);

	// Create a path object to extract the file name 
	// alone and append to the output path.
	std::filesystem::path outpath(output_path);
	std::filesystem::path old_output_fname(output_fname);
	std::filesystem::path new_output_fname = outpath / old_output_fname.filename();
	output_fname = new_output_fname.string();

	return output_fname; 
}

std::string ParseManager::final_outfile_name()
{
	std::string key = ".ch10";
	std::string output_fname = input_fname;
	/*char buff[10];
	sprintf(buff, ".h5");
	std::string s(buff);*/
	std::string s = ".h5";
	size_t pos = output_fname.rfind(key);
	if (pos == std::string::npos)
	{
		printf("Substring not found: %s\n", key.c_str());
		std::string def = "final_default.h5";
		return def;
	}
	output_fname.replace(pos, key.length(), s);

	// Create a path object to extract the file name 
	// alone and append to the output path.
	std::filesystem::path outpath(output_path);
	std::filesystem::path old_output_fname(output_fname);
	std::filesystem::path new_output_fname = outpath / old_output_fname.filename();
	output_fname = new_output_fname.string();

	return output_fname;
}

void ParseManager::worker_queue(bool append_mode)
{
	// Initially load all threads by keeping track of the
	// the number activated. Afterwards only activate new 
	// threads when an active thread finishes.
	uint16_t active_thread_count = 0;
	uint16_t current_active_worker = 0;
	bool thread_started = false;
	bool all_threads_active = false;
	bool eof_reached = false;
	uint16_t max_worker_ind = 0;
	std::streamsize current_read_count = 0;
	uint16_t bb_ind = 0;
	int concurrent_thread_count = 0;

	#ifdef DEBUG
	if (DEBUG > 0)
		printf("Starting worker threads\n");
	#endif

	// Create and wait for workers until the entire input file 
	// is parsed.

	// Loop over each block of data that is to be read and parsed. There is a 1:1
	// relationship between a worker and a block of data--each worker consumes a 
	// single data block. In the append_mode = true
	// case, the very last worker doesn't have any dangling packets because presumably
	// it parsed data until the ch10 file ended. Only [worker 1, worker last) need to
	// append data to their respective files. 
	uint16_t n_read_limit = n_reads;
	if (append_mode)
	{
		// If there is only 1 worker, then there is no need to 
		// append data. Also, there is no reason to append data
		// to the last worker (i.e., the worker that read the last 
		// portion of the Ch10 file) so only append data for all
		// workers except the last worker. 
		if (n_read_limit > 1)
			n_read_limit--;
		else
			return;
	}
	for (uint16_t read_ind = 0; read_ind < n_read_limit; read_ind++)
	{
		thread_started = false;
		// Stay in while loop until another worker is started. 
		while (!thread_started)
		{
			if (!all_threads_active)
			{
				#ifdef DEBUG
				if (DEBUG > 1)
					printf("\nAll threads NOT ACTIVE (%u active).\n", active_thread_count);
				#endif

				// Immediately activate a new worker.
				if (append_mode)
				{
					//printf("Ready to activate_append_mode_worker, active_thread_count = %hu, read_ind = %hu\n", active_thread_count, read_ind);
					current_read_count = activate_append_mode_worker(active_thread_count,
						read_ind, append_read_size);
				}
				else
				{
					current_read_count = activate_worker(active_thread_count, read_ind, total_read_pos,
						read_size);
				}

				// Check if the correct number of bytes were read.
				if (append_mode)
				{
					if (current_read_count != append_read_size)
					{
						eof_reached = true;
						printf("Current read count %Iu, not equal to expected size %u\n", current_read_count, append_read_size);
					}
				}
				else if (current_read_count != read_size)
				{
					eof_reached = true;
					#ifdef DEBUG
					if (DEBUG > 1)
						printf("EOF reached: %Iu/%u read\n", current_read_count, read_size);
					#endif
				}

				// Put worker in active workers list.
				active_workers.push_back(read_ind);
				
				//max_worker_ind = read_ind;

				active_thread_count += 1;
				if (active_thread_count == n_threads)
					all_threads_active = true;

				thread_started = true;

				// Stagger start time of initial worker threads. Because workers parse large chunks
				// of data and there are potentially 1e5 or 1e6 messages, the law of averages applies
				// and workers tend to take nearly identical amount of time to complete. This means
				// that the OS tries to write any remaining unwritten data to disk for each worker
				// then start a series of new workers which requires reading large amount of data 
				// from the disk, which results in an IO bottleneck that occurs every time the current
				// shift of workers finishes. Stagger the initial start time of workers to avoid this
				// bottleneck.
				if(!append_mode && active_thread_count != n_reads)
					std::this_thread::sleep_for(worker_start_offset);
			}
			else
			{
				#ifdef DEBUG
				if (DEBUG > 1)
					printf("\nAll threads ACTIVE (%u active).\n", active_thread_count);
				#endif
				// Check active workers to see if they are ready to be joined.
				for (size_t act_work_ind = 0; act_work_ind < active_workers.size(); act_work_ind++)
				{
					
					// Join workers that are complete
					if (workers[active_workers[act_work_ind]].completion_status() == true)
					{
						#ifdef DEBUG
						if (DEBUG > 1)
							printf("Worker %u INACTIVE/COMPLETE -- joining now\n", active_workers[act_work_ind]);

						#endif

						bb_ind = workers[active_workers[act_work_ind]].get_binbuff_ind();
						threads[active_workers[act_work_ind]].join();
						active_workers.erase(active_workers.begin() + act_work_ind);

						// Immediately start a new worker.
						if (append_mode)
						{
							current_read_count = activate_append_mode_worker(bb_ind,
								read_ind, append_read_size);
						}
						else
						{
							current_read_count = activate_worker(bb_ind, read_ind, total_read_pos,
								read_size);
						}

						// Check if the correct number of bytes were read.
						if (append_mode)
						{
							if (current_read_count != append_read_size)
							{
								printf("Worker %hu Error: %Iu bytes read, %u bytes indicated\n",
									read_ind, current_read_count, append_read_size);
							}
						}
						else if (current_read_count != read_size)
						{
							eof_reached = true;
							#ifdef DEBUG
							if (DEBUG > 1)
								printf("EOF reached: %Iu/%u read\n", current_read_count, read_size);
							#endif
						}

						// Place new worker among active workers.
						active_workers.push_back(read_ind);
						//max_worker_ind = read_ind;

						thread_started = true;
						break;
					}
					else
					{
						#ifdef DEBUG
						if (DEBUG > 1)
							printf("Worker %u STILL ACTIVE\n", active_workers[act_work_ind]);
						#endif
					}

				}
			}

			// Wait before checking for available workers.
			#ifdef DEBUG
			if (DEBUG > 1)
				printf("Waiting for workers\n");
			#endif
			std::this_thread::sleep_for(worker_wait);

		} // end while 

		// Increase the total read position.
		total_read_pos += current_read_count;

		if (eof_reached)
			break;

	} // end for loop over all worker indices.
}

void ParseManager::worker_retire_queue()
{
	#ifdef DEBUG
	if (DEBUG > 0)
		printf("\n-- Joining all remaining workers --\n");
	#endif
	while (active_workers.size() > 0)
	{
		for (size_t act_work_ind = 0; act_work_ind < active_workers.size(); act_work_ind++)
		{
			// Join workers that are complete
			if (workers[active_workers[act_work_ind]].completion_status() == true)
			{
				#ifdef DEBUG
				if (DEBUG > 0)
					printf("Worker %hu INACTIVE/COMPLETE -- joining now\n", active_workers[act_work_ind]);
				#endif
			
				threads[active_workers[act_work_ind]].join();

#ifdef PARQUET
#endif

				#ifdef DEBUG
				if (DEBUG > 2)
				{
					printf("Worker %u joined\n", active_workers[act_work_ind]);
				}
				#endif
				if (n_reads == 1)
					active_workers.resize(0);
				else
					active_workers.erase(active_workers.begin() + act_work_ind);
			}
			else
			{
				#ifdef DEBUG
				if (DEBUG > 1)
					printf("Worker %u STILL ACTIVE\n", active_workers[act_work_ind]);
				#endif
			}
		}

		// Wait before checking for available workers.
		#ifdef DEBUG
		if (DEBUG > 1)
			printf("Waiting for workers to complete\n");
		#endif
		std::this_thread::sleep_for(worker_wait);
	}

	#ifdef DEBUG
	if (DEBUG > 0)
		printf("\n-- All workers joined --\n");
	#endif
}

void ParseManager::collect_stats()
{
	PacketStats pstats;
	PacketStats pstats_error;
	PacketStats* pstats_temp;
	//Ch10MilStd1553F1Stats milstdstats;
	Ch10MilStd1553F1Stats* milstdstats = workers[0].milstd1553_stats();

	// Loop over all workers and collect stats from each.
	for (uint16_t worker_ind = 0; worker_ind < n_reads; worker_ind++)
	{
		// Get present workers packet stats and sum with final count.
		pstats_temp = workers[worker_ind].get_packet_ledger();
		pstats.time_data_count += pstats_temp->time_data_count;
		pstats.milstd_1553pkt_count += pstats_temp->milstd_1553pkt_count;
		pstats.video_data_count += pstats_temp->video_data_count;
		pstats.discrete_data_count += pstats_temp->discrete_data_count;
		pstats.analog_data_count += pstats_temp->analog_data_count;

		pstats_temp = workers[worker_ind].get_packet_error_ledger();
		pstats_error.time_data_count += pstats_temp->time_data_count;
		pstats_error.milstd_1553pkt_count += pstats_temp->milstd_1553pkt_count;
		pstats_error.video_data_count += pstats_temp->video_data_count;
		pstats_error.discrete_data_count += pstats_temp->discrete_data_count;
		pstats_error.analog_data_count += pstats_temp->analog_data_count;

		// Get present workers mil std 1553 message stats.
		if (worker_ind > 0)
			milstdstats->add_stats(workers[worker_ind].milstd1553_stats());
		/*milstdstats_temp = workers[worker_ind].milstd1553_stats();
		milstdstats.msg_identity_fail += milstdstats_temp->msg_identity_fail;
		milstdstats.wrd_count_mismatch += milstdstats_temp->wrd_count_mismatch;
		milstdstats.ts_not_impl += milstdstats_temp->ts_not_impl;
		milstdstats.ts_reserved += milstdstats_temp->ts_reserved;*/
	}

	#ifdef DEBUG
	if (DEBUG > 0)
	{
		uint32_t err = 0;
		uint32_t noerr = 0;
		err = pstats_error.time_data_count;
		noerr = pstats.time_data_count;
		printf("\n-- Packet Type Stats (err/total : %%) --\n");
		printf("Time data          : %02u/%05u : %f\n", err, err + noerr, float(err) / (err + noerr));

		err = pstats_error.milstd_1553pkt_count;
		noerr = pstats.milstd_1553pkt_count;
		printf("MilStd 1553        : %02u/%05u : %f\n", err, err + noerr, float(err) / (err + noerr));

		err = pstats_error.video_data_count;
		noerr = pstats.video_data_count;
		printf("Video data         : %02u/%05u : %f\n", err, err + noerr, float(err) / (err + noerr));

		err = pstats_error.discrete_data_count;
		noerr = pstats.discrete_data_count;
		printf("Discrete data      : %02u/%05u : %f\n", err, err + noerr, float(err) / (err + noerr));

		err = pstats_error.analog_data_count;
		noerr = pstats.analog_data_count;
		printf("Analog data        : %02u/%05u : %f\n", err, err + noerr, float(err) / (err + noerr));

		printf("\n-- MilStd 1553 Error Stats --\n");
		printf("MSG_IDENTITY_FAIL  : %03u\n", milstdstats->msg_identity_fail);
		printf("WRD_COUNT_MISMATCH : %03u\n", milstdstats->wrd_count_mismatch);
		printf("TS_NOT_IMPL        : %03u\n", milstdstats->ts_not_impl);
		printf("TS_RESERVED        : %03u\n\n", milstdstats->ts_reserved);
	}
	#endif

	milstdstats->print_msg_stats();
}

#ifdef PARQUET
// Concatenate data files which contain multiple data types.
// Not implemented yet, but I expect this to modified for use
// with Parquet or ORC files output.
void ParseManager::concatenate_data_files()
{

}
#endif

ParseManager::~ParseManager()
{
	ifile.close();
	//printf("in destructor ... \n");
	if(workers_allocated)
	{
		#ifdef DEBUG
		if (DEBUG > 1)
			printf("ParseManager: Deleting \"workers\", \"binary_buffers\", \"threads\"\n");
		#endif
		
		delete[] threads;
		//printf("after delete threads\n");
		delete[] workers;
		//printf("after delete workers\n");
		delete[] binary_buffers;
		//printf("after delete binary_buffers\n");
		
	}
}

bool ParseManager::file_exists(std::string& path)
{
	std::ifstream temp_ifile;
	temp_ifile.open(path.c_str());
	bool exists = false;
	if (temp_ifile.is_open())
		exists = true;
	temp_ifile.close();
	return exists;
}

#ifdef PARQUET
void ParseManager::record_msg_names()
{
	std::filesystem::path input_path(input_fname);
	std::filesystem::path out_path(output_path);
	std::filesystem::path msg_names_path = out_path / input_path.stem();
	msg_names_path += std::filesystem::path("_1553_messages.txt");
	printf("Message names output path: %s\n", msg_names_path.string().c_str());

	std::ofstream msg_file(msg_names_path.string());
	if (msg_file.is_open())
	{
		for (std::set<std::string>::iterator it = name_set.begin(); it != name_set.end(); ++it)
		{
			msg_file << *it << "\n";
		}
		msg_file.close();
	}
	else
		printf("Unable to open file: %s\n", msg_names_path.string().c_str());
}

#endif

void ParseManager::ProcessTMATS()
{
	// if tmats doesn't exist return
	if (tmats_body_vec_.size() == 0)
	{
		printf("\nNo TMATS Present\n");
		return;
	}

	std::string full_TMATS_string;
	for (int i = 0; i < tmats_body_vec_.size(); i++)
	{
		full_TMATS_string += tmats_body_vec_[i];
	}

	std::filesystem::path path;
	path = fspath_map[Ch10DataType::MILSTD1553_DATA_F1] /
		std::filesystem::path("_TMATS.txt");
	std::ofstream tmats;
	tmats.open(path.string(), std::ios::trunc | std::ios::binary);
	if (tmats.good())
	{
		printf("\nWriting TMATS to %s\n", path.string().c_str());
		tmats << full_TMATS_string;
	}

	tmats.close();

	// Gather TMATs attributes of interest
	// for metadata
	TMATSParser tmats_parser = TMATSParser(full_TMATS_string);
	TMATsChannelIDToSourceMap = tmats_parser.MapAttrs("R-x\\TK1-n", "R-x\\DSI-n");
	TMATsChannelIDToTypeMap = tmats_parser.MapAttrs("R-x\\TK1-n", "R-x\\CDT-n");
}