// ch10_milstd1553f1.cpp

#include "ch10_milstd1553f1.h"

Ch10MilStd1553F1::~Ch10MilStd1553F1()
{
	
}

#ifdef PARQUET
void Ch10MilStd1553F1::get_msg_names(std::set<std::string>& output_name_set)
{
	db.add_names_to_set(output_name_set);
}
#endif

void Ch10MilStd1553F1::Initialize(const Ch10TimeData* ch10td, const Ch10HeaderData* ch10hd)
{
	ch10td_ptr_ = ch10td;
	ch10hd_ptr_ = ch10hd;
}

//void Ch10MilStd1553F1::scan_initialize(uint32_t chanID)
//{
//	channel_id = chanID;
//}

void Ch10MilStd1553F1::set_channelid_remoteaddress_output(std::map<uint32_t, std::set<uint16_t>>* map_remoteaddr1,
	std::map<uint32_t, std::set<uint16_t>>* map_remoteaddr2)
{
	chanid_remoteaddr1_ptr = map_remoteaddr1;
	chanid_remoteaddr2_ptr = map_remoteaddr2;
}

//uint8_t Ch10MilStd1553F1::parse_minimal()
//{
//	retcode_ = 0;
//
//	// Parse the MilStd 1553 channel specific data.
//	data_fmt_ptr_ = (MilStd1553F1ChanSpecFormat*)bb_ptr_->Data();
//
//#ifdef DEBUG
//	if (DEBUG > 3)
//		debug_info();
//#endif
//
//	// Advance buffer position to beginning of first message.
//	// In the context of the loop a message includes the time 
//	// stamp, intra-pkt data header (block status word, gap times word,
//	// length word), and all data payload which has a variable word 
//	// count.
//
//	// Also note that the packet header parsing protocol would have already
//	// determined if the body or packet trailer exceeds the buffer size, in 
//	// which case the parsing process would have exited. Therefore, I don't need to 
//	// check the return value of the request to advance.
//	bb_ptr_->AdvanceReadPos(data_fmt_size_);
//
//	// Process each 1553 message
//	if (use_comet_command_words)
//	{
//		for (msg_index = 0; msg_index < data_fmt_ptr_->count; msg_index++)
//		{
//			// Before each new message, initialize the status as OK.
//			status_ = MilStd1553F1Status::PARSE_OK;
//
//			// Interpret the current read position as a 1553 message. 
//			// The first 8 bytes are time stamp data. 
//			ts_ptr_ = (const TimeStamp*)bb_ptr_->Data();
//
//			// Parse the time stamp and calculate the absolute time.
//			if (CalcAbsTimeFromTs() == 1)
//				return 1;
//
//			// Set MilStd1553F1Msg class pointer to 2 32-bit positions
//			// past the time stamp pointer = 1 MsgTimeStamp position.
//			// bb_ptr_->AdvanceReadPos(ts_size_) could also be used.
//			msg = (const MilStd1553F1Msg*)(ts_ptr_ + 1);
//
//			// Advance read position to beginning of payload.
//			// Note the msg_hdr_size is the time stamp (8 bytes) 
//			// plus the intra-pkt message header (6 bytes).
//			//bb_ptr_->AdvanceReadPos(intrapkt_hdr_size);
//			bb_ptr_->AdvanceReadPos(msg_hdr_size);
//
//			// DO NOT parse the payload in parse_minimal().
//			/*if (parse_payload())
//				return retcode_;*/
//
//			// Advance read position to beginning of next message.
//			msg_size = msg->length;
//			bb_ptr_->AdvanceReadPos(msg_size);
//		}
//	}
//	else
//	{
//		for (msg_index = 0; msg_index < data_fmt_ptr_->count; msg_index++)
//		{
//			// Before each new message, initialize the status as OK.
//			status_ = MilStd1553F1Status::PARSE_OK;
//
//			// Interpret the current read position as a 1553 message. 
//			// The first 8 bytes are time stamp data. 
//			ts_ptr_ = (const TimeStamp*)bb_ptr_->Data();
//
//			// Parse the time stamp.
//			if (CalcAbsTimeFromTs() == 1)
//				return 1;
//
//			// Set MilStd1553F1Msg class pointer to 2 32-bit positions
//			// past the time stamp pointer = 1 MsgTimeStamp position.
//			msg_commword = (const MilStd1553F1MsgCommWord*)(ts_ptr_ + 1);
//
//			// Insert subaddress1 and 2 into the map.
//			if (msg_commword->RR)
//			{
//				chanid_remoteaddr1_ptr->at(channel_id).insert(msg_commword->remote_addr1);
//				chanid_remoteaddr2_ptr->at(channel_id).insert(msg_commword->remote_addr2);
//			}
//			else
//				chanid_remoteaddr1_ptr->at(channel_id).insert(msg_commword->remote_addr1);
//			
//			//printf("insert subaddrs %02hu, %02hu\n", msg_commword->sub_addr1, msg_commword->sub_addr2);
//
//			// Advance read position to beginning of payload.
//			// Note the msg_hdr_size is the time stamp (8 bytes) 
//			// plus the intra-pkt message header (6 bytes), total 14 bytes.
//			//bb_ptr_->AdvanceReadPos(intrapkt_hdr_size);
//			bb_ptr_->AdvanceReadPos(msg_hdr_size);
//
//			// DO NOT parse the payload in parse_minimal().
//			/*if (parse_payload_without_comet_command_improved())
//				return retcode_;*/
//
//			// Advance read position to beginning of next message.
//			msg_size = msg_commword->length;
//			bb_ptr_->AdvanceReadPos(msg_size);
//		}
//	}
//	return retcode_;
//}

