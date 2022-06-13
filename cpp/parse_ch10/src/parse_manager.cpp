// parse_manager.cpp

#include "parse_manager.h"

ParseManager::ParseManager() : worker_count_(0), worker_count(worker_count_), worker_chunk_size_bytes_(0), worker_chunk_size_bytes(worker_chunk_size_bytes_), workers_vec(workers_vec_), threads_vec(threads_vec_), worker_config_vec(worker_config_vec_), ch10_input_stream_(), append_chunk_size_bytes_(100000000), parser_paths_(), parser_metadata_()
{
}

const std::string ParseManager::metadata_filename_ = "_metadata.yaml";

bool ParseManager::Configure(ManagedPath input_ch10_file_path, ManagedPath output_dir,
                             const ParserConfigParams& user_config)
{
    if(!parser_metadata_.Initialize(input_ch10_file_path))
        return false;

    bool success = false;
    input_ch10_file_path.GetFileSize(success, ch10_file_size_);
    if (!success)
        return false;
    spdlog::get("pm_logger")->info("Ch10 file size: {:f} MB", ch10_file_size_ / (1000.0 * 1000.0));

    if (!AllocateResources(user_config, ch10_file_size_))
        return false;

    if(!parser_paths_.CreateOutputPaths(input_ch10_file_path, output_dir, 
        user_config.ch10_packet_enabled_map_, worker_count_))
        return false;

    // Record the packet type config map in metadata and logs.
    LogPacketTypeConfig(user_config.ch10_packet_enabled_map_);

    // Open the input file stream
    spdlog::get("pm_logger")->debug("Opening ch10 file path: {:s}", input_ch10_file_path.string());
    ch10_input_stream_.open(input_ch10_file_path.string().c_str(), std::ios::binary);
    if (!(ch10_input_stream_.is_open()))
    {
        spdlog::get("pm_logger")->error("Error opening file: {:s}", input_ch10_file_path.RawString());
        ch10_input_stream_.close();
        return false;
    }

    return true;
}

bool ParseManager::AllocateResources(const ParserConfigParams& user_config,
                                     const uint64_t& ch10_file_size)
{
    /*
	Note that the CLI has limits applied to the parameters so there
	is no need to check range, etc.
	*/

    // Multiply by 1e6 because this configuration parameters is in units
    // of MB.
    worker_chunk_size_bytes_ = user_config.parse_chunk_bytes_ * 1e6;

    // Calculate the number of workers necessary to parse the entire file
    // based on the chunk of binary that each worker will parse.
    worker_count_ = static_cast<int>(ceil(static_cast<double>(ch10_file_size) / static_cast<double>(worker_chunk_size_bytes_)));

    spdlog::get("pm_logger")->info("AllocateResources: chunk size {:d} bytes", worker_chunk_size_bytes_);
    spdlog::get("pm_logger")->info("AllocateResources: using {:d} threads", user_config.parse_thread_count_);

    // If the user-specified max_chunk_read_count is less the calculated worker_count_,
    // decrease the worker_count_ to max_chunk_read_count.
    if (user_config.max_chunk_read_count_ < worker_count_)
        worker_count_ = user_config.max_chunk_read_count_;
    spdlog::get("pm_logger")->info("AllocateResources: creating {:d} workers", worker_count_);

    // Allocate objects necessary to parse each chunk
    spdlog::get("pm_logger")->debug(
        "AllocateResources: allocating memory for threads "
        "and WorkerConfig objects");
    threads_vec_.resize(worker_count_);
    worker_config_vec_.resize(worker_count_);

    // Allocate worker object for each unique_ptr<ParseWorker>
    spdlog::get("pm_logger")->debug("AllocateResources: creating ParseWorker objects");
    for (uint16_t worker_ind = 0; worker_ind < worker_count_; worker_ind++)
    {
        workers_vec_.push_back(std::make_unique<ParseWorker>());
        context_vec_.push_back(std::make_unique<Ch10Context>());
    }

    return true;
}

