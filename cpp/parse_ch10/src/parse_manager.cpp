// parse_manager.cpp

#include "parse_manager.h"

ParseManager::ParseManager() : worker_count_(0), worker_count(worker_count_), worker_chunk_size_bytes_(0), worker_chunk_size_bytes(worker_chunk_size_bytes_), workers_vec(workers_vec_), threads_vec(threads_vec_), worker_config_vec(worker_config_vec_), ch10_input_stream_(), append_chunk_size_bytes_(100000000), it_(), tmats_output_path_(""), tdp_output_path_("")
{
}

bool ParseManager::Configure(ManagedPath input_ch10_file_path, ManagedPath output_dir,
                             const ParserConfigParams& user_config)
{
    bool success = false;
    input_ch10_file_path.GetFileSize(success, ch10_file_size_);
    if (!success)
        return false;
    spdlog::get("pm_logger")->info("Ch10 file size: {:f} MB", ch10_file_size_ / (1000.0 * 1000.0));

    // Convert ch10_packet_type configuration map from string --> string to
    // Ch10PacketType --> bool
    std::map<Ch10PacketType, bool> packet_type_config_map;
    if(user_config.ch10_packet_enabled_map_.size() == 0)
    {
        if (!ConvertCh10PacketTypeMap(user_config.ch10_packet_type_map_, packet_type_config_map))
            return false;
    }
    else
        packet_type_config_map = user_config.ch10_packet_enabled_map_;

    tmats_output_path_ = output_dir.CreatePathObject(input_ch10_file_path,
        "_TMATS.txt");
    tdp_output_path_ = output_dir.CreatePathObject(input_ch10_file_path,
        "_time_data.parquet");
    spdlog::get("pm_logger")->info("TMATS output path: {:s}", tmats_output_path_.RawString());
    spdlog::get("pm_logger")->info("Time data output path: {:s}", tdp_output_path_.RawString());

    // Hard-code packet type directory extensions now. Later, import from
    // config yaml.
    std::map<Ch10PacketType, std::string> append_str_map = {
        {Ch10PacketType::MILSTD1553_F1, "_1553.parquet"},
        {Ch10PacketType::VIDEO_DATA_F0, "_video.parquet"},
        {Ch10PacketType::ETHERNET_DATA_F0, "_ethernet.parquet"},
        {Ch10PacketType::ARINC429_F0, "_arinc429.parquet"}};

    if (!CreateCh10PacketOutputDirs(output_dir, input_ch10_file_path,
                                    packet_type_config_map, append_str_map, output_dir_map_, true))
        return false;

    if (!AllocateResources(user_config, ch10_file_size_))
        return false;

    // Create output file names. A map of Ch10PacketType to
    // ManagedPath will be created for each worker.
    CreateCh10PacketWorkerFileNames(worker_count_, output_dir_map_, output_file_path_vec_,
                                    "parquet");

    // Record the packet type config map in metadata and logs.
    LogPacketTypeConfig(packet_type_config_map);

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
	Note that ParserConfigParams has limits applied to the parameters so there
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
    // Convert ch10_packet_type configuration map from string --> string to
    // Ch10PacketType --> bool
    std::map<Ch10PacketType, bool> packet_type_config_map;
    if(user_config.ch10_packet_enabled_map_.size() == 0)
    {
        if (!ConvertCh10PacketTypeMap(user_config.ch10_packet_type_map_, packet_type_config_map))
            return false;
    }
    else
        packet_type_config_map = user_config.ch10_packet_enabled_map_;

    // Start queue to activate all workers, limiting the quantity of
    // concurrent threads to n_threads.
    bool append = false;
    uint16_t effective_worker_count = workers_vec_.size();
    std::vector<uint16_t> active_workers_vec;
    spdlog::get("pm_logger")->debug("Parse: begin parsing with workers");
    if (!WorkerQueue(append, ch10_input_stream_, workers_vec_, active_workers_vec,
                     worker_config_vec_, effective_worker_count, worker_chunk_size_bytes_,
                     append_chunk_size_bytes_, ch10_file_size_, output_file_path_vec_,
                     packet_type_config_map, threads_vec_, tmats_body_vec_, user_config))
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
                     append_chunk_size_bytes_, ch10_file_size_, output_file_path_vec_,
                     packet_type_config_map, threads_vec_, tmats_body_vec_, user_config))
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
                                  const ParserConfigParams& user_config,
                                  const ProvenanceData& prov_data)
{
    // Convert ch10_packet_type configuration map from string --> string to
    // Ch10PacketType --> bool
    std::map<Ch10PacketType, bool> packet_type_config_map;
    if(user_config.ch10_packet_enabled_map_.size() == 0)
    {
        if (!ConvertCh10PacketTypeMap(user_config.ch10_packet_type_map_, packet_type_config_map))
            return false;
    }
    else
        packet_type_config_map = user_config.ch10_packet_enabled_map_;

    // Create a set of all the parsed packet types
    std::set<Ch10PacketType> parsed_packet_types;
    AssembleParsedPacketTypesSet(parsed_packet_types);

    // Process TMATs matter and record
    TMATSData tmats_data;
    ProcessTMATS(tmats_body_vec_, tmats_output_path_, tmats_data, parsed_packet_types);
    spdlog::get("pm_logger")->debug("RecordMetadata: begin record metadata");

    std::string md_filename("_metadata.yaml");

    for (std::map<Ch10PacketType, bool>::const_iterator it = packet_type_config_map.cbegin();
         it != packet_type_config_map.cend(); ++it)
    {
        if (it->second)
        {
            switch (it->first)
            {
                case Ch10PacketType::MILSTD1553_F1:
                    if (parsed_packet_types.count(Ch10PacketType::MILSTD1553_F1) == 1)
                    {
                        if (!RecordMilStd1553F1Metadata(input_ch10_file_path,
                            user_config, prov_data, tmats_data,
                            ch10packettype_to_string_map.at(Ch10PacketType::MILSTD1553_F1),
                            output_dir_map_[Ch10PacketType::MILSTD1553_F1] / md_filename))
                            return false;
                    }
                    break;
                case Ch10PacketType::VIDEO_DATA_F0:
                    if (parsed_packet_types.count(Ch10PacketType::VIDEO_DATA_F0) == 1)
                    {
                        if (!RecordVideoDataF0Metadata(input_ch10_file_path,
                            user_config, prov_data, tmats_data,
                            ch10packettype_to_string_map.at(Ch10PacketType::VIDEO_DATA_F0),
                            output_dir_map_[Ch10PacketType::VIDEO_DATA_F0] / md_filename))
                            return false;
                    }
                    break;
                case Ch10PacketType::ARINC429_F0:
                    if (parsed_packet_types.count(Ch10PacketType::ARINC429_F0) == 1)
                    {
                        if (!RecordARINC429F0Metadata(input_ch10_file_path,
                            user_config, prov_data, tmats_data,
                            ch10packettype_to_string_map.at(Ch10PacketType::ARINC429_F0),
                            output_dir_map_[Ch10PacketType::ARINC429_F0] / md_filename))
                            return false;
                    }
                    break;
                default:
                    if (parsed_packet_types.count(it->first) == 1)
                    {
                        spdlog::get("pm_logger")->warn(
                            "RecordMetadata: No metadata output "
                            "function for packet type \"{:s}\"",
                            ch10packettype_to_string_map.at(it->first));
                    }
                    break;
            }  // end switch
        }      // end if(it->second)
    }          // end for loop
    spdlog::get("pm_logger")->debug("RecordMetadata: complete record metadata");

    RemoveCh10PacketOutputDirs(output_dir_map_, parsed_packet_types);

    spdlog::get("pm_logger")->debug("Record Time Data");
    ParquetContext pq_ctx;
    ParquetTDPF1 pq_tdp(&pq_ctx);
    std::vector<const Ch10Context*> ctx_vec;
    for(std::vector<std::unique_ptr<Ch10Context>>::iterator it = context_vec_.begin();
        it != context_vec_.end(); ++it)
    { ctx_vec.push_back((*it).get()); }
    WriteTDPData(ctx_vec, &pq_tdp, tdp_output_path_);

    return true;
}

