#include "ch10_videodataf0.h"

Ch10VideoDataF0::~Ch10VideoDataF0()
{

}

#ifdef LOCALDB
#ifdef PARQUET
Ch10VideoDataF0::Ch10VideoDataF0(BinBuff& buff, uint16_t ID, TMATS& tmats, std::string out_path) : 
	ParseContext(buff, ID),
outpath(out_path), subpkt_unit_count(0), subpkt_unit_index(0),
transport_stream_pkt(nullptr), subpkt_unit_size(0), transport_stream_data(MAX_DATA_COUNT, 0),
transport_stream_TS(MAX_TransportStream_UNITS, 0), video_datum_it(transport_stream_data.begin()),
db(outpath, ID, true),
have_chanid_to_label_map(false), total_data_unit_count(0), chanid_label("")
{
	// Use TMATS class to obtain a map of the channel id to the video source name.
	// TODO: move creation of map to ParseManager and pass reference to map to all workers for speed.
	chanid_to_label_map = tmats.map_channel_to_source_by_type(ChannelDataType::VIDIN);
	if (chanid_to_label_map.size() > 0)
		have_chanid_to_label_map = true;
#ifdef DEBUG
#if DEBUG > 1
	std::map<uint32_t, std::string>::iterator it;
	for (it = chanid_to_label_map.begin(); it != chanid_to_label_map.end(); ++it)
	{
		printf("video chanid_to_label_map: %04u --> %s\n", it->first, (it->second).c_str());
	}
#endif
#endif
	// TODO: create *.ts file name from out_path.

}
#endif
#endif

void Ch10VideoDataF0::Initialize(const Ch10TimeData* ch10td, const Ch10HeaderData* ch10hd)
{
	ch10td_ptr_ = ch10td;
	ch10hd_ptr_ = ch10hd;
}