bool ParseManager::Parse(const ParserConfigParams& user_config)
{
    // Start queue to activate all workers, limiting the quantity of
    // concurrent threads to n_threads.
    bool append = false;
    uint16_t effective_worker_count = workers_vec_.size();
    std::vector<uint16_t> active_workers_vec;
    spdlog::get("pm_logger")->debug("Parse: begin parsing with workers");
    if (!WorkerQueue(append, ch10_input_stream_, workers_vec_, active_workers_vec,
                     worker_config_vec_, effective_worker_count, worker_chunk_size_bytes_,
                     append_chunk_size_bytes_, ch10_file_size_, parser_paths_.GetWorkerPathVec(),
                     parser_paths_.GetCh10PacketTypeEnabledMap(), threads_vec_, 
                     tmats_body_vec_, user_config))
    {
        spdlog::get("pm_logger")->warn("Parse: Returning after first WorkerQueue");
        return false;
    }
    spdlog::get("pm_logger")->debug("Parse: end parsing with workers");

    // Wait for all active workers to finish.
    if (!WorkerRetireQueue(workers_vec_, active_workers_vec, worker_config_vec_,
                           threads_vec_, user_config.worker_shift_wait_ms_))
    {
        return false;
    }

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

    // If there is only 1 worker, then there is no need to
    // append data. Also, there is no reason to append data
    // to the last worker (i.e., the worker that read the last
    // portion of the Ch10 file) so only append data for all
    // workers except the last worker.
    append = true;
    if (effective_worker_count > 1)
        effective_worker_count--;
    else
        return true;
    threads_vec_.clear();
    threads_vec_.resize(effective_worker_count);

    spdlog::get("pm_logger")->debug("Parse: begin parsing in append mode");

    if (!WorkerQueue(append, ch10_input_stream_, workers_vec_, active_workers_vec,
                     worker_config_vec_, effective_worker_count, worker_chunk_size_bytes_,
                     append_chunk_size_bytes_, ch10_file_size_, parser_paths_.GetWorkerPathVec(),
                     parser_paths_.GetCh10PacketTypeEnabledMap(), threads_vec_, 
                     tmats_body_vec_, user_config))
    {
        spdlog::get("pm_logger")->warn("Parse: Returning after append mode WorkerQueue");
        return false;
    }
    spdlog::get("pm_logger")->debug("Parse: end parsing in append mode");

    // Wait for all active workers to finish.
    if (!WorkerRetireQueue(workers_vec_, active_workers_vec, worker_config_vec_,
                           threads_vec_, user_config.worker_shift_wait_ms_))
    {
        return false;
    }
    spdlog::get("pm_logger")->info("Parse: Parsing complete with no errors");
    return true;
}

bool ParseManager::RecordMetadata(ManagedPath input_ch10_file_path,
                                  const ParserConfigParams& user_config)
{
    // Create a set of all the parsed packet types
    std::set<Ch10PacketType> parsed_packet_types;
    AssembleParsedPacketTypesSet(parsed_packet_types);

    std::vector<const Ch10Context*> ctx_vec;
    for(std::vector<std::unique_ptr<Ch10Context>>::iterator it = context_vec_.begin();
        it != context_vec_.end(); ++it)
    { ctx_vec.push_back((*it).get()); }

    ManagedPath md_filename(metadata_filename_);
    if(!parser_metadata_.RecordMetadata(md_filename, &parser_paths_, parsed_packet_types,
        user_config, tmats_body_vec_, ctx_vec))
        return false;

    parser_paths_.RemoveCh10PacketOutputDirs(parsed_packet_types);

    spdlog::get("pm_logger")->debug("Recording Time Data");
    ParquetContext pq_ctx;
    ParquetTDPF1 pq_tdp(&pq_ctx);
    WriteTDPData(ctx_vec, &pq_tdp, parser_paths_.GetTDPOutputPath());

    return true;
}