uint8_t Ch10MilStd1553F1::Parse()
{
	retcode_ = 0;
	
	// Parse the MilStd 1553 channel specific data.
	data_fmt_ptr_ = (const MilStd1553F1ChanSpecFormat*) bb_ptr_->Data();
	#ifdef DEBUG
	if (DEBUG > 3)
		debug_info();
	#endif

	// Advance buffer position to beginning of first message.
	// In the context of the loop a message includes the time 
	// stamp, intra-pkt data header (block status word, gap times word,
	// length word), and all data payload which has a variable word 
	// count.
	
	// Also note that the packet header parsing protocol would have already
	// determined if the body or packet trailer exceeds the buffer size, in 
	// which case the parsing process would have exited. Therefore, I don't need to 
	// check the return value of the request to advance.
	bb_ptr_->AdvanceReadPos(data_fmt_size_);
	
	// Process each 1553 message
	for (msg_index = 0; msg_index < data_fmt_ptr_->count; msg_index++)
	{
		// Before each new message, initialize the status as OK.
		status_ = MilStd1553F1Status::PARSE_OK;

		// Interpret the current read position as a 1553 message. 
		// The first 8 bytes are time stamp data. 
		ts_ptr_ = (const TimeStamp*)bb_ptr_->Data();

		// Parse the time stamp.
		if (CalcAbsTimeFromTs() == 1)
			return 1;

		// Set MilStd1553F1Msg class pointer to 2 32-bit positions
		// past the time stamp pointer = 1 MsgTimeStamp position.
		msg_commword = (const MilStd1553F1MsgCommWord*)(ts_ptr_ + 1);

		// Insert subaddress1 and 2 into the map.
		if (msg_commword->RR)
		{
			chanid_remoteaddr1_ptr->at(ch10hd_ptr_->channel_id_).insert(msg_commword->remote_addr1);
			chanid_remoteaddr2_ptr->at(ch10hd_ptr_->channel_id_).insert(msg_commword->remote_addr2);
		}
		else
			chanid_remoteaddr1_ptr->at(ch10hd_ptr_->channel_id_).insert(msg_commword->remote_addr1);

		// Advance read position to beginning of payload.
		// Note the msg_hdr_size is the time stamp (8 bytes) 
		// plus the intra-pkt message header (6 bytes), total 14.
		//bb_ptr_->AdvanceReadPos(intrapkt_hdr_size);
		bb_ptr_->AdvanceReadPos(msg_hdr_size);

		// Parse the payload.
		if (parse_payload_new())
			return retcode_; 

		// Advance read position to beginning of next message.
		msg_size = msg_commword->length;
		bb_ptr_->AdvanceReadPos(msg_size);
	}

	return retcode_;
}

