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

int ParquetMilStd1553F1::GetRowGroupRowCount()
{
    return DEFAULT_ROW_GROUP_COUNT;
}

int ParquetMilStd1553F1::GetRowGroupBufferCount()
{
    return DEFAULT_BUFFER_SIZE_MULTIPLIER;
}

int ParquetMilStd1553F1::GetDataPayloadListElementCount()
{
    return DATA_PAYLOAD_LIST_COUNT;
}


int ParquetMilStd1553F1::Initialize(const ManagedPath& outfile, uint16_t thread_id)
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

    status_word1_.resize(max_temp_element_count_);
    terminal1_.resize(max_temp_element_count_);
    dynbusctrl1_.resize(max_temp_element_count_);
    subsys1_.resize(max_temp_element_count_);
    busy1_.resize(max_temp_element_count_);
    bcastrcv1_.resize(max_temp_element_count_);
    svcreq1_.resize(max_temp_element_count_);
    instr1_.resize(max_temp_element_count_);
    msgerr1_.resize(max_temp_element_count_);
    status_rtaddr1_.resize(max_temp_element_count_);

    status_word2_.resize(max_temp_element_count_);
    terminal2_.resize(max_temp_element_count_);
    dynbusctrl2_.resize(max_temp_element_count_);
    subsys2_.resize(max_temp_element_count_);
    busy2_.resize(max_temp_element_count_);
    bcastrcv2_.resize(max_temp_element_count_);
    svcreq2_.resize(max_temp_element_count_);
    instr2_.resize(max_temp_element_count_);
    msgerr2_.resize(max_temp_element_count_);
    status_rtaddr2_.resize(max_temp_element_count_);


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

    pq_ctx_->AddField(arrow::int32(), "statwrd1");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::boolean(), "terminal1");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::boolean(), "dynbusctrl1");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::boolean(), "subsys1");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::boolean(), "busy1");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::boolean(), "bcastrcv1");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::boolean(), "svcreq1");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::boolean(), "instr1");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::boolean(), "msgerr1");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::int8(), "status_rtaddr1");  // GCOVR_EXCL_LINE

    pq_ctx_->AddField(arrow::int32(), "statwrd2");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::boolean(), "terminal2");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::boolean(), "dynbusctrl2");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::boolean(), "subsys2");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::boolean(), "busy2");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::boolean(), "bcastrcv2");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::boolean(), "svcreq2");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::boolean(), "instr2");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::boolean(), "msgerr2");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::int8(), "status_rtaddr2");  // GCOVR_EXCL_LINE

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

    pq_ctx_->SetMemoryLocation(status_word1_, "statwrd1");
    pq_ctx_->SetMemoryLocation(terminal1_, "terminal1");
    pq_ctx_->SetMemoryLocation(dynbusctrl1_, "dynbusctrl1");
    pq_ctx_->SetMemoryLocation(subsys1_, "subsys1");
    pq_ctx_->SetMemoryLocation(busy1_, "busy1");
    pq_ctx_->SetMemoryLocation(bcastrcv1_, "bcastrcv1");
    pq_ctx_->SetMemoryLocation(svcreq1_, "svcreq1");
    pq_ctx_->SetMemoryLocation(instr1_, "instr1");
    pq_ctx_->SetMemoryLocation(msgerr1_, "msgerr1");
    pq_ctx_->SetMemoryLocation(status_rtaddr1_, "status_rtaddr1");

    pq_ctx_->SetMemoryLocation(status_word2_, "statwrd2");
    pq_ctx_->SetMemoryLocation(terminal2_, "terminal2");
    pq_ctx_->SetMemoryLocation(dynbusctrl2_, "dynbusctrl2");
    pq_ctx_->SetMemoryLocation(subsys2_, "subsys2");
    pq_ctx_->SetMemoryLocation(busy2_, "busy2");
    pq_ctx_->SetMemoryLocation(bcastrcv2_, "bcastrcv2");
    pq_ctx_->SetMemoryLocation(svcreq2_, "svcreq2");
    pq_ctx_->SetMemoryLocation(instr2_, "instr2");
    pq_ctx_->SetMemoryLocation(msgerr2_, "msgerr2");
    pq_ctx_->SetMemoryLocation(status_rtaddr2_, "status_rtaddr2");


    if (!pq_ctx_->OpenForWrite(outfile_, true))
    {
        SPDLOG_ERROR("({:03d}) OpenForWrite failed for file {:s}", thread_id_,
                     outfile_);
        return EX_IOERR;
    }

    // Setup automatic tracking of appended data.
    if (!pq_ctx_->SetupRowCountTracking(DEFAULT_ROW_GROUP_COUNT,
                               DEFAULT_BUFFER_SIZE_MULTIPLIER, true, "MilStd1553F1"))
    {
        SPDLOG_ERROR("({:03d}) SetupRowCountTracking not configured correctly",
                     thread_id_);
        return EX_SOFTWARE;
    }

    pq_ctx_->EnableEmptyFileDeletion(outfile_);
    return EX_OK;
}

