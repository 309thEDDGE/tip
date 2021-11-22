// parse_worker.cpp
#include "parse_worker.h"

ParseWorker::ParseWorker() : complete_(false), ch10_context_(ctx_), ctx_()
{
}

void ParseWorker::operator()(WorkerConfig& worker_config,
                             std::vector<std::string>& tmats_body_vec)
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
    ctx_.Initialize(worker_config.start_position_, worker_config.worker_index_);
    ctx_.SetSearchingForTDP(!worker_config.append_mode_);

    if (!ConfigureContext(ctx_, worker_config.ch10_packet_type_map_, worker_config.output_file_paths_))
    {
        complete_ = true;
        return;
    }

    ParseBufferData(&ctx_, &worker_config.bb_, tmats_body_vec);

    // Update last_position_;
    worker_config.last_position_ = ctx_.absolute_position;

    // Close all file writers if append_mode is true or
    // this is the final worker which has no append mode.
    if (worker_config.append_mode_ || worker_config.final_worker_)
    {
        SPDLOG_DEBUG("({:02d}) Closing file writers", worker_config.worker_index_);
        ctx_.CloseFileWriters();
    }

    SPDLOG_INFO("({:02d}) End of worker's shift", worker_config.worker_index_);
    SPDLOG_DEBUG("({:02d}) End of shift, absolute position: {:d}",
                 worker_config.worker_index_, worker_config.last_position_);
    complete_ = true;
}

std::atomic<bool>& ParseWorker::CompletionStatus()
{
    return complete_;
}

bool ParseWorker::ConfigureContext(Ch10Context& ctx,
                                   const std::map<Ch10PacketType, bool>& ch10_packet_type_map,
                                   const std::map<Ch10PacketType, ManagedPath>& output_file_paths_map)
{
    if (!ctx.IsConfigured())
    {
        // Configure packet parsing.
        if (!ctx.SetPacketTypeConfig(ch10_packet_type_map, ctx.pkt_type_config_map))
            return false;

        // Check configuration. Are the packet parse and output paths configs
        // consistent?
        std::map<Ch10PacketType, ManagedPath> enabled_paths;
        bool config_ok = ctx.CheckConfiguration(ctx.pkt_type_config_map,
                                                output_file_paths_map, enabled_paths);
        if (!config_ok)
            return false;

        // For each packet type that is enabled and has an output path specified,
        // create a file writer object that is owned by Ch10Context to maintain
        // state between regular and append mode calls to this worker's operator().
        // Pass a pointer to the file writer to the relevant parser for use in
        // writing data to disk.
        if (!ctx.InitializeFileWriters(enabled_paths))
            return false;
    }
    return true;
}

void ParseWorker::ParseBufferData(Ch10Context* ctx, BinBuff* bb,
                                  std::vector<std::string>& tmats_vec)
{
    // Instantiate Ch10Packet object
    Ch10Packet packet(bb, ctx, tmats_vec);

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