bool ParseManager::ConfigureWorker(WorkerConfig& worker_config, const uint16_t& worker_index,
                                   const uint16_t& worker_count, const uint64_t& read_pos, const uint64_t& read_size,
                                   const uint64_t& total_size, BinBuff* binbuff_ptr,
                                   std::ifstream& ch10_input_stream, std::streamsize& actual_read_size,
                                   const std::map<Ch10PacketType, ManagedPath>& output_file_path_map,
                                   const std::map<Ch10PacketType, bool>& packet_type_config_map)
{
    // Check for invalid worker_index
    if (worker_index > worker_count - 1)
    {
        spdlog::get("pm_logger")->warn("ConfigureWorker: worker_index ({:d}) > worker_count ({:d}) - 1", worker_index, worker_count);
        return false;
    }

    // Final worker is the last in order of index and chunks to be parsed
    // from the ch10. As such there is no append mode for this worker, so
    // it will need to know in order to close the file writers prior
    // to returning.
    worker_config.final_worker_ = false;
    if (worker_index == worker_count - 1)
    {
        worker_config.final_worker_ = true;
    }

    worker_config.worker_index_ = worker_index;
    worker_config.start_position_ = read_pos;
    worker_config.append_mode_ = false;
    worker_config.output_file_paths_ = output_file_path_map;
    worker_config.ch10_packet_type_map_ = packet_type_config_map;

    spdlog::get("pm_logger")->debug("ConfigureWorker {:d}: start = {:d}, read size = {:d}", worker_index, read_pos, read_size);

    actual_read_size = binbuff_ptr->Initialize(ch10_input_stream,
                                               total_size, read_pos, read_size);

    if (actual_read_size == UINT64_MAX)
        return false;

    if (actual_read_size != read_size)
    {
        spdlog::get("pm_logger")->debug(
            "ConfigureWorker: worker {:d} actual read size ({:d}) "
            "not equal to requested read size ({:d})",
            worker_index, actual_read_size, read_size);

        // If the last worker is being configured, it will undoubtedly reach the
        // EOF and this will occur, which is not an error.
        if (worker_index == worker_count - 1 && actual_read_size < read_size)
        {
            spdlog::get("pm_logger")->debug("ConfigureWorker: Last worker reached EOF OK");
            return true;
        }
        return false;
    }
    return true;
}

bool ParseManager::ConfigureAppendWorker(WorkerConfig& worker_config, const uint16_t& worker_index,
                                         const uint64_t& append_read_size, const uint64_t& total_size, BinBuff* binbuff_ptr,
                                         std::ifstream& ch10_input_stream, std::streamsize& actual_read_size)
{
    uint64_t last_pos = worker_config.last_position_;

    worker_config.start_position_ = last_pos;
    worker_config.append_mode_ = true;

    spdlog::get("pm_logger")->debug(
        "ConfigureAppendWorker: worker {:d} initializing buffer at position "
        "{:d}, reading {:d} bytes from file with total size {:d}",
        worker_index, worker_config.start_position_, append_read_size, total_size);

    actual_read_size = binbuff_ptr->Initialize(ch10_input_stream,
                                               total_size, worker_config.start_position_, append_read_size);

    if (actual_read_size == UINT64_MAX)
        return false;

    // Append mode workers ought to never read to the end of the file.
    // The final append mode worker parses the data beginning at the location
    // where the second to last first-pass worker stopped parsing due to
    // an incomplete packet in the buffer. If a read failure occurs, indicated
    // by the following logic, during configuration of an append-mode worker,
    // then an error has occurred.
    if (actual_read_size != append_read_size)
    {
        // It's possible that the last worker only read, for example,
        // 5MB because of the way the ch10 is chunked. Say the second-to-last
        // worker parsed up to last megabyte or so of the chunk it was given.
        // (Typically it parses up until the last few bytes or kilobytes.)
        // Then the append worker for the second-to-last worker will try to
        // read the current default append_read_size = 100MB. However, in this
        // example there is only 1MB + 5 MB left in the file, so the read
        // size is not what is requested. Allow for this to happen without
        // indicating an error.
        if (worker_config.start_position_ + append_read_size > total_size)
            return true;

        spdlog::get("pm_logger")->warn(
            "ConfigureAppendWorker: worker {:d} actual read size ({:d}) not "
            "equal to requested read size ({:d})",
            worker_index, actual_read_size, append_read_size);
        return false;
    }

    return true;
}