void ParquetMilStd1553F1::Append(const uint64_t& time_stamp, uint8_t doy,
                                 const MilStd1553F1CSDWFmt* const chan_spec,
                                 const MilStd1553F1DataHeaderCommWordFmt* msg, const uint16_t* const data,
                                 const uint16_t& chanid, int8_t calcwrdcnt, uint8_t payload_incomplete, 
                                 const MilStd1553F1StatusWordFmt* statwrd1,
                                 const MilStd1553F1StatusWordFmt* statwrd2)
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

    // Status words will be set to -1 if the status word is not present for the given 
    // message (indicated by nullptr). This is a way to explicitly indicate the 
    // status word is null. All of the decomposed fields will be set to 0.
    if(statwrd1 == nullptr)
    {
        status_word1_[pq_ctx_->append_count_] = -1;
        terminal1_[pq_ctx_->append_count_] = 0;
        dynbusctrl1_[pq_ctx_->append_count_] = 0;
        subsys1_[pq_ctx_->append_count_] = 0;
        busy1_[pq_ctx_->append_count_] = 0;
        bcastrcv1_[pq_ctx_->append_count_] = 0;
        svcreq1_[pq_ctx_->append_count_] = 0;
        instr1_[pq_ctx_->append_count_] = 0;
        msgerr1_[pq_ctx_->append_count_] = 0;
        status_rtaddr1_[pq_ctx_->append_count_] = 0;
    }
    else
    {
        status_word1_[pq_ctx_->append_count_] = static_cast<int32_t>(
            *(reinterpret_cast<const uint16_t*>(statwrd1)));
        terminal1_[pq_ctx_->append_count_] = statwrd1->terminal;
        dynbusctrl1_[pq_ctx_->append_count_] = statwrd1->dynbusctrl;
        subsys1_[pq_ctx_->append_count_] = statwrd1->subsys;
        busy1_[pq_ctx_->append_count_] = statwrd1->busy;
        bcastrcv1_[pq_ctx_->append_count_] = statwrd1->bcastrcv;
        svcreq1_[pq_ctx_->append_count_] = statwrd1->svcreq;
        instr1_[pq_ctx_->append_count_] = statwrd1->instr;
        msgerr1_[pq_ctx_->append_count_] = statwrd1->msgerr;
        status_rtaddr1_[pq_ctx_->append_count_] = statwrd1->rtaddr;
    }

    if(statwrd2 == nullptr)
    {
        status_word2_[pq_ctx_->append_count_] = -1;
        terminal2_[pq_ctx_->append_count_] = 0;
        dynbusctrl2_[pq_ctx_->append_count_] = 0;
        subsys2_[pq_ctx_->append_count_] = 0;
        busy2_[pq_ctx_->append_count_] = 0;
        bcastrcv2_[pq_ctx_->append_count_] = 0;
        svcreq2_[pq_ctx_->append_count_] = 0;
        instr2_[pq_ctx_->append_count_] = 0;
        msgerr2_[pq_ctx_->append_count_] = 0;
        status_rtaddr2_[pq_ctx_->append_count_] = 0;
    }
    else
    {
        status_word2_[pq_ctx_->append_count_] = static_cast<int32_t>(
            *(reinterpret_cast<const uint16_t*>(statwrd2)));
        terminal2_[pq_ctx_->append_count_] = statwrd2->terminal;
        dynbusctrl2_[pq_ctx_->append_count_] = statwrd2->dynbusctrl;
        subsys2_[pq_ctx_->append_count_] = statwrd2->subsys;
        busy2_[pq_ctx_->append_count_] = statwrd2->busy;
        bcastrcv2_[pq_ctx_->append_count_] = statwrd2->bcastrcv;
        svcreq2_[pq_ctx_->append_count_] = statwrd2->svcreq;
        instr2_[pq_ctx_->append_count_] = statwrd2->instr;
        msgerr2_[pq_ctx_->append_count_] = statwrd2->msgerr;
        status_rtaddr2_[pq_ctx_->append_count_] = statwrd2->rtaddr;
    }


    // Increment the count variable and write data if row group(s) are filled.
    if (pq_ctx_->IncrementAndWrite(thread_id_))
    {
        // Reset list buffers.
        std::fill(data_.begin(), data_.end(), 0);
    }
}
