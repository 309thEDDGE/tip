#include "parser_metadata.h"

ParserMetadata::ParserMetadata() : ch10_hash_byte_count_(150e6), prov_data_(), config_(),
    parser_paths_()
{}

bool ParserMetadata::Initialize(const ManagedPath& ch10_path, const ParserConfigParams& config,
    const ParserPaths& parser_paths)
{
    config_ = config;
    parser_paths_ = parser_paths;
    
    if(!GetProvenanceData(ch10_path.absolute(), ch10_hash_byte_count_, prov_data_))
        return false;
    spdlog::get("pm_logger")->info("Ch10 hash: {:s}", prov_data_.hash);

    // Record the packet type config map in metadata and logs.
    ParserMetadataFunctions funcs;
    funcs.LogPacketTypeConfig(config.ch10_packet_enabled_map_);

    return true;
}

int ParserMetadata::RecordMetadata(ManagedPath md_filename, 
    const std::vector<const Ch10Context*>& context_vec)
{
    int retcode = 0;
    ParserMetadataFunctions funcs;

    // Create a set of all the parsed packet types
    std::set<Ch10PacketType> parsed_pkt_types;
    funcs.AssembleParsedPacketTypesSet(context_vec, parsed_pkt_types);

    std::vector<std::string> tmats_body_vec;
    funcs.GatherTMATSData(context_vec, tmats_body_vec);

    // Process TMATs matter and record
    TMATSData tmats_data;
    funcs.ProcessTMATS(tmats_body_vec, parser_paths_.GetTMATSOutputPath(), 
        tmats_data, parsed_pkt_types);
    spdlog::get("pm_logger")->debug("RecordMetadata: begin record metadata");

    for (std::map<Ch10PacketType, bool>::const_iterator it = 
        parser_paths_.GetCh10PacketTypeEnabledMap().cbegin();
         it != parser_paths_.GetCh10PacketTypeEnabledMap().cend(); ++it)
    {
        if (it->second && (parsed_pkt_types.count(it->first) == 1))
        {
            TIPMDDocument tip_md;
            if((retcode = RecordMetadataForPktType(md_filename, it->first, &parser_paths_, 
                config_, prov_data_, &tmats_data, context_vec, &tip_md, &funcs)) != 0)
            {
               spdlog::get("pm_logger")->error("RecordMetadata: Failed for type {:s}",
                ch10packettype_to_string_map.at(it->first));
                return retcode;
            }
        }
    }

    // write tdpdata
    spdlog::get("pm_logger")->debug("Recording Time Data");
    ParquetContext pq_ctx;
    ParquetTDPF1 pq_tdp(&pq_ctx);
    if((retcode = funcs.WriteTDPData(context_vec, &pq_tdp, parser_paths_.GetTDPOutputPath())) != 0)
        return retcode;

    parser_paths_.RemoveCh10PacketOutputDirs(parsed_pkt_types);

    spdlog::get("pm_logger")->debug("RecordMetadata: complete record metadata");
    return 0;
}

void ParserMetadataFunctions::RecordProvenanceData(TIPMDDocument* md,
    const ManagedPath& input_ch10_file_path, const std::string& packet_type_label,
    const ProvenanceData& prov_data)
{
    md->type_category_->SetScalarValue("parsed_" + packet_type_label);

    std::string ch10_hash = prov_data.hash;
    std::string uid = CalcSHA256(ch10_hash + prov_data.time +
        prov_data.tip_version + packet_type_label);
    md->uid_category_->SetScalarValue(uid);
    md->AddResource("CH10", input_ch10_file_path.RawString(), ch10_hash);

    md->prov_category_->SetMappedValue("time", prov_data.time);
    md->prov_category_->SetMappedValue("version", prov_data.tip_version);
}