bool ParseManager::RecordProvenanceData(TIPMDDocument& md,
    const ManagedPath& input_ch10_file_path, const std::string& packet_type_label,
    const ProvenanceData& prov_data)
{
    md.type_category_->SetScalarValue("parsed_" + packet_type_label);

    std::string ch10_hash = prov_data.hash;
    std::string uid = Sha256(ch10_hash + prov_data.time +
        prov_data.tip_version + packet_type_label);
    md.uid_category_->SetScalarValue(uid);
    md.AddResource("CH10", input_ch10_file_path.RawString(), ch10_hash);

    if(!md.prov_category_->SetMappedValue("time", prov_data.time))
        return false;
    if(!md.prov_category_->SetMappedValue("version", prov_data.tip_version))
        return false;
    return true;
}

void ParseManager::RecordUserConfigData(std::shared_ptr<MDCategoryMap> config_category,
    const ParserConfigParams& user_config)
{
    config_category->SetArbitraryMappedValue("ch10_packet_type",
        user_config.ch10_packet_type_map_);
    config_category->SetArbitraryMappedValue("parse_chunk_bytes",
        user_config.parse_chunk_bytes_);
    config_category->SetArbitraryMappedValue("parse_thread_count",
        user_config.parse_thread_count_);
    config_category->SetArbitraryMappedValue("max_chunk_read_count",
        user_config.max_chunk_read_count_);
    config_category->SetArbitraryMappedValue("worker_offset_wait_ms",
        user_config.worker_offset_wait_ms_);
    config_category->SetArbitraryMappedValue("worker_shift_wait_ms",
        user_config.worker_shift_wait_ms_);
    config_category->SetArbitraryMappedValue("stdout_log_level",
        user_config.stdout_log_level_);
}