uint8_t Ch10VideoDataF0::Parse()
{
	retcode_ = 0;

	// Parse the channel specific data.
	data_fmt_ptr_ = (const VideoDataF0ChanSpecFormat*)bb_ptr_->Data();

	// Calculate the quantity of TS packets. If the intra-pkt header
	// is present then each TS packet is prefaced by a time stamp. Include
	// the time stamp data size in the calculation if present.
	if (data_fmt_ptr_->IPH)
	{
		// Intra-packet header is present. 
		subpkt_unit_size = TransportStream_UNIT_SIZE + ts_size_;
	}
	else
	{
		// Intra-packet header is not present.
		subpkt_unit_size = TransportStream_UNIT_SIZE;

		// No intra-pkt header, which contains per-packet time stamps.
		if(CalcAbsTimeFromHdrRtc() == 1)
			printf("(%03hu) msg_abstime_ = %llu\n", id_, msg_abstime_);
	}

#ifdef DEBUG
#if DEBUG > 2
	print_video_pkt_info();
#endif
#endif

	if ((ch10hd_ptr_->pkt_body_size_ - data_fmt_size_) % subpkt_unit_size != 0)
	{
		status_ = VideoDataF1Status::NONINTEGER_TSPKT_COUNT;
#ifdef DEBUG
#if DEBUG > 0
		printf("(%03hu) Non-integer TS unit count\n", id_);
#endif
#endif
		retcode_ = 1;
		return retcode_;
	}
	subpkt_unit_count = (ch10hd_ptr_->pkt_body_size_ - data_fmt_size_) / subpkt_unit_size;
#ifdef DEBUG
#if DEBUG > 2
	printf("(%03hu) subpkt_unit_count: %u\n", id, subpkt_unit_count);
#endif
#endif

	if (subpkt_unit_count > MAX_TransportStream_UNITS)
	{
		printf("(%03hu) subpkt_unit_count (%u) > MAX_TransportStream_UNITS (%03d)\n", 
			id_, subpkt_unit_count, MAX_TransportStream_UNITS);
		retcode_ = 1;
		return retcode_;
	}

	// Advance read position to beginning of first TS unit.
	bb_ptr_->AdvanceReadPos(data_fmt_size_);

	// Loop over all TS units and process as necessary.
	for (subpkt_unit_index = 0; subpkt_unit_index < subpkt_unit_count; subpkt_unit_index++)
	{
		if (data_fmt_ptr_->IPH)
		{
			// If Intra-packet header is present, set the TimeStamp pointer.
			ts_ptr_ = (const TimeStamp*)bb_ptr_->Data();

			// Parse the time stamp and calculate the absolute time.
			if (CalcAbsTimeFromTs())
			{
				switch (parse_status_)
				{
				case ParseStatus::TS_NOT_IMPL:
					status_ = VideoDataF1Status::TS_NOT_IMPL;
					break;
				case ParseStatus::TS_RESERVED:
					status_ = VideoDataF1Status::TS_RESERVED;
					break;
				}
				return retcode_;
			}

			// Copy the time stamp (TS) into the transport_stream_TS vector.
			transport_stream_TS[subpkt_unit_count] = msg_abstime_;
#ifdef DEBUG
#if DEBUG > 2
			printf("(%03hu) absolute_msg_TS = %llu\n", id, absolute_msg_TS);
#endif
#endif

			// Set the new transport stream packet video_datum pointer.
			transport_stream_pkt = (const video_datum*)(ts_ptr_ + 1);
		}
		else
		{
			// If Intra-packet header is not present, set only the video_datum pointer.
			transport_stream_pkt = (const video_datum*)bb_ptr_->Data();
		}

		// Copy the current transport stream packet into the transport_stream_data vector.
		std::copy(transport_stream_pkt, transport_stream_pkt + TransportStream_DATA_COUNT, 
			video_datum_it + (subpkt_unit_index * TransportStream_DATA_COUNT));

		// Advance the read position to the next TS unit (which may include a preceding time stamp).
		bb_ptr_->AdvanceReadPos(subpkt_unit_size);
	}

	/* Commit data to file. */
	if (have_chanid_to_label_map)
	{
		chanid_label = chanid_to_label_map[ch10hd_ptr_->channel_id_];
	}
	else
	{
		chanid_label = std::to_string(ch10hd_ptr_->channel_id_);
	}

#ifdef PARQUET
	if (!data_fmt_ptr_->IPH)
	{
		transport_stream_TS[0] = msg_abstime_;
	}
	db.append_data(transport_stream_TS, ch10td_ptr_->doy_, chanid_label, 
		ch10hd_ptr_->channel_id_, data_fmt_ptr_, subpkt_unit_count, transport_stream_data);
#endif

	return retcode_;
}

void Ch10VideoDataF0::print_video_pkt_info()
{
	printf("\n(%03u) --- Video Data, Format 0 ---\n", id_);
	printf("absolute_pkt_TS      : %llu\n", ch10td_ptr_->timedatapkt_abstime_);
	printf("absolute_TS_rtc      : %llu\n", ch10td_ptr_->timedatapkt_rtc_);
	printf("TS_doy               : %hhu\n", ch10td_ptr_->doy_);
	printf("TS_source            : %u\n", ch10hd_ptr_->intrapkt_ts_source_);
	printf("TS_format            : %u\n", ch10hd_ptr_->time_format_);
	printf("channel_ID           : %u\n", ch10hd_ptr_->channel_id_);
	printf("data_size            : %u\n", ch10hd_ptr_->pkt_body_size_);
	printf("payload (PL)         : %u\n", data_fmt_ptr_->PL);
	printf("intra-pkt hdr (IPH)  : %u\n", data_fmt_ptr_->IPH);
	printf("subpk_unit_size         : %u\n", TransportStream_UNIT_SIZE);
	printf("subpkt_unit_count (calc.): %f\n", 
		float((ch10hd_ptr_->pkt_body_size_ - data_fmt_size_) / subpkt_unit_size));
}

void Ch10VideoDataF0::close()
{
#ifdef PARQUET
	db.commit();
#endif

}

void Ch10VideoDataF0::set_truncate(bool state)
{
}
