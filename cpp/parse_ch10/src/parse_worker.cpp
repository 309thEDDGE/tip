// parse_worker.cpp
#include "parse_worker.h"

ParseWorker::ParseWorker() : complete_(false), retval_(0)
{
}

void ParseWorker::operator()(WorkerConfig& worker_config, Ch10Context* ctx)
{
    // Reset completion status.
    complete_ = false;

    if (worker_config.append_mode_)
        SPDLOG_INFO("({:02d}) APPEND MODE ParseWorker now active", worker_config.worker_index_);
    else
        SPDLOG_INFO("({:02d}) ParseWorker now active", worker_config.worker_index_);

    SPDLOG_DEBUG("({:02d}) Beginning of shift, absolute position: {:d}",
                 worker_config.worker_index_, worker_config.start_position_);

    // Initialize Ch10Context object. Note that this Ch10Context instance
    // created in the ParseWorker constructor is persistent until the
    // ParseWorker instance is garbage-collected to maintain/retain file
    // writer state.
    ctx->Initialize(worker_config.start_position_, worker_config.worker_index_);
    ctx->SetSearchingForTDP(!worker_config.append_mode_);

    if ((retval_ = ConfigureContext(ctx, worker_config.ch10_packet_type_map_, worker_config.output_file_paths_)) != 0)
    {
        complete_ = true;
        return;
    }

    ParseBufferData(ctx, worker_config.bb_);

    // Update last_position_;
    worker_config.last_position_ = ctx->absolute_position;

    // Close all file writers if append_mode is true or
    // this is the final worker which has no append mode.
    if (worker_config.append_mode_ || worker_config.final_worker_)
    {
        SPDLOG_DEBUG("({:02d}) Closing file writers", worker_config.worker_index_);
        ctx->CloseFileWriters();
    }

    SPDLOG_INFO("({:02d}) End of worker's shift", worker_config.worker_index_);
    SPDLOG_DEBUG("({:02d}) End of shift, absolute position: {:d}",
                 worker_config.worker_index_, worker_config.last_position_);

    //new
    worker_config.bb_->Clear();

    complete_ = true;
    retval_ = 0;
}

std::atomic<bool>& ParseWorker::CompletionStatus()
{
    return complete_;
}


std::atomic<int>& ParseWorker::ReturnValue()
{
    return retval_;
}

int ParseWorker::ConfigureContext(Ch10Context* ctx,
                                   const std::map<Ch10PacketType, bool>& ch10_packet_type_map,
                                   const std::map<Ch10PacketType, ManagedPath>& output_file_paths_map)
{
    if (!ctx->IsConfigured())
    {
        // Configure packet parsing.
        if (!ctx->SetPacketTypeConfig(ch10_packet_type_map, ctx->pkt_type_config_map))
            return EX_SOFTWARE;

        // Check configuration. Are the packet parse and output paths configs
        // consistent?
        std::map<Ch10PacketType, ManagedPath> enabled_paths;
        bool config_ok = ctx->CheckConfiguration(ctx->pkt_type_config_map,
                                                output_file_paths_map, enabled_paths);
        if (!config_ok)
            return EX_SOFTWARE;

        // For each packet type that is enabled and has an output path specified,
        // create a file writer object that is owned by Ch10Context to maintain
        // state between regular and append mode calls to this worker's operator().
        // Pass a pointer to the file writer to the relevant parser for use in
        // writing data to disk.
        int retcode = 0;
        if ((retcode = ctx->InitializeFileWriters(enabled_paths)) != 0)
            return retcode;
    }
    return EX_OK;
}

void ParseWorker::ParseBufferData(Ch10Context* ctx, BinBuff* bb)
{
    Ch10PacketHeaderComponent header(ctx);
    Ch10TMATSComponent tmats(ctx);
    Ch10TDPComponent tdp(ctx);
    Ch101553F1Component milstd1553(ctx);
    Ch10VideoF0Component vid(ctx);
    Ch10EthernetF0Component eth(ctx);
    Ch10429F0Component arinc429(ctx);
    Ch10Time ch10time;

    // Enable Pcap output if Ethernet data packet parsing is enabled.
    if (ctx->pkt_type_config_map.at(Ch10PacketType::ETHERNET_DATA_F0))
    {
        eth.EnablePcapOutput(
            ctx->pkt_type_paths_map.at(Ch10PacketType::ETHERNET_DATA_F0));
    }

    // Instantiate Ch10Packet object
    Ch10Packet packet(bb, ctx, &ch10time);
    packet.SetCh10ComponentParsers(&header, &tmats, &tdp, &milstd1553, &vid, &eth, &arinc429);
    if(!packet.IsConfigured())
        return;

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
}