bool ParseManager::ProcessTMATSForType(const TMATSData& tmats_data, TIPMDDocument& md,
		Ch10PacketType pkt_type)
{
    // Filter TMATS maps
    std::map<std::string, std::string> tmats_chanid_to_type_filtered;
    if(!tmats_data.FilterTMATSType(tmats_data.chanid_to_type_map,
        pkt_type, tmats_chanid_to_type_filtered))
    {
        spdlog::get("pm_logger")->error("Failed to filter TMATS for type \"{:s}\"",
            ch10packettype_to_string_map.at(pkt_type));
        return false;
    }
    std::map<std::string, std::string> tmats_chanid_to_source_filtered;
    tmats_chanid_to_source_filtered = tmats_data.FilterByChannelIDToType(
        tmats_chanid_to_type_filtered, tmats_data.chanid_to_source_map);

    // Record the TMATS channel ID to source map.
    md.runtime_category_->SetArbitraryMappedValue("tmats_chanid_to_source",
        tmats_chanid_to_source_filtered);

    // Record the TMATS channel ID to type map.
    md.runtime_category_->SetArbitraryMappedValue("tmats_chanid_to_type",
        tmats_chanid_to_type_filtered);

    return true;
}

bool ParseManager::RecordMilStd1553F1Metadata(ManagedPath input_ch10_file_path,
                                              const ParserConfigParams& user_config,
                                              const ProvenanceData& prov_data,
                                              const TMATSData& tmats_data,
                                              const std::string& packet_type_label,
                                              const ManagedPath& md_file_path)
{
    spdlog::get("pm_logger")->debug("RecordMetadata: recording {:s} metadata",
        packet_type_label);

    TIPMDDocument md;
    if(!RecordProvenanceData(md, input_ch10_file_path, packet_type_label, prov_data))
        return false;
    RecordUserConfigData(md.config_category_, user_config);

    // Obtain the tx and rx combined channel ID to LRU address map and
    // record it to the Yaml writer. First compile all the channel ID to
    // LRU address maps from the workers.
    std::vector<std::map<uint32_t, std::set<uint16_t>>> chanid_lruaddr1_maps;
    std::vector<std::map<uint32_t, std::set<uint16_t>>> chanid_lruaddr2_maps;
    for (uint16_t worker_ind = 0; worker_ind < worker_count_; worker_ind++)
    {
        chanid_lruaddr1_maps.push_back(context_vec_[worker_ind]->chanid_remoteaddr1_map);
        chanid_lruaddr2_maps.push_back(context_vec_[worker_ind]->chanid_remoteaddr2_map);
    }
    std::map<uint32_t, std::set<uint16_t>> output_chanid_remoteaddr_map;
    if (!CombineChannelIDToLRUAddressesMetadata(output_chanid_remoteaddr_map,
                                                chanid_lruaddr1_maps, chanid_lruaddr2_maps))
        return false;
    md.runtime_category_->SetArbitraryMappedValue("chanid_to_lru_addrs",
        output_chanid_remoteaddr_map);

    // Obtain the channel ID to command words set map.
    std::vector<std::map<uint32_t, std::set<uint32_t>>> chanid_commwords_maps;
    for (uint16_t worker_ind = 0; worker_ind < worker_count_; worker_ind++)
        chanid_commwords_maps.push_back(context_vec_[worker_ind]->chanid_commwords_map);

    std::map<uint32_t, std::vector<std::vector<uint32_t>>> output_chanid_commwords_map;
    if (!CombineChannelIDToCommandWordsMetadata(output_chanid_commwords_map,
                                                chanid_commwords_maps))
        return false;
    md.runtime_category_->SetArbitraryMappedValue("chanid_to_comm_words",
        output_chanid_commwords_map);

    if(!ProcessTMATSForType(tmats_data, md, Ch10PacketType::MILSTD1553_F1))
        return false;

    // Write the complete Yaml record to the metadata file.
    md.CreateDocument();
    std::ofstream stream_1553_metadata(md_file_path.string(),
                                       std::ofstream::out | std::ofstream::trunc);
    stream_1553_metadata << md.GetMetadataString();
    stream_1553_metadata.close();
    return true;
}

