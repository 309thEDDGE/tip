#include "parquet_milstd1553f1.h"

const int ParquetMilStd1553F1::DEFAULT_ROW_GROUP_COUNT = 10000;
const int ParquetMilStd1553F1::DEFAULT_BUFFER_SIZE_MULTIPLIER = 10;
const int ParquetMilStd1553F1::DATA_PAYLOAD_LIST_COUNT = 32;

ParquetMilStd1553F1::ParquetMilStd1553F1(ParquetContext* parquet_context) : 
                                             pq_ctx_(parquet_context),
                                             max_temp_element_count_(DEFAULT_ROW_GROUP_COUNT * DEFAULT_BUFFER_SIZE_MULTIPLIER),
                                             thread_id_(UINT16_MAX),
                                             commword_ptr_(nullptr), outfile_("")
{
}

bool ParquetMilStd1553F1::Initialize(const ManagedPath& outfile, uint16_t thread_id)
{
    thread_id_ = thread_id;
    outfile_ = outfile.string();

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
    totwrdcnt_.resize(max_temp_element_count_);
    calcwrdcnt_.resize(max_temp_element_count_);
    payload_incomplete_.resize(max_temp_element_count_);

    // Add fields to table.
    pq_ctx_->AddField(arrow::int64(), "time");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::boolean(), "doy");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::int32(), "channelid");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::int8(), "ttb");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::boolean(), "WE");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::boolean(), "SE");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::boolean(), "WCE");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::boolean(), "TO");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::boolean(), "FE");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::boolean(), "RR");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::boolean(), "ME");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::int16(), "gap1");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::int16(), "gap2");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::boolean(), "mode");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::int32(), "data", DATA_PAYLOAD_LIST_COUNT);  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::int32(), "txcommwrd");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::int32(), "rxcommwrd");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::int8(), "txrtaddr");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::boolean(), "txtr");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::int8(), "txsubaddr");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::int8(), "txwrdcnt");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::int8(), "rxrtaddr");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::boolean(), "rxtr");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::int8(), "rxsubaddr");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::int8(), "rxwrdcnt");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::int8(), "totwrdcnt");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::int8(), "calcwrdcnt");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::boolean(), "incomplete");  // GCOVR_EXCL_LINE

    // Set memory locations.
    pq_ctx_->SetMemoryLocation(time_stamp_, "time");
    pq_ctx_->SetMemoryLocation(doy_, "doy");
    pq_ctx_->SetMemoryLocation(ttb_, "ttb");
    pq_ctx_->SetMemoryLocation(WE_, "WE");
    pq_ctx_->SetMemoryLocation(SE_, "SE");
    pq_ctx_->SetMemoryLocation(WCE_, "WCE");
    pq_ctx_->SetMemoryLocation(TO_, "TO");
    pq_ctx_->SetMemoryLocation(FE_, "FE");
    pq_ctx_->SetMemoryLocation(RR_, "RR");
    pq_ctx_->SetMemoryLocation(ME_, "ME");
    pq_ctx_->SetMemoryLocation(gap1_, "gap1");
    pq_ctx_->SetMemoryLocation(gap2_, "gap2");
    pq_ctx_->SetMemoryLocation(mode_code_, "mode");
    pq_ctx_->SetMemoryLocation(data_, "data");
    pq_ctx_->SetMemoryLocation(comm_word1_, "txcommwrd");
    pq_ctx_->SetMemoryLocation(comm_word2_, "rxcommwrd");
    pq_ctx_->SetMemoryLocation(rtaddr1_, "txrtaddr");
    pq_ctx_->SetMemoryLocation(tr1_, "txtr");
    pq_ctx_->SetMemoryLocation(subaddr1_, "txsubaddr");
    pq_ctx_->SetMemoryLocation(wrdcnt1_, "txwrdcnt");
    pq_ctx_->SetMemoryLocation(rtaddr2_, "rxrtaddr");
    pq_ctx_->SetMemoryLocation(tr2_, "rxtr");
    pq_ctx_->SetMemoryLocation(subaddr2_, "rxsubaddr");
    pq_ctx_->SetMemoryLocation(wrdcnt2_, "rxwrdcnt");
    pq_ctx_->SetMemoryLocation(channel_id_, "channelid");
    pq_ctx_->SetMemoryLocation(totwrdcnt_, "totwrdcnt");
    pq_ctx_->SetMemoryLocation(calcwrdcnt_, "calcwrdcnt");
    pq_ctx_->SetMemoryLocation(payload_incomplete_, "incomplete");

    if (!pq_ctx_->OpenForWrite(outfile_, true))
    {
        SPDLOG_ERROR("({:03d}) OpenForWrite failed for file {:s}", thread_id_,
                     outfile_);
        return false;
    }

    // Setup automatic tracking of appended data.
    if (!pq_ctx_->SetupRowCountTracking(DEFAULT_ROW_GROUP_COUNT,
                               DEFAULT_BUFFER_SIZE_MULTIPLIER, true, "MilStd1553F1"))
    {
        SPDLOG_ERROR("({:03d}) SetupRowCountTracking not configured correctly",
                     thread_id_);
        return false;
    }

    pq_ctx_->EnableEmptyFileDeletion(outfile_);
    return true;
}

