// ch10_packet_header.cpp

#include "ch10_packet_header.h"

Ch10PacketHeader::~Ch10PacketHeader()
{
}

uint8_t Ch10PacketHeader::Parse()
{	
	retcode_ = 0;
	start_pos = bb_ptr_->position_;
	// Check if the packet header extends beyond the binary
	// date in memory.
	bool data_present = bb_ptr_->BytesAvailable(data_fmt_size_);
	if(!data_present)
	{
#ifdef DEBUG
#if DEBUG > 2
			printf("(%03u) Ch10PacketHeader: Packet header exceeds binary buffer\n", id_);
#endif
#endif
		status_ = Ch10PacketHeaderStatus::HEADER_EXCEEDS;
		retcode_ = 1;
		return retcode_;
	}
	
	// Parse the packet header by casting the appropriate location
	// in the binary buffer.
	data_fmt_ptr_ = (const Ch10PacketHeaderFormat*) bb_ptr_->Data();
	
	// Confirm correct sync value.
	if(data_fmt_ptr_->sync != sync_value)
	{
		// Find next header . . . 
#ifdef DEBUG
#if DEBUG > 1
		printf("(%03u) Ch10PacketHeader::parse(): Sync value incorrect\n", id_);
#endif
#endif
		status_ = Ch10PacketHeaderStatus::SYNC_INCORRECT;
		retcode_ = 1;
		return retcode_;
	}
	
	// Calculate checksum.
	bool check = calc_checksum();
	if(!check)
	{
#ifdef DEBUG
#if DEBUG > 2
			printf("(%03u) Ch10PacketHeader: checksum is INCORRECT\n", id_);
#endif
#endif
		status_ = Ch10PacketHeaderStatus::CHECKSUM_INCORRECT;
		retcode_ = 1;
		return retcode_;
	}
	
	// Check if the packet body extends beyond binary 
	// data. 
	data_present = bb_ptr_->BytesAvailable(data_fmt_ptr_->pkt_size);
	if(!data_present)
	{
#ifdef DEBUG
#if DEBUG > 2
			printf("(%03u) Ch10PacketHeader: Packet body exceeds binary buffer\n", id_);
#endif
#endif
		status_ = Ch10PacketHeaderStatus::BODY_EXCEEDS;
		retcode_ = 1;
		return retcode_;
	}
	
	// Calculate the data checksum. This function also advances the buffer position
	// to the beginning of the packet body if there is no error, i.e., check = 0.
	check = calc_data_checksum();
	if(!check)
	{
#ifdef DEBUG
#if DEBUG > 2
			printf("(%03u) Ch10PacketHeader: data checksum is INCORRECT\n", id_);
#endif
#endif
		status_ = Ch10PacketHeaderStatus::DATA_CHECKSUM_INCORRECT;
		retcode_ = 1;
		return retcode_;
	}
	
	// Calculate the relative time counter (RTC). This function automatically
	// fills the Ch10HeaderData::calculated_rtc_ field.
	CalcRtc(data_fmt_ptr_->rtc1, data_fmt_ptr_->rtc2);
	ch10hd_.header_rtc_ = calculated_rtc_ref_;

	// Fill Ch10HeaderData object with parsed data.
	ch10hd_.time_format_ = data_fmt_ptr_->time_format;
	ch10hd_.intrapkt_ts_source_ = data_fmt_ptr_->intrapkt_ts_source;
	ch10hd_.channel_id_ = data_fmt_ptr_->chanID;
	ch10hd_.pkt_body_size_ = data_fmt_ptr_->data_size;
	
#ifdef DEBUG
#if DEBUG > 3
		debug_info();
#endif
#endif
	
	status_ = Ch10PacketHeaderStatus::PARSE_OK;
	return retcode_;
}

#ifdef LIBIRIG106
void Ch10PacketHeader::UseLibIRIG106PktHdrStruct(const I106C10Header* i106_header_ptr, const int64_t& offset,
	int header_length)
{
	// Set the packet start position as the current LibIRIG106 file pointer offset
	// (which is placed at the beginning of the packet body after reading a header)
	// subract the header size.
	start_pos = offset - header_length;

	data_fmt_ptr_ = (const Ch10PacketHeaderFormat*)i106_header_ptr;

	// Calculate the relative time counter (RTC). This function automatically
	// fills the Ch10HeaderData::header_rtc_ field.
	CalcRtc(data_fmt_ptr_->rtc1, data_fmt_ptr_->rtc2);
	ch10hd_.header_rtc_ = calculated_rtc_ref_;

	// Fill Ch10HeaderData object with parsed data.
	ch10hd_.time_format_ = data_fmt_ptr_->time_format;
	ch10hd_.intrapkt_ts_source_ = data_fmt_ptr_->intrapkt_ts_source;
	ch10hd_.channel_id_ = data_fmt_ptr_->chanID;
	ch10hd_.pkt_body_size_ = data_fmt_ptr_->data_size;

#ifdef DEBUG
#if DEBUG > 3
		debug_info();
#endif
#endif
}
#endif