bool ParseManager::RecordVideoDataF0Metadata(ManagedPath input_ch10_file_path,
                                             const ParserConfigParams& user_config,
                                             const ProvenanceData& prov_data,
                                             const TMATSData& tmats_data,
                                             const std::string& packet_type_label,
                                             const ManagedPath& md_file_path)

{
    spdlog::get("pm_logger")->debug("RecordMetadata: recording {:s} metadata", ch10packettype_to_string_map.at(Ch10PacketType::VIDEO_DATA_F0));

    TIPMDDocument md;
    if(!RecordProvenanceData(md, input_ch10_file_path, packet_type_label, prov_data))
        return false;
    RecordUserConfigData(md.config_category_, user_config);

    // Get the channel ID to minimum time stamp map.
    std::vector<std::map<uint16_t, uint64_t>> worker_chanid_to_mintimestamps_maps;
    for (uint16_t worker_ind = 0; worker_ind < worker_count_; worker_ind++)
    {
        worker_chanid_to_mintimestamps_maps.push_back(context_vec_[worker_ind]->chanid_minvideotimestamp_map);
    }
    std::map<uint16_t, uint64_t> output_min_timestamp_map;
    CreateChannelIDToMinVideoTimestampsMetadata(output_min_timestamp_map,
                                                worker_chanid_to_mintimestamps_maps);

    // Record the map in the Yaml writer and write the
    // total yaml text to file.
    md.runtime_category_->SetArbitraryMappedValue("chanid_to_first_timestamp",
        output_min_timestamp_map);

    if(!ProcessTMATSForType(tmats_data, md, Ch10PacketType::VIDEO_DATA_F0))
        return false;

    md.CreateDocument();
    std::ofstream stream_video_metadata(md_file_path.string(),
                                        std::ofstream::out | std::ofstream::trunc);
    stream_video_metadata << md.GetMetadataString();
    stream_video_metadata.close();
    return true;
}

