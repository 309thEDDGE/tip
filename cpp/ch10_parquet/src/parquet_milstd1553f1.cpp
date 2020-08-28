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
	AddField(arrow::int64(), "time");
	AddField(arrow::int16(), "doy");
	AddField(arrow::int32(), "channelid");
	AddField(arrow::int8(), "ttb");
	AddField(arrow::boolean(), "WE");
	AddField(arrow::boolean(), "SE");
	AddField(arrow::boolean(), "WCE");
	AddField(arrow::boolean(), "TO");
	AddField(arrow::boolean(), "FE");
	AddField(arrow::boolean(), "RR");
	AddField(arrow::boolean(), "ME");
	AddField(arrow::int16(), "gap1");
	AddField(arrow::int16(), "gap2");
	AddField(arrow::boolean(), "mode");
	AddField(arrow::int32(), "data", DATA_PAYLOAD_LIST_COUNT);
	//AddField(arrow::int8(), "count");
	AddField(arrow::int32(), "txcommwrd");
	AddField(arrow::int32(), "rxcommwrd");
	AddField(arrow::int8(), "txrtaddr");
	AddField(arrow::boolean(), "txtr");
	AddField(arrow::int8(), "txsubaddr");
	AddField(arrow::int8(), "txwrdcnt");
	AddField(arrow::int8(), "rxrtaddr");
	AddField(arrow::boolean(), "rxtr");
	AddField(arrow::int8(), "rxsubaddr");
	AddField(arrow::int8(), "rxwrdcnt");
	AddField(arrow::int8(), "payloadwrdcnt");
	//AddField(arrow::int16(), "msglen");

	// Set memory locations.
	SetMemoryLocation<uint64_t>(time_stamp_, "time");
	SetMemoryLocation<uint8_t>(doy_, "doy");
	SetMemoryLocation<int8_t>(ttb_, "ttb");
	SetMemoryLocation<uint8_t>(WE_, "WE");
	SetMemoryLocation<uint8_t>(SE_, "SE");
	SetMemoryLocation<uint8_t>(WCE_, "WCE");
	SetMemoryLocation<uint8_t>(TO_, "TO");
	SetMemoryLocation<uint8_t>(FE_, "FE");
	SetMemoryLocation<uint8_t>(RR_, "RR");
	SetMemoryLocation<uint8_t>(ME_, "ME");
	SetMemoryLocation<uint8_t>(gap1_, "gap1");
	SetMemoryLocation<uint8_t>(gap2_, "gap2");
	SetMemoryLocation<uint8_t>(mode_code_, "mode");
	SetMemoryLocation<uint16_t>(data_, "data");
	//SetMemoryLocation<int8_t>(word_count_, "count");
	SetMemoryLocation<uint16_t>(comm_word1_, "txcommwrd");
	SetMemoryLocation<uint16_t>(comm_word2_, "rxcommwrd");
	SetMemoryLocation<int8_t>(rtaddr1_, "txrtaddr");
	SetMemoryLocation<uint8_t>(tr1_, "txtr");
	SetMemoryLocation<int8_t>(subaddr1_, "txsubaddr");
	SetMemoryLocation<int8_t>(wrdcnt1_, "txwrdcnt");
	SetMemoryLocation<int8_t>(rtaddr2_, "rxrtaddr");
	SetMemoryLocation<uint8_t>(tr2_, "rxtr");
	SetMemoryLocation<int8_t>(subaddr2_, "rxsubaddr");
	SetMemoryLocation<int8_t>(wrdcnt2_, "rxwrdcnt");
	SetMemoryLocation<uint16_t>(channel_id_, "channelid");
	SetMemoryLocation<int8_t>(payload_count_, "payloadwrdcnt");
	//SetMemoryLocation<int16_t>(msglen_, "msglen");

	uint8_t ret = OpenForWrite(outfile, truncate);
}

void ParquetMilStd1553F1::append_data(const uint64_t& time_stamp, uint8_t doy, const char* name,
	const MilStd1553F1ChanSpecFormat* chan_spec, const MilStd1553F1MsgCommWord* msg, 
	const uint16_t* data, const uint16_t& chanid, const uint16_t& payload_count)
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

	// Check for mode code.
	if (msg->sub_addr1 > 0 && msg->sub_addr1 < 31)
	{
		mode_code_[temp_element_count_] = 0;
		if (msg->word_count1 == 0)
		{
			//word_count_[temp_element_count_] = 32;
			std::copy(data, data + 32, data_.data() + temp_element_count_ * DATA_PAYLOAD_LIST_COUNT);
		}
		else
		{
			//word_count_[temp_element_count_] = msg->word_count1;
			std::copy(data, data + msg->word_count1, data_.data() + temp_element_count_ * DATA_PAYLOAD_LIST_COUNT);
		}
	}
	else
	{
		mode_code_[temp_element_count_] = 1;

		// There is the possibility that a mode code message
		// includes a single data payload in addition to the command
		// word (see Section 4.3.3.5.1.7 of MIL-STD-1553B). The single
		// data payload condition can only occur if the mode code, i.e.,
		// word count bits, of a mode code message is > 15. Further,
		// not all mode code messages in the range [16, 31] shall contain
		// a single data payload, so use the payload_count to decide
		// if data ought to be copied.
		if (msg->word_count1 > 15 && payload_count == 1)
		{
			data_[temp_element_count_ * DATA_PAYLOAD_LIST_COUNT] = data[0];
		}

	}

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
			WriteColumns(DEFAULT_ROW_GROUP_COUNT, i * DEFAULT_ROW_GROUP_COUNT);
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
				WriteColumns(temp_element_count_ - (n_calls - 1)*DEFAULT_ROW_GROUP_COUNT, i * DEFAULT_ROW_GROUP_COUNT);
				/*printf("(%03u) ParquetMilStd1553F1::commit(): write %u\n", id_,
					temp_element_count_ - (n_calls - 1) * DEFAULT_ROW_GROUP_COUNT);*/
			}
			else
			{
				WriteColumns(DEFAULT_ROW_GROUP_COUNT, i*DEFAULT_ROW_GROUP_COUNT);
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