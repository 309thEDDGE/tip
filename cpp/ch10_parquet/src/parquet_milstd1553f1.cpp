#include "parquet_milstd1553f1.h"

ParquetMilStd1553F1::ParquetMilStd1553F1() : max_temp_element_count_(DEFAULT_ROW_GROUP_COUNT* DEFAULT_BUFFER_SIZE_MULTIPLIER),
ParquetContext(DEFAULT_ROW_GROUP_COUNT), id_(0), temp_element_count_(0), commword_ptr_(nullptr)
{

}

ParquetMilStd1553F1::ParquetMilStd1553F1(std::string outfile, uint16_t ID, bool truncate) : 
	max_temp_element_count_(DEFAULT_ROW_GROUP_COUNT * DEFAULT_BUFFER_SIZE_MULTIPLIER),
	ParquetContext(DEFAULT_ROW_GROUP_COUNT), id_(ID), temp_element_count_(0), commword_ptr_(nullptr)
{
	// Allocate vector memory. 
	time_stamp_.resize(max_temp_element_count_);
	doy_.resize(max_temp_element_count_);
	ttb_.resize(max_temp_element_count_);
	WE_.resize(max_temp_element_count_);
	SE_.resize(max_temp_element_count_);
	WCE_.resize(max_temp_element_count_);
	TO_.resize(max_temp_element_count_);
	FE_.resize(max_temp_element_count_);
	RR_.resize(max_temp_element_count_);
	ME_.resize(max_temp_element_count_);
	gap1_.resize(max_temp_element_count_);
	gap2_.resize(max_temp_element_count_);
	mode_code_.resize(max_temp_element_count_);
	data_.resize(max_temp_element_count_ * DATA_PAYLOAD_LIST_COUNT, 0);
	//word_count_.resize(max_temp_element_count_);
	comm_word1_.resize(max_temp_element_count_);
	comm_word2_.resize(max_temp_element_count_);
	rtaddr1_.resize(max_temp_element_count_);
	tr1_.resize(max_temp_element_count_);
	subaddr1_.resize(max_temp_element_count_);
	wrdcnt1_.resize(max_temp_element_count_);
	rtaddr2_.resize(max_temp_element_count_);
	tr2_.resize(max_temp_element_count_);
	subaddr2_.resize(max_temp_element_count_);
	wrdcnt2_.resize(max_temp_element_count_);
	channel_id_.resize(max_temp_element_count_);
	payload_count_.resize(max_temp_element_count_);
	//msglen_.resize(max_temp_element_count_);

	// Add fields to table.
	addField(arrow::int64(), "time");
	addField(arrow::int16(), "doy");
	addField(arrow::int32(), "channelid");
	addField(arrow::int8(), "ttb");
	addField(arrow::boolean(), "WE");
	addField(arrow::boolean(), "SE");
	addField(arrow::boolean(), "WCE");
	addField(arrow::boolean(), "TO");
	addField(arrow::boolean(), "FE");
	addField(arrow::boolean(), "RR");
	addField(arrow::boolean(), "ME");
	addField(arrow::int16(), "gap1");
	addField(arrow::int16(), "gap2");
	addField(arrow::boolean(), "mode");
	addField(arrow::int32(), "data", DATA_PAYLOAD_LIST_COUNT);
	//addField(arrow::int8(), "count");
	addField(arrow::int32(), "txcommwrd");
	addField(arrow::int32(), "rxcommwrd");
	addField(arrow::int8(), "txrtaddr");
	addField(arrow::boolean(), "txtr");
	addField(arrow::int8(), "txsubaddr");
	addField(arrow::int8(), "txwrdcnt");
	addField(arrow::int8(), "rxrtaddr");
	addField(arrow::boolean(), "rxtr");
	addField(arrow::int8(), "rxsubaddr");
	addField(arrow::int8(), "rxwrdcnt");
	addField(arrow::int8(), "payloadwrdcnt");
	//addField(arrow::int16(), "msglen");

	// Set memory locations.
	setMemoryLocation<uint64_t>(time_stamp_, "time");
	setMemoryLocation<uint8_t>(doy_, "doy");
	setMemoryLocation<int8_t>(ttb_, "ttb");
	setMemoryLocation<uint8_t>(WE_, "WE");
	setMemoryLocation<uint8_t>(SE_, "SE");
	setMemoryLocation<uint8_t>(WCE_, "WCE");
	setMemoryLocation<uint8_t>(TO_, "TO");
	setMemoryLocation<uint8_t>(FE_, "FE");
	setMemoryLocation<uint8_t>(RR_, "RR");
	setMemoryLocation<uint8_t>(ME_, "ME");
	setMemoryLocation<uint8_t>(gap1_, "gap1");
	setMemoryLocation<uint8_t>(gap2_, "gap2");
	setMemoryLocation<uint8_t>(mode_code_, "mode");
	setMemoryLocation<uint16_t>(data_, "data");
	//setMemoryLocation<int8_t>(word_count_, "count");
	setMemoryLocation<uint16_t>(comm_word1_, "txcommwrd");
	setMemoryLocation<uint16_t>(comm_word2_, "rxcommwrd");
	setMemoryLocation<int8_t>(rtaddr1_, "txrtaddr");
	setMemoryLocation<uint8_t>(tr1_, "txtr");
	setMemoryLocation<int8_t>(subaddr1_, "txsubaddr");
	setMemoryLocation<int8_t>(wrdcnt1_, "txwrdcnt");
	setMemoryLocation<int8_t>(rtaddr2_, "rxrtaddr");
	setMemoryLocation<uint8_t>(tr2_, "rxtr");
	setMemoryLocation<int8_t>(subaddr2_, "rxsubaddr");
	setMemoryLocation<int8_t>(wrdcnt2_, "rxwrdcnt");
	setMemoryLocation<uint16_t>(channel_id_, "channelid");
	setMemoryLocation<int8_t>(payload_count_, "payloadwrdcnt");
	//setMemoryLocation<int16_t>(msglen_, "msglen");

	uint8_t ret = open_for_write(outfile, truncate);
}