bool ParseManager::RecordARINC429F0Metadata(ManagedPath input_ch10_file_path,
                                   const ParserConfigParams& user_config,
								   const ProvenanceData& prov_data,
                                   const TMATSData& tmats_data,
								   const std::string& packet_type_label,
								   const ManagedPath& md_file_path)
{
    spdlog::get("pm_logger")->debug("RecordMetadata: recording {:s} metadata", ch10packettype_to_string_map.at(Ch10PacketType::ARINC429_F0));

    TIPMDDocument md;
    if(!RecordProvenanceData(md, input_ch10_file_path, packet_type_label, prov_data))
        return false;
    RecordUserConfigData(md.config_category_, user_config);

    // Obtain the channel ID to 429 label set map.
    std::vector<std::map<uint32_t, std::set<uint16_t>>> chanid_label_maps;
    for (uint16_t worker_ind = 0; worker_ind < worker_count_; worker_ind++)
        chanid_label_maps.push_back(context_vec_[worker_ind]->chanid_labels_map);

    std::map<uint32_t, std::set<uint16_t>> output_chanid_label_map;
    if (!CombineChannelIDToLabelsMetadata(output_chanid_label_map,
                                                chanid_label_maps))
        return false;
    md.runtime_category_->SetArbitraryMappedValue("chanid_to_labels",
        output_chanid_label_map);

    // Obtain the channel ID to ARINC message header bus number set map.
    std::vector<std::map<uint32_t, std::set<uint16_t>>> chanid_busnumber_maps;
    for (uint16_t worker_ind = 0; worker_ind < worker_count_; worker_ind++)
        chanid_busnumber_maps.push_back(context_vec_[worker_ind]->chanid_busnumbers_map);

    std::map<uint32_t, std::set<uint16_t>> output_chanid_busnumber_map;
    if (!CombineChannelIDToBusNumbersMetadata(output_chanid_busnumber_map,
                                                chanid_busnumber_maps))
        return false;
    md.runtime_category_->SetArbitraryMappedValue("chanid_to_bus_numbers",
        output_chanid_busnumber_map);

    if(!ProcessTMATSForType(tmats_data, md, Ch10PacketType::ARINC429_F0))
        return false;

    // Record ARINC429-specific TMATS data
    md.runtime_category_->SetArbitraryMappedValue("tmats_chanid_to_429_format",
        tmats_data.chanid_to_429_format);
    md.runtime_category_->SetArbitraryMappedValue("tmats_chanid_to_429_subchans",
        tmats_data.chanid_to_429_subchans);
    md.runtime_category_->SetArbitraryMappedValue("tmats_chanid_to_429_subchan_and_name",
        tmats_data.chanid_to_429_subchan_and_name);

    // Write the complete Yaml record to the metadata file.
    std::ofstream stream_429_metadata(md_file_path.string(),
                                       std::ofstream::out | std::ofstream::trunc);
    md.CreateDocument();
    stream_429_metadata << md.GetMetadataString();
    stream_429_metadata.close();
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

void ParseManager::ProcessTMATS(const std::vector<std::string>& tmats_vec,
                                const ManagedPath& tmats_file_path,
                                TMATSData& tmats_data,
                                const std::set<Ch10PacketType>& parsed_pkt_types)
{
    // if tmats doesn't exist return
    if (tmats_vec.size() == 0)
    {
        spdlog::get("pm_logger")->warn("ProcessTMATS: no TMATS Present");
        return;
    }

    std::string full_TMATS_string;
    for (int i = 0; i < tmats_vec.size(); i++)
    {
        full_TMATS_string += tmats_vec[i];
    }

    std::ofstream tmats;
    tmats.open(tmats_file_path.string(), std::ios::trunc | std::ios::binary);
    if (tmats.good())
    {
        spdlog::get("pm_logger")->info("ProcessTMATS: writing TMATS to {:s}", tmats_file_path.RawString());
        tmats << full_TMATS_string;
    }

    tmats.close();

    // Gather TMATs attributes of interest
    // for metadata
    if(!tmats_data.Parse(full_TMATS_string, parsed_pkt_types))
    {
        spdlog::get("pm_logger")->info("ProcessTMATS:: Failed to parse TMATS");
    }
}

bool ParseManager::ConvertCh10PacketTypeMap(const std::map<std::string, std::string>& input_map,
                                            std::map<Ch10PacketType, bool>& output_map)
{
    // The ch10 can't be parsed if there is no packet type configuration.
    // If the input_map is empty, return false.
    if (input_map.size() == 0)
    {
        spdlog::get("pm_logger")->info("ConvertCh10PacketTypeMap: input_map is empty");
        return false;
    }

    // Define string to Ch10PacketType map
    std::map<std::string, Ch10PacketType> conversion_map = {
        {"MILSTD1553_FORMAT1", Ch10PacketType::MILSTD1553_F1},
        {"VIDEO_FORMAT0", Ch10PacketType::VIDEO_DATA_F0},
        {"ETHERNET_DATA0", Ch10PacketType::ETHERNET_DATA_F0},
        {"ARINC429_FORMAT0", Ch10PacketType::ARINC429_F0}};

    ParseText pt;
    std::string bool_string;
    for (std::map<std::string, std::string>::const_iterator it = input_map.cbegin();
         it != input_map.cend(); ++it)
    {
        // If the name of the packet type from the conversion map is not present,
        // clear the output map and return false.
        if (conversion_map.count(it->first) == 0)
        {
            output_map.clear();
            spdlog::get("pm_logger")->warn(
                "ConvertCh10PacketTypeMap: ch10_packet_type "
                "configuration key \"{:s}\" not in conversion_map",
                it->first);
            return false;
        }

        // Check for valid boolean string
        bool_string = pt.ToLower(it->second);
        if (bool_string == "true")
            output_map[conversion_map.at(it->first)] = true;
        else if (bool_string == "false")
            output_map[conversion_map.at(it->first)] = false;
        else
        {
            output_map.clear();
            spdlog::get("pm_logger")->warn(
                "ch10_packet_type configuration boolean "
                "string {:s} not valid, must spell \"true\" or \"false\", upper or "
                "lower chars",
                it->second);
            return false;
        }
    }
    return true;
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

bool ParseManager::CreateCh10PacketOutputDirs(const ManagedPath& output_dir,
                                              const ManagedPath& base_file_name,
                                              const std::map<Ch10PacketType, bool>& packet_enabled_map,
                                              const std::map<Ch10PacketType, std::string>& append_str_map,
                                              std::map<Ch10PacketType, ManagedPath>& pkt_type_output_dir_map, bool create_dir)
{
    // Check for present of append_str_map entry for each packet_enabled_map
    // entry which maps to true.
    std::map<Ch10PacketType, bool>::const_iterator it;
    bool result = false;
    ManagedPath pkt_type_output_dir;
    for (it = packet_enabled_map.cbegin(); it != packet_enabled_map.cend(); ++it)
    {
        if (it->second)
        {
            if (append_str_map.count(it->first) == 0)
            {
                spdlog::get("pm_logger")->warn(
                    "CreateCh10PacketOutputDirs: No append "
                    "string map entry for {:s}",
                    ch10packettype_to_string_map.at(it->first));
                return false;
            }

            // Fill pkt_type_output_dir_map for each packet type in packet_enabled_map.
            result = ManagedPath::CreateDirectoryFromComponents(output_dir, base_file_name,
                                                                append_str_map.at(it->first), pkt_type_output_dir, create_dir);
            if (!result)
            {
                pkt_type_output_dir_map.clear();
                return false;
            }
            spdlog::get("pm_logger")->info("Create {:s} output dir: {:s}", ch10packettype_to_string_map.at(it->first), pkt_type_output_dir.RawString());
            pkt_type_output_dir_map[it->first] = pkt_type_output_dir;
        }
    }

    return true;
}

bool ParseManager::RemoveCh10PacketOutputDirs(const std::map<Ch10PacketType, ManagedPath>& output_dir_map,
    const std::set<Ch10PacketType>& parsed_packet_types)
{
    std::string packet_type_name = "";
    bool retval = true;
    for (std::map<Ch10PacketType, ManagedPath>::const_iterator it = output_dir_map.cbegin();
        it != output_dir_map.cend(); ++it)
    {
        packet_type_name = ch10packettype_to_string_map.at(it->first);
        if (it->second.is_directory())
        {
            if(parsed_packet_types.count(it->first) == 0)
            {
                spdlog::get("pm_logger")->info("Removing unused {:s} dir: {:s}",
                    packet_type_name, it->second.RawString());
                if(!it->second.remove())
                {
                    spdlog::get("pm_logger")->warn("Failed to remove {:s} dir: {:s}",
                        packet_type_name, it->second.RawString());
                    retval = false;
                }
            }
        }
        else
        {
            spdlog::get("pm_logger")->warn("Expected output {:s} dir does not exist: {:s}",
                packet_type_name, it->second.RawString());
        }
    }
    return retval;
}


void ParseManager::CreateCh10PacketWorkerFileNames(const uint16_t& total_worker_count,
                                                   const std::map<Ch10PacketType, ManagedPath>& pkt_type_output_dir_map,
                                                   std::vector<std::map<Ch10PacketType, ManagedPath>>& output_vec_mapped_paths,
                                                   std::string file_extension)
{
    std::string replacement_ext = "";
    std::map<Ch10PacketType, ManagedPath>::const_iterator it;

    for (uint16_t worker_index = 0; worker_index < total_worker_count; worker_index++)
    {
        // Create the replacement extension for the current index.
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(3) << worker_index;
        if (file_extension != "")
        {
            ss << "." << file_extension;
        }
        replacement_ext = ss.str();

        // Create a temporary map to hold all of the output file paths for the
        // current index.
        std::map<Ch10PacketType, ManagedPath> temp_output_file_map;

        // Add an output file path for each Ch10PacketType and output dir
        // in pkt_type_output_dir_map.
        for (it = pkt_type_output_dir_map.cbegin(); it != pkt_type_output_dir_map.cend(); ++it)
        {
            //temp_output_file_map[it->first] = it->second.CreatePathObject(
            //    it->second, replacement_ext);
            temp_output_file_map[it->first] = it->second / replacement_ext;
        }

        // Add the temp map to the vector maps if the temp map has items.
        if (temp_output_file_map.size() > 0)
            output_vec_mapped_paths.push_back(temp_output_file_map);
    }
}

void ParseManager::CreateChannelIDToMinVideoTimestampsMetadata(
    std::map<uint16_t, uint64_t>& output_chanid_to_mintimestamp_map,
    const std::vector<std::map<uint16_t, uint64_t>>& chanid_mintimestamp_maps)
{
    // Gather the maps from each worker and combine them into one,
    //keeping only the lowest time stamps for each channel ID.
    for (size_t i = 0; i < chanid_mintimestamp_maps.size(); i++)
    {
        std::map<uint16_t, uint64_t> temp_map = chanid_mintimestamp_maps.at(i);
        for (std::map<uint16_t, uint64_t>::const_iterator it = temp_map.begin();
             it != temp_map.end(); ++it)
        {
            if (output_chanid_to_mintimestamp_map.count(it->first) == 0)
                output_chanid_to_mintimestamp_map[it->first] = it->second;
            else if (it->second < output_chanid_to_mintimestamp_map[it->first])
                output_chanid_to_mintimestamp_map[it->first] = it->second;
        }
    }
}

bool ParseManager::CombineChannelIDToLRUAddressesMetadata(
    std::map<uint32_t, std::set<uint16_t>>& output_chanid_lruaddr_map,
    const std::vector<std::map<uint32_t, std::set<uint16_t>>>& chanid_lruaddr1_maps,
    const std::vector<std::map<uint32_t, std::set<uint16_t>>>& chanid_lruaddr2_maps)
{
    // Input vectors must have the same length.
    if (chanid_lruaddr1_maps.size() != chanid_lruaddr2_maps.size())
    {
        spdlog::get("pm_logger")->warn(
            "CombineChannelIDToLRUAddressesMetadata: "
            "Input vectors are not the same size, chanid_lruaddr1_maps ({:d}) "
            "chanid_lruaddr2_maps ({:d})",
            chanid_lruaddr1_maps.size(), chanid_lruaddr2_maps.size());
        return false;
    }

    // Collect and combine the channel ID to LRU address maps
    // assembled by each worker.
    std::map<uint32_t, std::set<uint16_t>> chanid_remoteaddr_map1;
    std::map<uint32_t, std::set<uint16_t>> chanid_remoteaddr_map2;
    for (size_t i = 0; i < chanid_lruaddr1_maps.size(); i++)
    {
        //workers[read_ind].append_chanid_remoteaddr_maps(chanid_remoteaddr_map1, chanid_remoteaddr_map2);
        chanid_remoteaddr_map1 = it_.CombineCompoundMapsToSet(
            chanid_remoteaddr_map1, chanid_lruaddr1_maps.at(i));
        chanid_remoteaddr_map2 = it_.CombineCompoundMapsToSet(
            chanid_remoteaddr_map2, chanid_lruaddr2_maps.at(i));
    }

    // Combine the tx and rx maps into a single map.
    output_chanid_lruaddr_map = it_.CombineCompoundMapsToSet(
        chanid_remoteaddr_map1, chanid_remoteaddr_map2);

    return true;
}

bool ParseManager::CombineChannelIDToCommandWordsMetadata(
    std::map<uint32_t, std::vector<std::vector<uint32_t>>>& output_chanid_commwords_map,
    const std::vector<std::map<uint32_t, std::set<uint32_t>>>& chanid_commwords_maps)
{
    // Collect maps into one.
    std::map<uint32_t, std::set<uint32_t>> chanid_commwords_map;
    for (size_t i = 0; i < chanid_commwords_maps.size(); i++)
    {
        chanid_commwords_map = it_.CombineCompoundMapsToSet(chanid_commwords_map,
                                                            chanid_commwords_maps.at(i));
    }

    // Break compound command words each into a set of two command words,
    // a transmit and receive value.
    uint32_t mask_val = (1 << 16) - 1;
    for (std::map<uint32_t, std::set<uint32_t>>::const_iterator it = chanid_commwords_map.cbegin();
         it != chanid_commwords_map.cend(); ++it)
    {
        std::vector<std::vector<uint32_t>> temp_vec_of_vec;
        for (std::set<uint32_t>::const_iterator it2 = it->second.cbegin();
             it2 != it->second.cend(); ++it2)
        {
            // Vector needed here to retain order.
            std::vector<uint32_t> pair_vec = {*it2 >> 16, *it2 & mask_val};
            temp_vec_of_vec.push_back(pair_vec);
        }
        output_chanid_commwords_map[it->first] = temp_vec_of_vec;
    }
    return true;
}

bool ParseManager::CombineChannelIDToLabelsMetadata(
    std::map<uint32_t, std::set<uint16_t>>& output_chanid_labels_map,
        const std::vector<std::map<uint32_t, std::set<uint16_t>>>& chanid_labels_maps)
{
    // Collect maps into one.
    std::map<uint32_t, std::set<uint16_t>> chanid_labels_map;
    for (size_t i = 0; i < chanid_labels_maps.size(); i++)
    {
        chanid_labels_map = it_.CombineCompoundMapsToSet(chanid_labels_map,
                                                            chanid_labels_maps.at(i));
    }

    // iterate chanid_labels_map and add to output_chanid_labels_map
    output_chanid_labels_map = chanid_labels_map;

    return true;
}

bool ParseManager::CombineChannelIDToBusNumbersMetadata(
    std::map<uint32_t, std::set<uint16_t>>&  output_chanid_busnumbers_map,
        const std::vector<std::map<uint32_t, std::set<uint16_t>>>& chanid_busnumbers_maps)
{
    // Collect maps into one.
    std::map<uint32_t, std::set<uint16_t>> chanid_busnumbers_map;
    for (size_t i = 0; i < chanid_busnumbers_maps.size(); i++)
    {
        chanid_busnumbers_map = it_.CombineCompoundMapsToSet(chanid_busnumbers_map,
                                                            chanid_busnumbers_maps.at(i));
    }

    // iterate chanid_busnumbers_map and add to output_chanid_busnumbers_map
    output_chanid_busnumbers_map = chanid_busnumbers_map;

    return true;
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