uint8_t Ch10MilStd1553F1::parse_payload_new()
{
	// Cast the datum pointer to the location of the first 
	// payload word. 
	datum = (const uint16_t*)bb_ptr_->Data();

	RTtoBC = msg_commword->tx1;

	// Determine the order of rx/tx/status/data words.
	if (msg_commword->RR)
	{
		/*
			[ RX ][ TX ][ TX STAT ][ DATA0 ] ... [ DATAN ][ RX STAT ]
			// Length in bytes, so divide by 2 to get data word count.
			// Subtract four (rx, tx, tx stat, rx stat) to get the data word count.
		*/
		datum_count = (msg_commword->length / 2) - 4;
		datum_shift = 3;
	}
	else if (RTtoBC)
	{
		/* [ TX ][ STAT ][ DATA0 ] ... [ DATAN ] */

		datum_count = (msg_commword->length / 2) - 2;
		datum_shift = 2;
	}
	else
	{
		/* [ RX ][ DATA0 ] ... [ DATAN ][ STAT ] */

		datum_count = (msg_commword->length / 2) - 2;
		datum_shift = 1;
	}

	if ((msg_commword->word_count1 != datum_count) && check_word_count)
	{
		// If the calculated word count based on message length is equal
		// to the comet specified word count minus one AND the message 
		// Time Out error is set, then a status word wasn't received
		// which means one less word unit should have been subracted from
		// the datum_count and there is no actual mismatch. 

		/*
		Note: Per a discussion in analysis meeting 190523, I will
		record all data, regardless of word count mismatch. Message
		errors will be saved so post-processing check can be conducted
		for datum_count = word_count1 -1, in which case all data ought
		to be present.
		*/

		// if word_count1 == 0, this indicates an actual word count
		// of 32. 
		if (!(msg_commword->word_count1 == 0 && datum_count == 32))
		{
			if(status_ != MilStd1553F1Status::MSG_IDENTITY_FAIL)
				status_ = MilStd1553F1Status::WRD_COUNT_MISMATCH;
#ifdef COLLECT_STATS
			stats.wrd_count_mismatch++;
#endif	

#ifdef DEBUG
#if DEBUG > 3
			printf("(%03hu) WORD COUNT MISMATCH\n", id);
#endif
#endif			
		}
	}

	// Set data payload position based on the type of message.
	datum += datum_shift;

	if (status_ != MilStd1553F1Status::MSG_IDENTITY_FAIL)
	{

#ifdef LOCALDB
#ifdef PARQUET
#ifdef XDAT
		if (use_selected_msg_list)
		{
			msg_names_itr = std::find(msg_names_start, msg_names_end, msg_name);
			if (msg_names_itr != msg_names_end)
				db.append_data(msg_abstime_, day_of_year, msg_name, data_fmt_ptr_, msg_commword, datum);
		}
		else
			db.append_data(msg_abstime_, day_of_year, msg_name, data_fmt_ptr_, msg_commword, datum);
#else
		db.append_data(msg_abstime_, ch10td_ptr_->doy_, msg_name, 
			data_fmt_ptr_, msg_commword, datum, ch10hd_ptr_->channel_id_, datum_count);
#endif
#endif
#endif
	}
	return retcode_;
}

#ifdef LIBIRIG106
uint8_t Ch10MilStd1553F1::UseLibIRIG106(I106C10Header* i106_header, void* buffer)
{
	retcode_ = 0;

	// Parse the ch10 packet body using the LibIRIG106 parser. 
	//Get the first 1553 message.
	i106_status_ = I106_Decode_First1553F1(i106_header, buffer, &i106_1553msg_);
	if (i106_status_ != I106Status::I106_OK)
	{
		printf("\n(%03u) Ch10MilStd1553F1::UseLibIRIG106(): I106_Decode_First1553F1: %s\n", 
			id_, I106ErrorString(i106_status_));

		// There ought to be at least one message in a 1553 packet so I106_NO_MORE_DATA
		// error doesn't apply and the ReadData function that reads the packet body
		// ought to have checked for buffer overruns so the I106_BUFFER_OVERRUN error 
		// does also not apply. If there is a single error then something else is wrong 
		// and we should exit.
		retcode_ = 1;
		return retcode_;
	}

	// Set the CSDW pointer.
	data_fmt_ptr_ = (const MilStd1553F1ChanSpecFormat*)i106_1553msg_.CSDW;

	// Set up pointers and do some checks.
	if (IngestLibIRIG106Msg() == 1)
		return retcode_;

	// Parse the 1553 msg payload.
	ParsePayloadLibIRIG106();

	int msg_count = 1;
	while ((i106_status_ = I106_Decode_Next1553F1(&i106_1553msg_)) == I106Status::I106_OK)
	{
		// Set up pointers and do some checks.
		if (IngestLibIRIG106Msg() == 1)
			return retcode_;

		// Parse the 1553 msg payload.
		ParsePayloadLibIRIG106();

		msg_count++;
	}

	if (msg_count != i106_1553msg_.CSDW->MessageCount)
	{
		printf("\n(%03u) Ch10MilStd1553F1::UseLibIRIG106(): "
			"real message count (%d) not equal to header count (%u)\n",
			id_, msg_count, i106_1553msg_.CSDW->MessageCount);
	}

	// Under normal circumstances the only non-OK status occurs when the 
	// end of the packet body is reached and no more data is indicated. Otherwise,
	// an error occurred.
	if (i106_status_ != I106Status::I106_NO_MORE_DATA)
	{
		printf("\n(%03u) Ch10MilStd1553F1::UseLibIRIG106(): I106_Decode_Next1553F1: %s\n",
			id_, I106ErrorString(i106_status_));

		// I don't yet know what to do in this case. Return 1 for now.
		retcode_ = 1;
		return retcode_;
	}

	return retcode_;
}

