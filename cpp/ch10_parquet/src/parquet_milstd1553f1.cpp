#include "parquet_milstd1553f1.h"

ParquetMilStd1553F1::ParquetMilStd1553F1() : max_temp_element_count_(DEFAULT_ROW_GROUP_COUNT * DEFAULT_BUFFER_SIZE_MULTIPLIER),
                                             ParquetContext(DEFAULT_ROW_GROUP_COUNT),
                                             thread_id_(UINT16_MAX),
                                             commword_ptr_(nullptr)
{
}

bool ParquetMilStd1553F1::Initialize(const ManagedPath& outfile, uint16_t thread_id)
{
    thread_id_ = thread_id;

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
    AddField(arrow::int64(), "time");
    AddField(arrow::boolean(), "doy");
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
    AddField(arrow::int8(), "totwrdcnt");
    AddField(arrow::int8(), "calcwrdcnt");
    AddField(arrow::boolean(), "incomplete");

    // Set memory locations.
    SetMemoryLocation(time_stamp_, "time");
    SetMemoryLocation(doy_, "doy");
    SetMemoryLocation(ttb_, "ttb");
    SetMemoryLocation(WE_, "WE");
    SetMemoryLocation(SE_, "SE");
    SetMemoryLocation(WCE_, "WCE");
    SetMemoryLocation(TO_, "TO");
    SetMemoryLocation(FE_, "FE");
    SetMemoryLocation(RR_, "RR");
    SetMemoryLocation(ME_, "ME");
    SetMemoryLocation(gap1_, "gap1");
    SetMemoryLocation(gap2_, "gap2");
    SetMemoryLocation(mode_code_, "mode");
    SetMemoryLocation(data_, "data");
    SetMemoryLocation(comm_word1_, "txcommwrd");
    SetMemoryLocation(comm_word2_, "rxcommwrd");
    SetMemoryLocation(rtaddr1_, "txrtaddr");
    SetMemoryLocation(tr1_, "txtr");
    SetMemoryLocation(subaddr1_, "txsubaddr");
    SetMemoryLocation(wrdcnt1_, "txwrdcnt");
    SetMemoryLocation(rtaddr2_, "rxrtaddr");
    SetMemoryLocation(tr2_, "rxtr");
    SetMemoryLocation(subaddr2_, "rxsubaddr");
    SetMemoryLocation(wrdcnt2_, "rxwrdcnt");
    SetMemoryLocation(channel_id_, "channelid");
    SetMemoryLocation(totwrdcnt_, "totwrdcnt");
    SetMemoryLocation(calcwrdcnt_, "calcwrdcnt");
    SetMemoryLocation(payload_incomplete_, "incomplete");

    if (!OpenForWrite(outfile.string(), true))
    {
        SPDLOG_ERROR("({:03d}) OpenForWrite failed for file {:s}", thread_id_,
                     outfile.string());
        return false;
    }

    // Setup automatic tracking of appended data.
    if (!SetupRowCountTracking(DEFAULT_ROW_GROUP_COUNT,
                               DEFAULT_BUFFER_SIZE_MULTIPLIER, true, "MilStd1553F1"))
    {
        SPDLOG_ERROR("({:03d}) SetupRowCountTracking not configured correctly",
                     thread_id_);
        return false;
    }

    EnableEmptyFileDeletion(outfile.string());
    return true;
}