void ParquetMilStd1553F1::append_data(const uint64_t& time_stamp, uint8_t doy, const char* name,
	const MilStd1553F1ChanSpecFormat* chan_spec, const MilStd1553F1MsgCommWord* msg, 
	const uint16_t* data, const uint16_t& chanid, int8_t totwrdcnt, 
	int8_t calcwrdcnt, uint8_t payload_incomplete)
{
	WE_[temp_element_count_] = msg->WE;
	SE_[temp_element_count_] = msg->SE;
	WCE_[temp_element_count_] = msg->WCE;
	TO_[temp_element_count_] = msg->TO;
	FE_[temp_element_count_] = msg->FE;
	RR_[temp_element_count_] = msg->RR;
	ME_[temp_element_count_] = msg->ME;
	gap1_[temp_element_count_] = msg->gap1;
	gap2_[temp_element_count_] = msg->gap2;
	ttb_[temp_element_count_] = chan_spec->ttb;
	doy_[temp_element_count_] = doy;
	time_stamp_[temp_element_count_] = time_stamp;
	//msglen_[temp_element_count_] = msg->length;

	// Get full command words. Intepret the MilStd1553MsgCommword pointer
	// as a uint16_t pointer. 48 bits (= 3 * uint16_t) later is the beginning
	// of the data where the command words are located.
	commword_ptr_ = ((uint16_t*)msg) + 3;

	// Set the TX/RX command words and their data components differently
	// according to the message type.
	if (msg->RR)
	{
		// RT to RT, [ RX ][ TX ][ TX STAT ][ DATA0 ] ... [ DATAN ][ RX STAT ]
		comm_word1_[temp_element_count_] = commword_ptr_[1];
		comm_word2_[temp_element_count_] = commword_ptr_[0];

		rtaddr1_[temp_element_count_] = msg->remote_addr2;
		tr1_[temp_element_count_] = msg->tx2;
		subaddr1_[temp_element_count_] = msg->sub_addr2;
		wrdcnt1_[temp_element_count_] = msg->word_count2;

		rtaddr2_[temp_element_count_] = msg->remote_addr1;
		tr2_[temp_element_count_] = msg->tx1;
		subaddr2_[temp_element_count_] = msg->sub_addr1;
		wrdcnt2_[temp_element_count_] = msg->word_count1;
	}
	else
	{
		if (msg->tx1)
		{
			// RT to BC, [ TX ][ STAT ][ DATA0 ] ... [ DATAN ]
			comm_word1_[temp_element_count_] = commword_ptr_[0];
			comm_word2_[temp_element_count_] = 0;

			rtaddr1_[temp_element_count_] = msg->remote_addr1;
			tr1_[temp_element_count_] = msg->tx1;
			subaddr1_[temp_element_count_] = msg->sub_addr1;
			wrdcnt1_[temp_element_count_] = msg->word_count1;

			rtaddr2_[temp_element_count_] = 0;
			tr2_[temp_element_count_] = 0;
			subaddr2_[temp_element_count_] = 0;
			wrdcnt2_[temp_element_count_] = 0;
		}
		else
		{
			// BC to RT, [ RX ][ DATA0 ] ... [ DATAN ][ STAT ]
			comm_word1_[temp_element_count_] = 0;
			comm_word2_[temp_element_count_] = commword_ptr_[0];

			rtaddr1_[temp_element_count_] = 0;
			tr1_[temp_element_count_] = 0;
			subaddr1_[temp_element_count_] = 0;
			wrdcnt1_[temp_element_count_] = 0;

			rtaddr2_[temp_element_count_] = msg->remote_addr1;
			tr2_[temp_element_count_] = msg->tx1;
			subaddr2_[temp_element_count_] = msg->sub_addr1;
			wrdcnt2_[temp_element_count_] = msg->word_count1;
		}
	}
	
	channel_id_[temp_element_count_] = chanid;
	payload_count_[temp_element_count_] = payload_count;

	// If the calculated word count is less than or equal to zero,
	// do not copy any data. The payload for the current row shall
	// remain all zeros.
	if (calcwrdcnt > 0)
	{
		std::copy(data, data + calcwrdcnt, data_.data() + temp_element_count_ * DATA_PAYLOAD_LIST_COUNT);
	}

	// Check for mode code.
	if (msg->sub_addr1 > 0 && msg->sub_addr1 < 31)
		mode_code_[temp_element_count_] = 0;
	else
		mode_code_[temp_element_count_] = 1;

	// Increment the count variable.
	temp_element_count_++;

	if (temp_element_count_ == max_temp_element_count_)
	{
#ifdef DEBUG
#if DEBUG > 0	
		printf("(%03u) Writing MilStd1553F1 to Parquet, %d rows\n", id_, temp_element_count_);
#endif
#endif
		for (int i = 0; i < DEFAULT_BUFFER_SIZE_MULTIPLIER; i++)
		{
			//printf("write offset %d\n", i * DEFAULT_ROW_GROUP_COUNT);
			writeColumns(DEFAULT_ROW_GROUP_COUNT, i * DEFAULT_ROW_GROUP_COUNT);
		}

		// Set all of the data_ values to zero to ensure that only word_count_ 
		// values in each set of 32 are non-zero.
		std::fill(data_.begin(), data_.end(), 0);

		temp_element_count_ = 0;
	}
}