void Ch10PacketHeader::find_sync_locs()
{
	// Search for all locations of the packet header sync bytes.
	uint16_t search_pattern = sync_value;
	sync_loc_index = 0;
	bb_ptr_->SetReadPos(0);
	sync_locs = bb_ptr_->FindAllPattern(search_pattern);
#ifdef DEBUG
#if DEBUG > 1
		printf("(%03u) Ch10PacketHeader: Sync location quantity = %zu\n", id_, sync_locs.size());
#endif
#endif
}

uint32_t Ch10PacketHeader::find_first_time_data_packet(bool append_mode)
{
	uint8_t retcode = 0;
	find_sync_locs();
	
	// Loop over all sync locations and look for the time data packet.
	for(uint32_t i=0; i < sync_locs.size(); ++i)
	{
		// Set file read position to location of potential sync location.
		bb_ptr_->SetReadPos(sync_locs[i]);
		retcode = Parse();
		if(retcode)
		{
#ifdef DEBUG
#if DEBUG > 2
				printf("(%03u) Ch10PacketHeader::find_first_time_data_packet: pkt %u status %u\n", 
					id_, i, status_);
#endif
#endif
			//return UINT32_MAX; // debug, remove this!
		}
		else if (append_mode)
		{
			sync_loc_index = i;
			return sync_locs[i];
		}
		else if (data_fmt_ptr_->data_type == static_cast<uint8_t>(Ch10DataType::TIME_DATA_F1))
		{
			// Set bb_ptr_ read position to the end of the packet header.
#ifdef DEBUG
#if DEBUG > 2
				printf("(%03hu) Ch10PacketHeader::find_first_time_data_packet: time data pkt found at loc %llu\n", id_, sync_locs[i]);
#endif
#endif
			sync_loc_index = i;
			//bb_ptr_.SetReadPos(sync_locs[i]);
			return sync_locs[i];
		}
	}
	return UINT32_MAX;
}

/*void Ch10PacketHeader::calc_rtc()
{
	uint64_t rtc_temp = data_fmt_ptr_->rtc2;
	rtc = (rtc_temp << 32) + data_fmt_ptr_->rtc1;
}*/

bool Ch10PacketHeader::calc_checksum()
{
	uint16_t* check_units = (uint16_t*) bb_ptr_->Data();
	uint8_t n_units = uint8_t((data_fmt_size_ - checksum_size)/checksum_size);
	uint16_t checksum = 0;
	for(uint8_t i = 0; i < n_units; i++)
		checksum += check_units[i];
#ifdef DEBUG
#if DEBUG > 3
	{
		printf("\n(%03u) Ch10PacketHeader: %u checksum units\n", id_, n_units);
		printf("(%03u) checksum value/calculated: %u/%u\n", id_, data_fmt_ptr_->checksum, checksum);
	}
#endif
#endif
	if(checksum == data_fmt_ptr_->checksum)
		return true;
	return false;
}