bool ParseManager::ActivateWorker(bool append_mode, std::unique_ptr<ParseWorker>& parse_worker_ptr,
                                  std::thread& worker_thread, WorkerConfig& worker_config, const uint16_t& worker_index,
                                  const uint16_t& worker_count, const uint64_t& read_pos, const uint64_t& read_size,
                                  const uint64_t& append_read_size, const uint64_t& total_size, BinBuff* binbuff_ptr,
                                  std::ifstream& ch10_input_stream, std::streamsize& actual_read_size,
                                  const std::map<Ch10PacketType, ManagedPath>& output_file_path_map,
                                  const std::map<Ch10PacketType, bool>& packet_type_config_map,
                                  std::vector<std::string>& tmats_vec, std::unique_ptr<Ch10Context>& ctx)
{
    if (append_mode)
    {
        if (!ConfigureAppendWorker(worker_config, worker_index,
                                   append_read_size, total_size, binbuff_ptr,
                                   ch10_input_stream, actual_read_size))
        {
            spdlog::get("pm_logger")->warn(
                "ActivateWorker: ConfigureAppendWorker failed during "
                "initial thread loading");
            return false;
        }
    }
    else
    {
        if (!ConfigureWorker(worker_config, worker_index, worker_count,
                             read_pos, read_size, total_size, binbuff_ptr,
                             ch10_input_stream, actual_read_size, output_file_path_map,
                             packet_type_config_map))
        {
            spdlog::get("pm_logger")->warn(
                "ActivateWorker: ConfigureWorker failed during "
                "initial thread loading");
            return false;
        }
    }

    worker_thread = std::thread(std::ref(*parse_worker_ptr),
                                std::ref(worker_config), std::ref(tmats_vec), ctx.get());

    return true;
}