uint8_t Ch10MilStd1553F1::IngestLibIRIG106Msg()
{
	// This function assumes that i106_1553_msg_ has already been set.

	// Set the time stamp pointer to the beginning of the IPH.
	ts_ptr_ = (const TimeStamp*)i106_1553msg_.IPH;

	// Parse the time stamp.
	if (CalcAbsTimeFromTs() == 1)
	{
		retcode_ = 1;
		return retcode_;
	}

	// Set MilStd1553F1Msg class pointer to 2 32-bit positions
	// past the time stamp pointer = 1 MsgTimeStamp position.
	msg_commword = (const MilStd1553F1MsgCommWord*)(ts_ptr_ + 1);

	// Check if the data length is too long to make sense, i.e., if it exceeds
	// a max of (32 payload words + 2 command words + 2 status words) * 2 bytes per word = 72 bytes.
	if (msg_commword->length > 72)
	{
		printf("\n(%03u) Ch10MilStd1553F1::UseLibIRIG106(): Message data length (%u) > 64\n", id_,
			i106_1553msg_.DataLength);

		// Do not return 1. We only wish to stop parsing the current 1553 packet,
		// not retire the worker.
		return retcode_;
	}
	
	// Insert subaddress1 and 2 into the map.
	if (msg_commword->RR)
	{
		chanid_remoteaddr1_ptr->at(ch10hd_ptr_->channel_id_).insert(msg_commword->remote_addr1);
		chanid_remoteaddr2_ptr->at(ch10hd_ptr_->channel_id_).insert(msg_commword->remote_addr2);
	}
	else
		chanid_remoteaddr1_ptr->at(ch10hd_ptr_->channel_id_).insert(msg_commword->remote_addr1);

	return retcode_;
}

void Ch10MilStd1553F1::ParsePayloadLibIRIG106()
{
	datum = (const uint16_t*)i106_1553msg_.Data;

	// Using the old, often incorrect method of calculating the message
	// payload count from the message length. This is done to easily compare
	// with the "truth" data set. In the future, use i106_1553msg_.WordCount instead.
	if (msg_commword->RR)
		datum_count = (msg_commword->length / 2) - 4;
	else if (RTtoBC)
		datum_count = (msg_commword->length / 2) - 2;
	else
		datum_count = (msg_commword->length / 2) - 2;

#ifdef LOCALDB
#ifdef PARQUET
	db.append_data(msg_abstime_, ch10td_ptr_->doy_, msg_name,
		data_fmt_ptr_, msg_commword, datum, ch10hd_ptr_->channel_id_, datum_count);
#endif
#endif
}
#endif