void ParquetMilStd1553F1::commit()
{
#ifdef DEBUG
#if DEBUG > 0
	printf("(%03u) ParquetMilStd1553F1::commit(): temp_element_count_ = %d\n", id_, temp_element_count_);
#endif
#endif

	if (temp_element_count_ > 0)
	{
		//printf("(%03u) ParquetMilStd1553F1::commit(): total remaining to write = %u\n", id_, temp_element_count_);
		int n_calls = int(std::ceil(double(temp_element_count_) / double(DEFAULT_ROW_GROUP_COUNT)));
		for (int i = 0; i < n_calls; i++)
		{
			if (i == n_calls - 1)
			{
				writeColumns(temp_element_count_ - (n_calls - 1)*DEFAULT_ROW_GROUP_COUNT, i * DEFAULT_ROW_GROUP_COUNT);
				/*printf("(%03u) ParquetMilStd1553F1::commit(): write %u\n", id_,
					temp_element_count_ - (n_calls - 1) * DEFAULT_ROW_GROUP_COUNT);*/
			}
			else
			{
				writeColumns(DEFAULT_ROW_GROUP_COUNT, i*DEFAULT_ROW_GROUP_COUNT);
				//printf("(%03u) ParquetMilStd1553F1::commit(): write %u\n", id_, DEFAULT_ROW_GROUP_COUNT);
			}
		}
		
		std::fill(data_.begin(), data_.end(), 0);
	}
}

void ParquetMilStd1553F1::add_names_to_set(std::set<std::string>& output_set)
{
	for (std::set<std::string>::iterator it = name_set_.begin(); it != name_set_.end(); ++it)
		output_set.insert(*it);
}