void ParquetMilStd1553F1::Append(const uint64_t& time_stamp, uint8_t doy,
                                 const MilStd1553F1CSDWFmt* const chan_spec,
                                 const MilStd1553F1DataHeaderCommWordFmt* msg, const uint16_t* const data,
                                 const uint16_t& chanid, int8_t calcwrdcnt, uint8_t payload_incomplete)
{
    WE_[pq_ctx_->append_count_] = msg->WE;
    SE_[pq_ctx_->append_count_] = msg->SE;
    WCE_[pq_ctx_->append_count_] = msg->WCE;
    TO_[pq_ctx_->append_count_] = msg->TO;
    FE_[pq_ctx_->append_count_] = msg->FE;
    RR_[pq_ctx_->append_count_] = msg->RR;
    ME_[pq_ctx_->append_count_] = msg->ME;
    gap1_[pq_ctx_->append_count_] = static_cast<int16_t>(msg->gap1);
    gap2_[pq_ctx_->append_count_] = static_cast<int16_t>(msg->gap2);
    ttb_[pq_ctx_->append_count_] = chan_spec->ttb;
    doy_[pq_ctx_->append_count_] = doy;
    time_stamp_[pq_ctx_->append_count_] = static_cast<int64_t>(time_stamp);

    // Get full command words. Intepret the MilStd1553MsgCommword pointer
    // as a uint16_t pointer. 48 bits (= 3 * uint16_t) later is the beginning
    // of the data where the command words are located.
    commword_ptr_ = reinterpret_cast<const uint16_t*>(msg) + 3;

    // Set the TX/RX command words and their data components differently
    // according to the message type.
    if (msg->RR)
    {
        // RT to RT, [ RX ][ TX ][ TX STAT ][ DATA0 ] ... [ DATAN ][ RX STAT ]
        comm_word1_[pq_ctx_->append_count_] = static_cast<int32_t>(commword_ptr_[1]);
        comm_word2_[pq_ctx_->append_count_] = static_cast<int32_t>(commword_ptr_[0]);

        rtaddr1_[pq_ctx_->append_count_] = static_cast<int8_t>(msg->remote_addr2);
        tr1_[pq_ctx_->append_count_] = msg->tx2;
        subaddr1_[pq_ctx_->append_count_] = static_cast<int8_t>(msg->sub_addr2);
        wrdcnt1_[pq_ctx_->append_count_] = static_cast<int8_t>(msg->word_count2);

        rtaddr2_[pq_ctx_->append_count_] = static_cast<int8_t>(msg->remote_addr1);
        tr2_[pq_ctx_->append_count_] = msg->tx1;
        subaddr2_[pq_ctx_->append_count_] = static_cast<int8_t>(msg->sub_addr1);
        wrdcnt2_[pq_ctx_->append_count_] = static_cast<int8_t>(msg->word_count1);
    }
    else
    {
        if (msg->tx1)
        {
            // RT to BC, [ TX ][ STAT ][ DATA0 ] ... [ DATAN ]
            comm_word1_[pq_ctx_->append_count_] = static_cast<int32_t>(commword_ptr_[0]);
            comm_word2_[pq_ctx_->append_count_] = 0;

            rtaddr1_[pq_ctx_->append_count_] = static_cast<int8_t>(msg->remote_addr1);
            tr1_[pq_ctx_->append_count_] = msg->tx1;
            subaddr1_[pq_ctx_->append_count_] = static_cast<int8_t>(msg->sub_addr1);
            wrdcnt1_[pq_ctx_->append_count_] = static_cast<int8_t>(msg->word_count1);

            rtaddr2_[pq_ctx_->append_count_] = 0;
            tr2_[pq_ctx_->append_count_] = 0;
            subaddr2_[pq_ctx_->append_count_] = 0;
            wrdcnt2_[pq_ctx_->append_count_] = 0;
        }
        else
        {
            // BC to RT, [ RX ][ DATA0 ] ... [ DATAN ][ STAT ]
            comm_word1_[pq_ctx_->append_count_] = 0;
            comm_word2_[pq_ctx_->append_count_] = static_cast<int32_t>(commword_ptr_[0]);

            rtaddr1_[pq_ctx_->append_count_] = 0;
            tr1_[pq_ctx_->append_count_] = 0;
            subaddr1_[pq_ctx_->append_count_] = 0;
            wrdcnt1_[pq_ctx_->append_count_] = 0;

            rtaddr2_[pq_ctx_->append_count_] = static_cast<int8_t>(msg->remote_addr1);
            tr2_[pq_ctx_->append_count_] = msg->tx1;
            subaddr2_[pq_ctx_->append_count_] = static_cast<int8_t>(msg->sub_addr1);
            wrdcnt2_[pq_ctx_->append_count_] = static_cast<int8_t>(msg->word_count1);
        }
    }

    channel_id_[pq_ctx_->append_count_] = static_cast<int32_t>(chanid);
    totwrdcnt_[pq_ctx_->append_count_] = msg->length / 2;
    calcwrdcnt_[pq_ctx_->append_count_] = calcwrdcnt;
    payload_incomplete_[pq_ctx_->append_count_] = payload_incomplete;

    // If the calculated word count is less than or equal to zero,
    // do not copy any data. The payload for the current row shall
    // remain all zeros.
    if (calcwrdcnt > 0)
    {
        std::copy(data, data + calcwrdcnt, data_.data() + pq_ctx_->append_count_ * DATA_PAYLOAD_LIST_COUNT);
    }

    // Check for mode code.
    if (msg->sub_addr1 > 0 && msg->sub_addr1 < 31)
        mode_code_[pq_ctx_->append_count_] = 0;
    else
        mode_code_[pq_ctx_->append_count_] = 1;

    // Increment the count variable and write data if row group(s) are filled.
    if (pq_ctx_->IncrementAndWrite(thread_id_))
    {
        // Reset list buffers.
        std::fill(data_.begin(), data_.end(), 0);
    }
}