//uint8_t Ch10MilStd1553F1::parse_payload_without_comet_command()
//{
//	// Cast the datum pointer to the location of the first 
//	// payload word. 
//	datum = (uint16_t*)bb_ptr_->Data();
//
//	/* 
//	Note: Remove mode code check (immediately below). DRA keeps these and some mode codes
//	may be useful , such as IFM02, "LASTE MARKPOINT". 
//	*/
//
//	// The command word sub address is 0 or 31 if the message is a mode code,
//	// which we currently discard. Further, mode codes can't be identified
//	// using comet.
//	/*if (msg_commword->sub_addr1 == 0 || msg_commword->sub_addr1 == 31)
//	{
//		#ifdef DEBUG
//		if (DEBUG > 3)
//			printf("(%03hu) Mode code message -- skipping\n", id);
//		#endif
//	}*/
//
//	// Note: I may need additional information that is not given explicitly in 
//	// the message header. See comet_pd.py -> CometPD.parse_raw_message() for 
//	// the way to calculate if the message is RT to BC. I'll calculate it here 
//	// and use it for testing purposes.
//	RTtoBC = msg_commword->tx1;
//
//	// Determine the order of rx/tx/status/data words.
//	if (msg_commword->RR)
//	{
//		/*
//			[ RX ][ TX ][ TX STAT ][ DATA0 ] ... [ DATAN ][ RX STAT ]
//			// Length in bytes, so divide by 2 to get data word count.
//			// Subtract four (rx, tx, tx stat, rx stat) to get the data word count.
//		*/
//		datum_count = (msg_commword->length / 2) - 4;
//		datum_shift = 3;
//	}
//	else if (RTtoBC)
//	{
//		/* [ TX ][ STAT ][ DATA0 ] ... [ DATAN ] */
//
//		datum_count = (msg_commword->length / 2) - 2;
//		datum_shift = 2;
//	}
//	else
//	{
//		/* [ RX ][ DATA0 ] ... [ DATAN ][ STAT ] */
//
//		datum_count = (msg_commword->length / 2) - 2;
//		datum_shift = 1;
//	}
//
//	// If no message is identified, msg_name points to "NONE".
//	if (std::strcmp(msg_name, "NONE") == 0)
//	{
//		status_ = MilStd1553F1Status::MSG_IDENTITY_FAIL;
//		#ifdef COLLECT_STATS
//		stats.msg_identity_fail++;
//		#endif	
//
//		#ifdef DEBUG
//		if (DEBUG > 1)
//			printf("(%03hu) MESSAGE IDENTITY FAILURE\n", id_);
//		#endif
//	}
//	else if ((msg_commword->word_count1 != datum_count) && check_word_count)
//	{
//		// If the calculated word count based on message length is equal
//		// to the comet specified word count minus one AND the message 
//		// Time Out error is set, then a status word wasn't received
//		// which means one less word unit should have been subracted from
//		// the datum_count and there is no actual mismatch. 
//
//		/*
//		Note: Per a discussion in analysis meeting 190523, I will 
//		record all data, regardless of word count mismatch. Message
//		errors will be saved so post-processing check can be conducted
//		for datum_count = word_count1 -1, in which case all data ought
//		to be present.
//		*/
//
//		// if word_count1 == 0, this indicates an actual word count
//		// of 32. 
//		if (!(msg_commword->word_count1 == 0 && datum_count == 32))
//		{
//			status_ = MilStd1553F1Status::WRD_COUNT_MISMATCH;
//			#ifdef COLLECT_STATS
//			stats.wrd_count_mismatch++;
//			#endif	
//
//			#ifdef DEBUG
//			if (DEBUG > 3)
//				printf("(%03hu) WORD COUNT MISMATCH\n", id_);
//			#endif			
//		}
//	}
//
//	#ifdef COLLECT_STATS
//	if (std::strcmp(msg_name, "NONE") != 0)
//	{
//		stats.add_msg(msg_name);
//	}
//	#endif
//
//	#ifdef DEBUG
//	if (status_ == MilStd1553F1Status::MSG_IDENTITY_FAIL)
//	{
//		if (DEBUG > 2)
//		{
//			msg_debug_info2();
//		}
//	}
//	#endif
//
//	// Set data payload position based on the type of message.
//	datum += datum_shift;
//
//#ifdef LOCALDB
//	// Commit data to database.
//	if (status_ != MilStd1553F1Status::MSG_IDENTITY_FAIL)
//#ifdef XDAT
//	{
//		if (use_selected_msg_list)
//		{
//			msg_names_itr = std::find(msg_names_start, msg_names_end, msg_name);
//			if(msg_names_itr != msg_names_end)
//				db.append_data(msg_abstime_, day_of_year, msg_name, data_fmt_ptr_, msg_commword, datum);
//		}
//		else
//			db.append_data(msg_abstime_, day_of_year, msg_name, data_fmt_ptr_, msg_commword, datum);
//	}
//#else
//		db.append_data(msg_abstime_, ch10td_ptr_->doy_, msg_name,
//			data_fmt_ptr_, msg_commword, datum, ch10hd_ptr_->channel_id_, datum_count);
//#endif
//#endif
//	return retcode_;
//}