bool ParseManager::WorkerQueue(bool append_mode, std::ifstream& ch10_input_stream,
                               std::vector<std::unique_ptr<ParseWorker>>& worker_vec,
                               std::vector<uint16_t>& active_workers_vec,
                               std::vector<WorkerConfig>& worker_config_vec,
                               const uint16_t& effective_worker_count,
                               const uint64_t& read_size, const uint64_t& append_read_size,
                               const uint64_t& total_size,
                               const std::vector<std::map<Ch10PacketType, ManagedPath>>& output_file_path_vec,
                               const std::map<Ch10PacketType, bool>& packet_type_config_map,
                               std::vector<std::thread>& threads_vec,
                               std::vector<std::string>& tmats_vec,
                               const ParserConfigParams& user_config)
{
    // Load sleep duration from user config
    std::chrono::milliseconds worker_offset_wait_ms(user_config.worker_offset_wait_ms_);
    std::chrono::milliseconds worker_shift_wait_ms(user_config.worker_shift_wait_ms_);

    uint16_t worker_count = worker_vec.size();
    uint16_t active_thread_count = 0;
    bool thread_started = false;
    bool all_threads_active = false;
    std::streamsize actual_read_size = 0;
    uint64_t total_read_pos = 0;

    // Start each worker as threads are available
    spdlog::get("pm_logger")->debug("WorkerQueue: Starting worker threads");
    for (uint16_t worker_ind = 0; worker_ind < effective_worker_count; worker_ind++)
    {
        thread_started = false;

        // Stay in while loop until another worker is started.
        while (!thread_started)
        {
            if (!all_threads_active)
            {
                spdlog::get("pm_logger")->debug("WorkerQueue: All threads NOT ACTIVE ({:d} active)", active_thread_count);

                if (!ActivateWorker(append_mode, worker_vec[worker_ind], threads_vec[worker_ind],
                                    worker_config_vec[worker_ind], worker_ind, worker_count, total_read_pos,
                                    read_size, append_read_size, total_size, &worker_config_vec[worker_ind].bb_,
                                    ch10_input_stream, actual_read_size, output_file_path_vec[worker_ind],
                                    packet_type_config_map, tmats_vec, context_vec_[worker_ind]))
                    return false;

                // Put worker in active workers list.
                active_workers_vec.push_back(worker_ind);

                active_thread_count += 1;
                if (active_thread_count == user_config.parse_thread_count_)
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
                if (!append_mode && active_thread_count != user_config.parse_thread_count_)
                    std::this_thread::sleep_for(worker_offset_wait_ms);
            }
            else
            {
                spdlog::get("pm_logger")->debug("All threads ACTIVE ({:d} active)", active_thread_count);

                // Check active workers to see if they are ready to be joined.
                uint16_t current_active_worker = 0;
                for (uint16_t active_worker_ind = 0; active_worker_ind < active_workers_vec.size();
                     active_worker_ind++)
                {
                    current_active_worker = active_workers_vec[active_worker_ind];

                    // Join workers that are complete and start the worker associated
                    // with the current worker_ind
                    if (workers_vec[current_active_worker]->CompletionStatus() == true)
                    {
                        spdlog::get("pm_logger")->debug(
                            "WorkerQueue: worker {:d} "
                            "INACTIVE/COMPLETE -- joining now",
                            current_active_worker);

                        // Join the recently completed worker
                        threads_vec[current_active_worker].join();

                        // Clear the buffer
                        worker_config_vec[current_active_worker].bb_.Clear();

                        // Update the vector of active worker indices
                        active_workers_vec.erase(active_workers_vec.begin() + active_worker_ind);

                        if (!ActivateWorker(append_mode, worker_vec[worker_ind], threads_vec[worker_ind],
                                            worker_config_vec[worker_ind], worker_ind, worker_count, total_read_pos,
                                            read_size, append_read_size, total_size, &worker_config_vec[worker_ind].bb_,
                                            ch10_input_stream, actual_read_size, output_file_path_vec[worker_ind],
                                            packet_type_config_map, tmats_vec, context_vec_[worker_ind]))
                            return false;

                        // Place new worker among active workers.
                        active_workers_vec.push_back(worker_ind);

                        thread_started = true;
                        break;
                    }
                    else
                    {
                        spdlog::get("pm_logger")->debug("WorkerQueue: worker {:d} STILL ACTIVE", current_active_worker);
                    }
                }
            }

            // Wait before checking for available workers.
            spdlog::get("pm_logger")->trace("WorkerQueue: waiting for workers");
            std::this_thread::sleep_for(worker_shift_wait_ms);

        }  // end while

        // Increase the total read position.
        total_read_pos += actual_read_size;

    }  // end for loop over all worker indices.
    return true;
}

bool ParseManager::WorkerRetireQueue(std::vector<std::unique_ptr<ParseWorker>>& worker_vec,
                                     std::vector<uint16_t>& active_workers_vec,
                                     std::vector<WorkerConfig>& worker_config_vec,
                                     std::vector<std::thread>& threads_vec,
                                     int worker_shift_wait)
{
    uint16_t worker_count = worker_vec.size();
    std::chrono::milliseconds worker_shift_wait_ms(worker_shift_wait);
    spdlog::get("pm_logger")->debug("WorkerRetireQueue: Joining all remaining workers");
    uint16_t current_active_worker = 0;

    while (active_workers_vec.size() > 0)
    {
        for (uint16_t active_worker_ind = 0; active_worker_ind < active_workers_vec.size();
             active_worker_ind++)
        {
            // Join workers that are complete
            current_active_worker = active_workers_vec[active_worker_ind];
            if (workers_vec[current_active_worker]->CompletionStatus() == true)
            {
                spdlog::get("pm_logger")->debug(
                    "WorkerRetireQueue: worker {:d} "
                    "INACTIVE/COMPLETE -- joining now",
                    current_active_worker);

                threads_vec[current_active_worker].join();

                // Clear the binary buffer to free memory
                worker_config_vec[current_active_worker].bb_.Clear();

                spdlog::get("pm_logger")->debug("WorkerRetireQueue: worker {:d} joined", current_active_worker);
                if (worker_count == 1)
                    active_workers_vec.resize(0);
                else
                    active_workers_vec.erase(active_workers_vec.begin() + active_worker_ind);
            }
            else
            {
                spdlog::get("pm_logger")->debug("WorkerRetireQueue: worker {:d} STILL ACTIVE", current_active_worker);
            }
        }

        // Wait before checking for available workers.
        spdlog::get("pm_logger")->debug("WorkerRetireQueue: waiting for workers to complete");
        std::this_thread::sleep_for(worker_shift_wait_ms);
    }

    spdlog::get("pm_logger")->debug("WorkerRetireQueue: all workers joined");
    return true;
}