void ParserMetadataFunctions::RecordUserConfigData(std::shared_ptr<MDCategoryMap> config_category,
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

void ParserMetadataFunctions::ProcessTMATS(const std::vector<std::string>& tmats_vec,
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

bool ParserMetadataFunctions::ProcessTMATSForType(const TMATSData* tmats_data, TIPMDDocument* md,
		Ch10PacketType pkt_type)
{
    // Filter TMATS maps
    std::map<std::string, std::string> tmats_chanid_to_type_filtered;
    if(!tmats_data->FilterTMATSType(tmats_data->GetChannelIDToTypeMap(),
        pkt_type, tmats_chanid_to_type_filtered))
    {
        spdlog::get("pm_logger")->error("Failed to filter TMATS for type \"{:s}\"",
            ch10packettype_to_string_map.at(pkt_type));
        return false;
    }
    std::map<std::string, std::string> tmats_chanid_to_source_filtered;
    tmats_chanid_to_source_filtered = tmats_data->FilterByChannelIDToType(
        tmats_chanid_to_type_filtered, tmats_data->GetChannelIDToSourceMap());

    // Record the TMATS channel ID to source map.
    md->runtime_category_->SetArbitraryMappedValue("tmats_chanid_to_source",
        tmats_chanid_to_source_filtered);

    // Record the TMATS channel ID to type map.
    md->runtime_category_->SetArbitraryMappedValue("tmats_chanid_to_type",
        tmats_chanid_to_type_filtered);

    return true;
}

bool ParserMetadataFunctions::WriteStringToFile(const ManagedPath& outpath, const std::string& outdata)
{
    
    if(!outpath.parent_path().is_directory())
    {
        spdlog::get("pm_logger")->error("Failed to write file: {:s}, parent path "
        "is not a directory", outpath.RawString().c_str());
        return false;
    }

    std::ofstream stream(outpath.string(), std::ofstream::out | std::ofstream::trunc);
    if(!stream.is_open())
    {
        spdlog::get("pm_logger")->error("Failed to open output stream for file: {:s}", 
            outpath.RawString().c_str());
        return false;
    }

    stream << outdata;
    stream.close();

    return true;
}

bool ParserMetadataFunctions::RecordCh10PktTypeSpecificMetadata(Ch10PacketType pkt_type, 
    const std::vector<const Ch10Context*>& context_vec, MDCategoryMap* runtime_metadata, 
    const TMATSData* tmats, Ch10PacketTypeSpecificMetadata* spec_md)
{

    switch(pkt_type)
    {
        case Ch10PacketType::MILSTD1553_F1:
        {
            return spec_md->RecordMilStd1553F1SpecificMetadata(context_vec,
                runtime_metadata);
        }
        case Ch10PacketType::VIDEO_DATA_F0:
        {
            return spec_md->RecordVideoDataF0SpecificMetadata(context_vec,
                runtime_metadata);
        }
        case Ch10PacketType::ARINC429_F0:
        {
            return spec_md->RecordARINC429F0SpecificMetadata(context_vec,
                runtime_metadata, tmats);
        }
        default:
        {
            spdlog::get("pm_logger")->debug("RecordCh10PktTypeSpecificMetata: "
                "No handler for type: {:s}", ch10packettype_to_string_map.at(pkt_type));
            return true;
        }

    }
    return true;
}

int ParserMetadataFunctions::WriteTDPData(const std::vector<const Ch10Context*>& ctx_vec,
    ParquetTDPF1* pqtdp, const ManagedPath& file_path)
{
    int retcode = 0;
    if((retcode = pqtdp->Initialize(file_path, 0)) != 0)
    {
        spdlog::get("pm_logger")->error("Failed to initialize writer for time data, format 1 for "
            "output file: {:s}", file_path.RawString());
        return retcode;
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

    return 0;
}

void ParserMetadataFunctions::LogPacketTypeConfig(const std::map<Ch10PacketType, bool>& pkt_type_config_map)
{
    // Convert the Ch10PacketType to bool --> string to bool
    std::map<std::string, bool> str_packet_type_map;
    for (std::map<Ch10PacketType, bool>::const_iterator it = pkt_type_config_map.cbegin();
         it != pkt_type_config_map.cend(); ++it)
    {
        if(ch10packettype_to_string_map.count(it->first) == 1)
            str_packet_type_map[ch10packettype_to_string_map.at(it->first)] = it->second;
        else
        {
            spdlog::get("pm_logger")->warn("Ch10PacketType with integer value {:d} not in packet type "
                "to string map", static_cast<uint8_t>(it->first));
        }
    }

    IterableTools iter_tools;
    ParseText parse_text;

    // Get string representation of key-value pairs
    std::string stringrepr = iter_tools.GetPrintableMapElements_KeyToBool(
        str_packet_type_map);

    // Log the map information.
    spdlog::get("pm_logger")->info("Ch10 packet configuration:");
    std::vector<std::string> splitstr = parse_text.Split(stringrepr, '\n');
    for (std::vector<std::string>::const_iterator it = splitstr.cbegin();
         it != splitstr.cend(); ++it)
    {
        spdlog::get("pm_logger")->info("{:s}", *it);
    }
    spdlog::get("pm_logger")->flush();
}

void ParserMetadataFunctions::AssembleParsedPacketTypesSet(const std::vector<const Ch10Context*>& ctx_vec,
    std::set<Ch10PacketType>& parsed_packet_types)
{
    std::set<Ch10PacketType> temp_parsed_packet_types;
    for (std::vector<const Ch10Context*>::const_iterator it = ctx_vec.cbegin();
        it != ctx_vec.cend(); ++it)
    {
        temp_parsed_packet_types = (*it)->GetParsedPacketTypes();
        parsed_packet_types.insert(temp_parsed_packet_types.cbegin(), temp_parsed_packet_types.cend());
    }

    // Log the parsed packet types
    spdlog::get("pm_logger")->info("Parsed packet types:");
    for (std::set<Ch10PacketType>::const_iterator it = parsed_packet_types.cbegin();
        it != parsed_packet_types.cend(); ++it)
    {
        if(ch10packettype_to_string_map.count(*it) == 1)
            spdlog::get("pm_logger")->info(" - {:s}", ch10packettype_to_string_map.at(*it));
        else
        {
            spdlog::get("pm_logger")->warn("Ch10PacketType with integer value {:d} not in packet type "
                "to string map", static_cast<uint8_t>(*it));
        }
    }
    spdlog::get("pm_logger")->flush();
}

bool ParserMetadataFunctions::RecordCh10PktTypeSpecificMetadata(Ch10PacketType pkt_type, 
    const std::vector<const Ch10Context*>& context_vec, MDCategoryMap* runtime_metadata, 
    const TMATSData* tmats)
{
    Ch10PacketTypeSpecificMetadata spec_md;
    return RecordCh10PktTypeSpecificMetadata(pkt_type, context_vec, runtime_metadata,
        tmats, &spec_md);
}

void ParserMetadataFunctions::GatherTMATSData(const std::vector<const Ch10Context*>& ctx_vec,
    std::vector<std::string>& tmats_vec)
{
    for (std::vector<const Ch10Context*>::const_iterator it = ctx_vec.cbegin();
        it != ctx_vec.cend(); ++it)
    {
        tmats_vec.push_back((*it)->GetTMATSMatter());
    }
}

int ParserMetadata::RecordMetadataForPktType(const ManagedPath& md_filename, 
    Ch10PacketType pkt_type,
    const ParserPaths* parser_paths, const ParserConfigParams& config, 
    const ProvenanceData& prov_data, const TMATSData* tmats_data, 
    const std::vector<const Ch10Context*>& context_vec,
    TIPMDDocument* tip_md, ParserMetadataFunctions* md_funcs)
{
    std::string pkt_type_label = ch10packettype_to_string_map.at(pkt_type);
    spdlog::get("pm_logger")->debug("RecordMetadata: recording {:s} metadata",
        pkt_type_label);

    ManagedPath md_file_path = parser_paths->GetCh10PacketTypeOutputDirMap()
        .at(pkt_type) / md_filename;

    md_funcs->RecordProvenanceData(tip_md, parser_paths->GetCh10Path(), pkt_type_label, prov_data);
    md_funcs->RecordUserConfigData(tip_md->GetConfigCategory(), config);

    if(!md_funcs->RecordCh10PktTypeSpecificMetadata(pkt_type, context_vec, 
        tip_md->GetRuntimeCategory().get(), tmats_data))
        return 70;

    if(!md_funcs->ProcessTMATSForType(tmats_data, tip_md, pkt_type))
        return 70;

    // Write the complete Yaml record to the metadata file.
    tip_md->CreateDocument();
    if(!md_funcs->WriteStringToFile(md_file_path, tip_md->GetMetadataString()))
    {
        spdlog::get("pm_logger")->warn("Failed to write metadata file for {:s}",
            pkt_type_label.c_str());
        return 74;
    }

    return 0;
}