//void Ch10MilStd1553F1::msg_debug_info()
//{
//	printf("(%03hu) Message index  : %03hu\n", id_, msg_index);
//	printf("RTC [ns]       : %llu\n", calculated_rtc_ref_);
//	printf("RT to RT       : %hhu\n", msg->RR);
//	printf("Datum[0]       : %hu\n", datum[0]);
//	printf("Datum[1]       : %hu\n", datum[1]);
//	printf("Comm RT addr/TX/sub addr/N words : %02hu/%01hu/%02hu/%02hu\n", comm_word->remote_addr,
//		comm_word->tx, comm_word->sub_addr, comm_word->word_count);
//	printf("RTtoBC         : %hhu\n", RTtoBC);
//	printf("Message length : %hu\n", msg->length);
//	printf("Calc wrd count : %hu\n", datum_count);
//	printf("Comet wrd count: %hu\n", *word_count);
//	printf("Comet msg name : %s\n", msg_name);
//#ifdef DEBUG
//#if DEBUG > 2
//	{
//		printf("WE/SE/WCE/TO/FE/ME/BusID %hu/%hu/%hu/%hu/%hu/%hu/%hu\n", msg->WE, msg->SE,
//			msg->WCE, msg->TO, msg->FE, msg->ME, msg->bus_dir);
//	}
//#endif
//#endif
//	printf("\n");
//}

void Ch10MilStd1553F1::msg_debug_info2()
{
	printf("(%03hu) Message index  : %03hu\n", id_, msg_index);
	printf("RTC [ns]       : %llu\n", calculated_rtc_ref_);
	printf("RT to RT       : %hhu\n", msg_commword->RR);
	printf("Datum[0]       : %hu\n", datum[0]);
	printf("Datum[1]       : %hu\n", datum[1]);
	printf("Comm1 RT addr/TX/sub addr/N words : %02hu/%01hu/%02hu/%02hu\n", msg_commword->remote_addr1,
		msg_commword->tx1, msg_commword->sub_addr1, msg_commword->word_count1);
	printf("Comm2 RT addr/TX/sub addr/N words : %02hu/%01hu/%02hu/%02hu\n", msg_commword->remote_addr2,
		msg_commword->tx2, msg_commword->sub_addr2, msg_commword->word_count2);
	printf("RTtoBC         : %hhu\n", RTtoBC);
	printf("Message length : %hu\n", msg_commword->length);
	printf("Calc wrd count : %hu\n", datum_count);
	//printf("Comet wrd count: %hu\n", *word_count);
	printf("Comet msg name : %s\n", msg_name);
#ifdef DEBUG
#if DEBUG > 2
	{
		printf("WE/SE/WCE/TO/FE/ME/BusID %hu/%hu/%hu/%hu/%hu/%hu/%hu\n", msg_commword->WE, msg_commword->SE,
			msg_commword->WCE, msg_commword->TO, msg_commword->FE, msg_commword->ME, msg_commword->bus_dir);
	}
#endif
#endif
	printf("\n");
}

void Ch10MilStd1553F1::debug_info()
{
	printf("\n(%03u) -- MilStd-1553, Format 1 --\n", id_);
	printf("channel ID %u\n", ch10hd_ptr_->channel_id_);
	printf("abstime rtc 		= %llu\n", ch10td_ptr_->timedatapkt_abstime_);
	printf("message count		= %u\n", data_fmt_ptr_->count);
	printf("message ts size = %u\n", ts_size_);
	printf("intrapkt hdr size = %u\n", intrapkt_hdr_size);
}

//uint8_t Ch10MilStd1553F1::write_data()
//{
//	// Only save data if the status is PARSE_OK.
//	return retcode_;
//}

Ch10MilStd1553F1Stats& Ch10MilStd1553F1::get_stats()
{
	return stats;
}

void Ch10MilStd1553F1::set_truncate(bool state)
{
#ifdef LOCALDB

#endif
}

void Ch10MilStd1553F1::close()
{
#ifdef LOCALDB
#ifdef PARQUET
	db.commit();
#endif
#endif
	
}