ParseManager::~ParseManager()
{
    ch10_input_stream_.close();
}

void ParseManager::LogPacketTypeConfig(const std::map<Ch10PacketType, bool>& pkt_type_config_map)
{
    // Convert the Ch10PacketType to bool --> string to bool
    IterableTools iter_tools;
    std::map<std::string, bool> str_packet_type_map;
    for (std::map<Ch10PacketType, bool>::const_iterator it = pkt_type_config_map.cbegin();
         it != pkt_type_config_map.cend(); ++it)
    {
        str_packet_type_map[ch10packettype_to_string_map.at(it->first)] = it->second;
    }

    // Get string representation of key-value pairs
    std::string stringrepr = iter_tools.GetPrintableMapElements_KeyToBool(
        str_packet_type_map);

    // Log the map information.
    spdlog::get("pm_logger")->info("Ch10 packet configuration:");
    ParseText pt;
    std::vector<std::string> splitstr = pt.Split(stringrepr, '\n');
    for (std::vector<std::string>::const_iterator it = splitstr.cbegin();
         it != splitstr.cend(); ++it)
    {
        spdlog::get("pm_logger")->info("{:s}", *it);
    }
    spdlog::get("pm_logger")->flush();
}

void ParseManager::AssembleParsedPacketTypesSet(std::set<Ch10PacketType>& parsed_packet_types)
{
    std::set<Ch10PacketType> temp_parsed_packet_types;
    for (std::vector<std::unique_ptr<Ch10Context>>::const_iterator it = context_vec_.cbegin();
        it != context_vec_.cend(); ++it)
    {
        temp_parsed_packet_types = (*it)->parsed_packet_types;
        parsed_packet_types.insert(temp_parsed_packet_types.cbegin(), temp_parsed_packet_types.cend());
    }

    // Log the parsed packet types
    spdlog::get("pm_logger")->info("Parsed packet types:");
    for (std::set<Ch10PacketType>::const_iterator it = parsed_packet_types.cbegin();
        it != parsed_packet_types.cend(); ++it)
    {
        spdlog::get("pm_logger")->info(" - {:s}", ch10packettype_to_string_map.at(*it));
    }
    spdlog::get("pm_logger")->flush();
}

bool ParseManager::WriteTDPData(const std::vector<const Ch10Context*>& ctx_vec,
    ParquetTDPF1* pqtdp, const ManagedPath& file_path)
{
    if(!pqtdp->Initialize(file_path, 0))
    {
        spdlog::get("pm_logger")->error("Failed to initialize writer for time data, format 1 for "
            "output file: {:s}", file_path.RawString());
        return false;
    }

    for(std::vector<const Ch10Context*>::const_iterator it = ctx_vec.cbegin(); 
        it != ctx_vec.cend(); ++it)
    {
        const std::vector<TDF1CSDWFmt>& tdcsdw_vec = (*it)->tdf1csdw_vec;
        const std::vector<uint64_t>& time_vec = (*it)->tdp_abs_time_vec;
        for(size_t i = 0; i < time_vec.size(); i++)
        {
            pqtdp->Append(time_vec.at(i), tdcsdw_vec.at(i));
        }
    }
    uint16_t thread_id = 0;
    pqtdp->Close(thread_id);

    return true;
}