bool Ch10PacketHeader::calc_data_checksum()
{
	// Note that this function advances the buffer read position to 
	// the beginning of the packet body only if case 0, 1, 2, or 3 occurs. 
	uint8_t checksum_bits = 0;
	uint32_t read_pos = 0;
	uint16_t n_units = 0;
	uint32_t calculated_checksum = 0;
	uint32_t given_checksum = 0;
	
	// Read the file checksum and calculate it from raw data. 
	switch(data_fmt_ptr_->checksum_existence)
	{
		case 0:
		{
			// No checksum present.
			read_pos = data_fmt_size_;
			bb_ptr_->AdvanceReadPos(read_pos);
			return true;
		}
		case 1:
		{
			// 8-bit checksum
			checksum_bits = 8;
			read_pos = data_fmt_ptr_->pkt_size - checksum_bits/8;
			bb_ptr_->AdvanceReadPos(read_pos);
			uint8_t* checksum = (uint8_t*) bb_ptr_->Data();
			given_checksum = *checksum;
			read_pos = start_pos + data_fmt_size_;
			bb_ptr_->SetReadPos(read_pos);
			uint8_t* check_units = (uint8_t*) bb_ptr_->Data();
			uint8_t temp_checksum = 0;
			n_units = uint16_t((data_fmt_ptr_->pkt_size - data_fmt_size_ - checksum_bits/8)/(checksum_bits/8));
			for(uint16_t i = 0; i < n_units; i++)
				temp_checksum += check_units[i];
			calculated_checksum = temp_checksum;
			break;
		}
		case 2:
		{
			// 16-bit checksum
			checksum_bits = 16;
			read_pos = data_fmt_ptr_->pkt_size - checksum_bits/8;
			bb_ptr_->AdvanceReadPos(read_pos);
			uint16_t* checksum = (uint16_t*) bb_ptr_->Data();
			given_checksum = *checksum;
			read_pos = start_pos + data_fmt_size_;
			bb_ptr_->SetReadPos(read_pos);
			uint16_t* check_units = (uint16_t*) bb_ptr_->Data();
			uint16_t temp_checksum = 0;
			n_units = uint16_t((data_fmt_ptr_->pkt_size - data_fmt_size_ - checksum_bits/8)/(checksum_bits/8));
			for(uint16_t i = 0; i < n_units; i++)
				temp_checksum += check_units[i];
			calculated_checksum = temp_checksum;
			break;
		}
		case 3:
		{
			// 32-bit checksum
			checksum_bits = 32;
			read_pos = data_fmt_ptr_->pkt_size - checksum_bits/8;
			bb_ptr_->AdvanceReadPos(read_pos);
			uint32_t* checksum = (uint32_t*) bb_ptr_->Data();
			given_checksum = *checksum;
			read_pos = start_pos + data_fmt_size_;
			bb_ptr_->SetReadPos(read_pos);
			uint32_t* check_units = (uint32_t*) bb_ptr_->Data();
			uint32_t temp_checksum = 0;
			n_units = uint16_t((data_fmt_ptr_->pkt_size - data_fmt_size_ - checksum_bits/8)/(checksum_bits/8));
			for(uint16_t i = 0; i < n_units; i++)
				temp_checksum += check_units[i];
			calculated_checksum = temp_checksum;
			break;
		}
		default:
		{
#ifdef DEBUG
#if DEBUG > 1
				printf("(%03u) checksum_existence value %hhu invalid\n", id_, data_fmt_ptr_->checksum_existence);
#endif
#endif
			return false;
		}
	}
	
	#ifdef DEBUG
#if DEBUG > 3
	{
		printf("\n(%03u) Ch10PacketHeader: %u bit, %u checksum units\n", id_, checksum_bits, n_units);
		printf("(%03u) data checksum value/calculated: %u/%u\n", id_, given_checksum, calculated_checksum);
	}
#endif
	#endif
	if(given_checksum == calculated_checksum)
		return true;
	return false;
}

void Ch10PacketHeader::debug_info()
{
	printf("\n(%03u) Sync loc index		= %u\n", id_, sync_loc_index);
	printf("Buffer position		= %u\n", start_pos);
	printf("Channel ID		= %u\n", data_fmt_ptr_->chanID);
	printf("Packet size		= %u\n", data_fmt_ptr_->pkt_size);
	printf("Data size		= %u\n", data_fmt_ptr_->data_size);
	printf("Sequence number		= %u\n", data_fmt_ptr_->seq_num);
	printf("Time format		= %u\n", data_fmt_ptr_->time_format);
	printf("Overflow/Sync error	= %u/%u\n", data_fmt_ptr_->overflow_err, data_fmt_ptr_->sync_err);
	printf("Intrapkt TS source	= %u\n", data_fmt_ptr_->intrapkt_ts_source);
	printf("Secondary header	= %u\n", data_fmt_ptr_->secondary_hdr);
	printf("Data type 		= %u\n", data_fmt_ptr_->data_type);
	printf("RTC			= %llu\n", ch10hd_.header_rtc_);
	printf("Checksum existence	= %u\n", data_fmt_ptr_->checksum_existence);
}

uint8_t Ch10PacketHeader::advance_to_next()
{
	uint32_t new_pos = start_pos + data_fmt_ptr_->pkt_size;
	return bb_ptr_->SetReadPos(new_pos);
}

uint8_t Ch10PacketHeader::advance_to_next_sync_location()
{
	for(sync_loc_index; sync_loc_index < sync_locs.size(); sync_loc_index++)
	{
		if(sync_locs[sync_loc_index] > bb_ptr_->position_)
			return bb_ptr_->SetReadPos(sync_locs[sync_loc_index]);
	}
	return 1;
} 

uint64_t& Ch10PacketHeader::relative_time_counter()
{
	return ch10hd_.header_rtc_;
}

uint32_t& Ch10PacketHeader::start_position()
{
	return start_pos;
}

std::string Ch10PacketHeader::status_desc()
{
	switch (status_)
	{
	case Ch10PacketHeaderStatus::BODY_EXCEEDS:
		return "BODY_EXCEEDS";
	case Ch10PacketHeaderStatus::CHECKSUM_INCORRECT:
		return "CHECKSUM_INCORRECT";
	case Ch10PacketHeaderStatus::DATA_CHECKSUM_INCORRECT:
		return "DATA_CHECKSUM_INCORRECT";
	case Ch10PacketHeaderStatus::HEADER_EXCEEDS:
		return "HEADER_EXCEEDS";
	case Ch10PacketHeaderStatus::PARSE_OK:
		return "PARSE_OK";
	case Ch10PacketHeaderStatus::SYNC_INCORRECT:
		return "SYNC_INCORRECT";
	default:
		return "";
	}
}