void ParquetMilStd1553F1::Append(const uint64_t& time_stamp, uint8_t doy,
                                 const MilStd1553F1CSDWFmt* const chan_spec,
                                 const MilStd1553F1DataHeaderCommWordFmt* msg, const uint16_t* const data,
                                 const uint16_t& chanid, int8_t calcwrdcnt, uint8_t payload_incomplete)
{
    WE_[append_count_] = msg->WE;
    SE_[append_count_] = msg->SE;
    WCE_[append_count_] = msg->WCE;
    TO_[append_count_] = msg->TO;
    FE_[append_count_] = msg->FE;
    RR_[append_count_] = msg->RR;
    ME_[append_count_] = msg->ME;
    gap1_[append_count_] = static_cast<int16_t>(msg->gap1);
    gap2_[append_count_] = static_cast<int16_t>(msg->gap2);
    ttb_[append_count_] = chan_spec->ttb;
    doy_[append_count_] = doy;
    time_stamp_[append_count_] = static_cast<int64_t>(time_stamp);

    // Get full command words. Intepret the MilStd1553MsgCommword pointer
    // as a uint16_t pointer. 48 bits (= 3 * uint16_t) later is the beginning
    // of the data where the command words are located.
    commword_ptr_ = reinterpret_cast<const uint16_t*>(msg) + 3;

    // Set the TX/RX command words and their data components differently
    // according to the message type.
    if (msg->RR)
    {
        // RT to RT, [ RX ][ TX ][ TX STAT ][ DATA0 ] ... [ DATAN ][ RX STAT ]
        comm_word1_[append_count_] = static_cast<int32_t>(commword_ptr_[1]);
        comm_word2_[append_count_] = static_cast<int32_t>(commword_ptr_[0]);

        rtaddr1_[append_count_] = static_cast<int8_t>(msg->remote_addr2);
        tr1_[append_count_] = msg->tx2;
        subaddr1_[append_count_] = static_cast<int8_t>(msg->sub_addr2);
        wrdcnt1_[append_count_] = static_cast<int8_t>(msg->word_count2);

        rtaddr2_[append_count_] = static_cast<int8_t>(msg->remote_addr1);
        tr2_[append_count_] = msg->tx1;
        subaddr2_[append_count_] = static_cast<int8_t>(msg->sub_addr1);
        wrdcnt2_[append_count_] = static_cast<int8_t>(msg->word_count1);
    }
    else
    {
        if (msg->tx1)
        {
            // RT to BC, [ TX ][ STAT ][ DATA0 ] ... [ DATAN ]
            comm_word1_[append_count_] = static_cast<int32_t>(commword_ptr_[0]);
            comm_word2_[append_count_] = 0;

            rtaddr1_[append_count_] = static_cast<int8_t>(msg->remote_addr1);
            tr1_[append_count_] = msg->tx1;
            subaddr1_[append_count_] = static_cast<int8_t>(msg->sub_addr1);
            wrdcnt1_[append_count_] = static_cast<int8_t>(msg->word_count1);

            rtaddr2_[append_count_] = 0;
            tr2_[append_count_] = 0;
            subaddr2_[append_count_] = 0;
            wrdcnt2_[append_count_] = 0;
        }
        else
        {
            // BC to RT, [ RX ][ DATA0 ] ... [ DATAN ][ STAT ]
            comm_word1_[append_count_] = 0;
            comm_word2_[append_count_] = static_cast<int32_t>(commword_ptr_[0]);

            rtaddr1_[append_count_] = 0;
            tr1_[append_count_] = 0;
            subaddr1_[append_count_] = 0;
            wrdcnt1_[append_count_] = 0;

            rtaddr2_[append_count_] = static_cast<int8_t>(msg->remote_addr1);
            tr2_[append_count_] = msg->tx1;
            subaddr2_[append_count_] = static_cast<int8_t>(msg->sub_addr1);
            wrdcnt2_[append_count_] = static_cast<int8_t>(msg->word_count1);
        }
    }

    channel_id_[append_count_] = static_cast<int32_t>(chanid);
    totwrdcnt_[append_count_] = msg->length / 2;
    calcwrdcnt_[append_count_] = calcwrdcnt;
    payload_incomplete_[append_count_] = payload_incomplete;

    // If the calculated word count is less than or equal to zero,
    // do not copy any data. The payload for the current row shall
    // remain all zeros.
    if (calcwrdcnt > 0)
    {
        std::copy(data, data + calcwrdcnt, data_.data() + append_count_ * DATA_PAYLOAD_LIST_COUNT);
    }

    // Check for mode code.
    if (msg->sub_addr1 > 0 && msg->sub_addr1 < 31)
        mode_code_[append_count_] = 0;
    else
        mode_code_[append_count_] = 1;

    // Increment the count variable and write data if row group(s) are filled.
    if (IncrementAndWrite(thread_id_))
    {
        // Reset list buffers.
        std::fill(data_.begin(), data_.end(), 0);
    }
}
