#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "parquet_context.h"
#include "parquet_milstd1553f1.h"
#include "parquet_context_mock.h"

using ::testing::Return;
using ::testing::Exactly;
using ::testing::_;
using ::testing::NiceMock;

class ParquetMilStd1553F1Test : public ::testing::Test
{
   protected:
    NiceMock<MockParquetContext> mock_pq_ctx_;
    ParquetMilStd1553F1 pq1553_;
    ManagedPath outf;
    bool truncate;
    uint16_t thread_id;

    // Initialize parameters that will be passed into Append function
    uint64_t time;
    uint8_t doy;
    uint16_t channel_id;
    int8_t calcwrdcnt;
    uint8_t payload_incomplete;
    uint8_t mode_code;
    MilStd1553F1CSDWFmt chan_spec;
    MilStd1553F1DataHeaderCommWordFmt msg;
    std::vector<uint16_t> data_vec;
    const uint16_t* data; 
    const uint16_t* commword_ptr;

    MilStd1553F1StatusWordFmt statwrd1_;
    MilStd1553F1StatusWordFmt statwrd2_;
    const MilStd1553F1StatusWordFmt* statwrd_ptr1_;
    const MilStd1553F1StatusWordFmt* statwrd_ptr2_;
    const uint16_t* statwrd_value1_;
    const uint16_t* statwrd_value2_; 

    ParquetMilStd1553F1Test() : mock_pq_ctx_(), pq1553_(&mock_pq_ctx_), time(48284949910),
        doy(1), channel_id(9), calcwrdcnt(20), payload_incomplete(0), mode_code(0),
        chan_spec{}, msg{}, data_vec{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 
        15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32}, 
        data(data_vec.data()), commword_ptr(reinterpret_cast<const uint16_t*>(&msg) + 3),
        outf("test.parquet"), truncate(true), thread_id(3), statwrd1_(), statwrd2_(),
        statwrd_ptr1_(&statwrd1_), statwrd_ptr2_(&statwrd2_)
    { 
        chan_spec.ttb = 1;
        msg.WE = 1;
        msg.SE = 1;
        msg.WCE = 1;
        msg.TO = 1;
        msg.FE = 1;
        msg.RR = 1;
        msg.ME = 1;
        msg.tx1 = 1;
        msg.tx2 = 1;
        msg.length = calcwrdcnt * 2; // bytes
        msg.remote_addr1 = 29;
        msg.remote_addr2 = 27;
        msg.sub_addr1 = 2;
        msg.sub_addr2 = 3;
        msg.word_count1 = calcwrdcnt;
        msg.word_count2 = calcwrdcnt;

        statwrd_value1_ = reinterpret_cast<const uint16_t*>(statwrd_ptr1_);
        statwrd_value2_ = reinterpret_cast<const uint16_t*>(statwrd_ptr2_);
    }

    void ValidateInitializeAddField()
    {
        ::testing::Sequence seq;
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "time", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "doy", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "channelid", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "ttb", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "WE", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "SE", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "WCE", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "TO", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "FE", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "RR", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "ME", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "gap1", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "gap2", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "mode", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "data", ParquetMilStd1553F1::DATA_PAYLOAD_LIST_COUNT)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "txcommwrd", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "rxcommwrd", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "txrtaddr", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "txtr", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "txsubaddr", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "txwrdcnt", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "rxrtaddr", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "rxtr", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "rxsubaddr", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "rxwrdcnt", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "totwrdcnt", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "calcwrdcnt", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "incomplete", 0)).InSequence(seq).WillOnce(Return(true));


        EXPECT_CALL(mock_pq_ctx_, AddField(_, "statwrd1", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "terminal1", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "dynbusctrl1", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "subsys1", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "busy1", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "bcastrcv1", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "svcreq1", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "instr1", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "msgerr1", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "status_rtaddr1", 0)).InSequence(seq).WillOnce(Return(true));

        EXPECT_CALL(mock_pq_ctx_, AddField(_, "statwrd2", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "terminal2", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "dynbusctrl2", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "subsys2", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "busy2", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "bcastrcv2", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "svcreq2", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "instr2", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "msgerr2", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "status_rtaddr2", 0)).InSequence(seq).WillOnce(Return(true));

    }

    void ValidateInitializeSetMemoryLocation()
    {
        ::testing::Sequence seq;
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI64(_, "time", nullptr)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "doy", nullptr)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI8(_, "ttb", nullptr)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "WE", nullptr)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "SE", nullptr)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "WCE", nullptr)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "TO", nullptr)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "FE", nullptr)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "RR", nullptr)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "ME", nullptr)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI16(_, "gap1", nullptr)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI16(_, "gap2", nullptr)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "mode", nullptr)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI32(_, "data", nullptr)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI32(_, "txcommwrd", nullptr)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI32(_, "rxcommwrd", nullptr)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI8(_, "txrtaddr", nullptr)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "txtr", nullptr)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI8(_, "txsubaddr", nullptr)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI8(_, "txwrdcnt", nullptr)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI8(_, "rxrtaddr", nullptr)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "rxtr", nullptr)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI8(_, "rxsubaddr", nullptr)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI8(_, "rxwrdcnt", nullptr)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI32(_, "channelid", nullptr)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI8(_, "totwrdcnt", nullptr)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI8(_, "calcwrdcnt", nullptr)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "incomplete", nullptr)).InSequence(seq).WillOnce(Return(true));


        EXPECT_CALL(mock_pq_ctx_, SetMemLocI32(_, "statwrd1", nullptr)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "terminal1", nullptr)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "dynbusctrl1", nullptr)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "subsys1", nullptr)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "busy1", nullptr)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "bcastrcv1", nullptr)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "svcreq1", nullptr)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "instr1", nullptr)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "msgerr1", nullptr)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI8(_, "status_rtaddr1", nullptr)).InSequence(seq).WillOnce(Return(true));

        EXPECT_CALL(mock_pq_ctx_, SetMemLocI32(_, "statwrd2", nullptr)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "terminal2", nullptr)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "dynbusctrl2", nullptr)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "subsys2", nullptr)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "busy2", nullptr)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "bcastrcv2", nullptr)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "svcreq2", nullptr)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "instr2", nullptr)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "msgerr2", nullptr)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI8(_, "status_rtaddr2", nullptr)).InSequence(seq).WillOnce(Return(true));
    }
    
    void ValidateInitializeAddFieldFalse()
    {
        ::testing::Sequence seq;
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "time", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "doy", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "channelid", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "ttb", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "WE", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "SE", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "WCE", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "TO", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "FE", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "RR", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "ME", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "gap1", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "gap2", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "mode", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "data", ParquetMilStd1553F1::DATA_PAYLOAD_LIST_COUNT)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "txcommwrd", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "rxcommwrd", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "txrtaddr", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "txtr", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "txsubaddr", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "txwrdcnt", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "rxrtaddr", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "rxtr", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "rxsubaddr", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "rxwrdcnt", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "totwrdcnt", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "calcwrdcnt", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "incomplete", 0)).InSequence(seq).WillOnce(Return(false));

        EXPECT_CALL(mock_pq_ctx_, AddField(_, "statwrd1", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "terminal1", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "dynbusctrl1", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "subsys1", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "busy1", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "bcastrcv1", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "svcreq1", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "instr1", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "msgerr1", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "status_rtaddr1", 0)).InSequence(seq).WillOnce(Return(false));

        EXPECT_CALL(mock_pq_ctx_, AddField(_, "statwrd2", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "terminal2", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "dynbusctrl2", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "subsys2", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "busy2", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "bcastrcv2", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "svcreq2", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "instr2", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "msgerr2", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "status_rtaddr2", 0)).InSequence(seq).WillOnce(Return(false));

    }

    void ValidateInitializeSetMemoryLocationFalse()
    {
        ::testing::Sequence seq;
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI64(_, "time", nullptr)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "doy", nullptr)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI8(_, "ttb", nullptr)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "WE", nullptr)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "SE", nullptr)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "WCE", nullptr)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "TO", nullptr)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "FE", nullptr)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "RR", nullptr)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "ME", nullptr)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI16(_, "gap1", nullptr)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI16(_, "gap2", nullptr)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "mode", nullptr)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI32(_, "data", nullptr)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI32(_, "txcommwrd", nullptr)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI32(_, "rxcommwrd", nullptr)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI8(_, "txrtaddr", nullptr)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "txtr", nullptr)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI8(_, "txsubaddr", nullptr)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI8(_, "txwrdcnt", nullptr)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI8(_, "rxrtaddr", nullptr)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "rxtr", nullptr)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI8(_, "rxsubaddr", nullptr)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI8(_, "rxwrdcnt", nullptr)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI32(_, "channelid", nullptr)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI8(_, "totwrdcnt", nullptr)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI8(_, "calcwrdcnt", nullptr)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "incomplete", nullptr)).InSequence(seq).WillOnce(Return(false));

        EXPECT_CALL(mock_pq_ctx_, SetMemLocI32(_, "statwrd1", nullptr)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "terminal1", nullptr)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "dynbusctrl1", nullptr)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "subsys1", nullptr)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "busy1", nullptr)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "bcastrcv1", nullptr)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "svcreq1", nullptr)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "instr1", nullptr)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "msgerr1", nullptr)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI8(_, "status_rtaddr1", nullptr)).InSequence(seq).WillOnce(Return(false));

        EXPECT_CALL(mock_pq_ctx_, SetMemLocI32(_, "statwrd2", nullptr)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "terminal2", nullptr)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "dynbusctrl2", nullptr)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "subsys2", nullptr)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "busy2", nullptr)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "bcastrcv2", nullptr)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "svcreq2", nullptr)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "instr2", nullptr)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "msgerr2", nullptr)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI8(_, "status_rtaddr2", nullptr)).InSequence(seq).WillOnce(Return(false));
    }

    void ValidateInitializeResize()
    {
        size_t expected_size = ParquetMilStd1553F1::DEFAULT_ROW_GROUP_COUNT * 
            ParquetMilStd1553F1::DEFAULT_BUFFER_SIZE_MULTIPLIER;
        EXPECT_EQ(expected_size, pq1553_.time_stamp_.size());
        EXPECT_EQ(expected_size, pq1553_.doy_.size());
        EXPECT_EQ(expected_size, pq1553_.ttb_.size());
        EXPECT_EQ(expected_size, pq1553_.WE_.size());
        EXPECT_EQ(expected_size, pq1553_.SE_.size());
        EXPECT_EQ(expected_size, pq1553_.WCE_.size());
        EXPECT_EQ(expected_size, pq1553_.TO_.size());
        EXPECT_EQ(expected_size, pq1553_.FE_.size());
        EXPECT_EQ(expected_size, pq1553_.RR_.size());
        EXPECT_EQ(expected_size, pq1553_.ME_.size());
        EXPECT_EQ(expected_size, pq1553_.gap1_.size());
        EXPECT_EQ(expected_size, pq1553_.gap2_.size());
        EXPECT_EQ(expected_size, pq1553_.mode_code_.size());
        EXPECT_EQ(expected_size * ParquetMilStd1553F1::DATA_PAYLOAD_LIST_COUNT, pq1553_.data_.size());
        EXPECT_EQ(expected_size, pq1553_.comm_word1_.size());
        EXPECT_EQ(expected_size, pq1553_.comm_word2_.size());
        EXPECT_EQ(expected_size, pq1553_.rtaddr1_.size());
        EXPECT_EQ(expected_size, pq1553_.tr1_.size());
        EXPECT_EQ(expected_size, pq1553_.subaddr1_.size());
        EXPECT_EQ(expected_size, pq1553_.wrdcnt1_.size());
        EXPECT_EQ(expected_size, pq1553_.rtaddr1_.size());
        EXPECT_EQ(expected_size, pq1553_.tr2_.size());
        EXPECT_EQ(expected_size, pq1553_.subaddr2_.size());
        EXPECT_EQ(expected_size, pq1553_.wrdcnt2_.size());
        EXPECT_EQ(expected_size, pq1553_.channel_id_.size());
        EXPECT_EQ(expected_size, pq1553_.totwrdcnt_.size());
        EXPECT_EQ(expected_size, pq1553_.calcwrdcnt_.size());
        EXPECT_EQ(expected_size, pq1553_.payload_incomplete_.size());
    }

    void ValidateAppended1553Words(const std::vector<uint16_t>& input_data, 
        const std::vector<int32_t>& appended_data, int8_t calcwrdcnt, 
        size_t offset)
    {
        size_t total_offset = offset * ParquetMilStd1553F1::DATA_PAYLOAD_LIST_COUNT;
        for(int8_t i = 0; i < calcwrdcnt; i++)
        {
            EXPECT_EQ(input_data.at(i), appended_data.at(total_offset + i));
        }
    }

    void ValidateCommonMetadata()
    {
        EXPECT_EQ(msg.WE, pq1553_.WE_.at(0));
        EXPECT_EQ(msg.SE, pq1553_.SE_.at(0));
        EXPECT_EQ(msg.WCE, pq1553_.WCE_.at(0));
        EXPECT_EQ(msg.TO, pq1553_.TO_.at(0));
        EXPECT_EQ(msg.FE, pq1553_.FE_.at(0));
        EXPECT_EQ(msg.RR, pq1553_.RR_.at(0));
        EXPECT_EQ(msg.ME, pq1553_.ME_.at(0));
        EXPECT_EQ(msg.gap1, pq1553_.gap1_.at(0));
        EXPECT_EQ(msg.gap2, pq1553_.gap2_.at(0));
        EXPECT_EQ(chan_spec.ttb, pq1553_.ttb_.at(0));
        EXPECT_EQ(doy, pq1553_.doy_.at(0));
        EXPECT_EQ(time, pq1553_.time_stamp_.at(0));
        EXPECT_EQ(commword_ptr, pq1553_.commword_ptr_);
        EXPECT_EQ(channel_id, pq1553_.channel_id_.at(0));
        EXPECT_EQ(msg.length / 2, pq1553_.totwrdcnt_.at(0));
        EXPECT_EQ(calcwrdcnt, pq1553_.calcwrdcnt_.at(0));
        EXPECT_EQ(payload_incomplete, pq1553_.payload_incomplete_.at(0));
        EXPECT_EQ(mode_code, pq1553_.mode_code_.at(0));
    }

    void ValidateAppendedMetadataRTtoRT()
    {
        ValidateCommonMetadata();
        EXPECT_EQ(commword_ptr[1], pq1553_.comm_word1_.at(0));
        EXPECT_EQ(commword_ptr[0], pq1553_.comm_word2_.at(0));
        EXPECT_EQ(msg.remote_addr2, pq1553_.rtaddr1_.at(0));
        EXPECT_EQ(msg.remote_addr1, pq1553_.rtaddr2_.at(0));
        EXPECT_EQ(msg.sub_addr1, pq1553_.subaddr2_.at(0));
        EXPECT_EQ(msg.sub_addr2, pq1553_.subaddr1_.at(0));
        EXPECT_EQ(msg.word_count1, pq1553_.wrdcnt2_.at(0));
        EXPECT_EQ(msg.word_count2, pq1553_.wrdcnt1_.at(0));
    }

    void ValidateAppendedMetadataRTtoBC()
    {
        ValidateCommonMetadata();
        EXPECT_EQ(commword_ptr[0], pq1553_.comm_word1_.at(0));
        EXPECT_EQ(0, pq1553_.comm_word2_.at(0));
        EXPECT_EQ(msg.remote_addr1, pq1553_.rtaddr1_.at(0));
        EXPECT_EQ(0, pq1553_.rtaddr2_.at(0));
        EXPECT_EQ(0, pq1553_.subaddr2_.at(0));
        EXPECT_EQ(msg.sub_addr1, pq1553_.subaddr1_.at(0));
        EXPECT_EQ(0, pq1553_.wrdcnt2_.at(0));
        EXPECT_EQ(msg.word_count1, pq1553_.wrdcnt1_.at(0));
    }

    void ValidateAppendedMetadataBCtoRT()
    {
        ValidateCommonMetadata();
        EXPECT_EQ(0, pq1553_.comm_word1_.at(0));
        EXPECT_EQ(commword_ptr[0], pq1553_.comm_word2_.at(0));
        EXPECT_EQ(0, pq1553_.rtaddr1_.at(0));
        EXPECT_EQ(msg.remote_addr1, pq1553_.rtaddr2_.at(0));
        EXPECT_EQ(msg.sub_addr1, pq1553_.subaddr2_.at(0));
        EXPECT_EQ(0, pq1553_.subaddr1_.at(0));
        EXPECT_EQ(msg.word_count1, pq1553_.wrdcnt2_.at(0));
        EXPECT_EQ(0, pq1553_.wrdcnt1_.at(0));
    }

    void ValidateStatusWord1()
    {
        EXPECT_EQ(statwrd1_.terminal, pq1553_.terminal1_.at(0));
        EXPECT_EQ(statwrd1_.dynbusctrl, pq1553_.dynbusctrl1_.at(0));
        EXPECT_EQ(statwrd1_.subsys, pq1553_.subsys1_.at(0));
        EXPECT_EQ(statwrd1_.busy, pq1553_.busy1_.at(0));
        EXPECT_EQ(statwrd1_.bcastrcv, pq1553_.bcastrcv1_.at(0));
        EXPECT_EQ(statwrd1_.svcreq, pq1553_.svcreq1_.at(0));
        EXPECT_EQ(statwrd1_.instr, pq1553_.instr1_.at(0));
        EXPECT_EQ(statwrd1_.msgerr, pq1553_.msgerr1_.at(0));
        EXPECT_EQ(statwrd1_.rtaddr, pq1553_.status_rtaddr1_.at(0));
        EXPECT_EQ(*statwrd_value1_, pq1553_.status_word1_.at(0));
    }

    void ValidateStatusWord2()
    {
        EXPECT_EQ(statwrd2_.terminal, pq1553_.terminal2_.at(0));
        EXPECT_EQ(statwrd2_.dynbusctrl, pq1553_.dynbusctrl2_.at(0));
        EXPECT_EQ(statwrd2_.subsys, pq1553_.subsys2_.at(0));
        EXPECT_EQ(statwrd2_.busy, pq1553_.busy2_.at(0));
        EXPECT_EQ(statwrd2_.bcastrcv, pq1553_.bcastrcv2_.at(0));
        EXPECT_EQ(statwrd2_.svcreq, pq1553_.svcreq2_.at(0));
        EXPECT_EQ(statwrd2_.instr, pq1553_.instr2_.at(0));
        EXPECT_EQ(statwrd2_.msgerr, pq1553_.msgerr2_.at(0));
        EXPECT_EQ(statwrd2_.rtaddr, pq1553_.status_rtaddr2_.at(0));
        EXPECT_EQ(*statwrd_value2_, pq1553_.status_word2_.at(0));
    }

    void ValidateStatusWord1Null()
    {
        EXPECT_EQ(0, pq1553_.terminal1_.at(0));
        EXPECT_EQ(0, pq1553_.dynbusctrl1_.at(0));
        EXPECT_EQ(0, pq1553_.subsys1_.at(0));
        EXPECT_EQ(0, pq1553_.busy1_.at(0));
        EXPECT_EQ(0, pq1553_.bcastrcv1_.at(0));
        EXPECT_EQ(0, pq1553_.svcreq1_.at(0));
        EXPECT_EQ(0, pq1553_.instr1_.at(0));
        EXPECT_EQ(0, pq1553_.msgerr1_.at(0));
        EXPECT_EQ(0, pq1553_.status_rtaddr1_.at(0));
        EXPECT_EQ(-1, pq1553_.status_word1_.at(0));
    }

    void ValidateStatusWord2Null()
    {
        EXPECT_EQ(0, pq1553_.terminal2_.at(0));
        EXPECT_EQ(0, pq1553_.dynbusctrl2_.at(0));
        EXPECT_EQ(0, pq1553_.subsys2_.at(0));
        EXPECT_EQ(0, pq1553_.busy2_.at(0));
        EXPECT_EQ(0, pq1553_.bcastrcv2_.at(0));
        EXPECT_EQ(0, pq1553_.svcreq2_.at(0));
        EXPECT_EQ(0, pq1553_.instr2_.at(0));
        EXPECT_EQ(0, pq1553_.msgerr2_.at(0));
        EXPECT_EQ(0, pq1553_.status_rtaddr2_.at(0));
        EXPECT_EQ(-1, pq1553_.status_word2_.at(0));
    }

};

// CC = Cyclomatic Complexity
TEST_F(ParquetMilStd1553F1Test, Initialize)
{
    ValidateInitializeAddField();
    ValidateInitializeSetMemoryLocation();

    EXPECT_CALL(mock_pq_ctx_, OpenForWrite(outf.string(), truncate)).WillOnce(Return(true));
    EXPECT_CALL(mock_pq_ctx_, SetupRowCountTracking(pq1553_.DEFAULT_ROW_GROUP_COUNT, 
        pq1553_.DEFAULT_BUFFER_SIZE_MULTIPLIER, true, "MilStd1553F1")).WillOnce(Return(true));
    EXPECT_CALL(mock_pq_ctx_, EnableEmptyFileDeletion(outf.string())).Times(Exactly(1));
    ASSERT_TRUE(pq1553_.Initialize(outf, thread_id));
    EXPECT_EQ(thread_id, pq1553_.thread_id_);
    EXPECT_EQ(outf.string(), pq1553_.outfile_);
    ValidateInitializeResize();
}

TEST_F(ParquetMilStd1553F1Test, InitializeOpenForWriteFail)
{
    ValidateInitializeAddField();
    ValidateInitializeSetMemoryLocation();

    EXPECT_CALL(mock_pq_ctx_, OpenForWrite(outf.string(), truncate)).WillOnce(Return(false));
    ASSERT_FALSE(pq1553_.Initialize(outf, thread_id));
    EXPECT_EQ(thread_id, pq1553_.thread_id_);

    ValidateInitializeResize();
}

TEST_F(ParquetMilStd1553F1Test, InitializeSetupRowCountTrackingFail)
{
    ValidateInitializeAddField();
    ValidateInitializeSetMemoryLocation();

    EXPECT_CALL(mock_pq_ctx_, OpenForWrite(outf.string(), truncate)).WillOnce(Return(true));
    EXPECT_CALL(mock_pq_ctx_, SetupRowCountTracking(pq1553_.DEFAULT_ROW_GROUP_COUNT, 
        pq1553_.DEFAULT_BUFFER_SIZE_MULTIPLIER, true, "MilStd1553F1")).WillOnce(Return(false));
    ASSERT_FALSE(pq1553_.Initialize(outf, thread_id));
    EXPECT_EQ(thread_id, pq1553_.thread_id_);

    ValidateInitializeResize();
}

TEST_F(ParquetMilStd1553F1Test, InitializeFalse)
{
    ValidateInitializeAddFieldFalse();
    ValidateInitializeSetMemoryLocationFalse();

    EXPECT_CALL(mock_pq_ctx_, OpenForWrite(outf.string(), truncate)).WillOnce(Return(true));
    EXPECT_CALL(mock_pq_ctx_, SetupRowCountTracking(pq1553_.DEFAULT_ROW_GROUP_COUNT, 
        pq1553_.DEFAULT_BUFFER_SIZE_MULTIPLIER, true, "MilStd1553F1")).WillOnce(Return(true));
    EXPECT_CALL(mock_pq_ctx_, EnableEmptyFileDeletion(outf.string())).Times(Exactly(1));
    ASSERT_TRUE(pq1553_.Initialize(outf, thread_id));
    EXPECT_EQ(thread_id, pq1553_.thread_id_);

    ValidateInitializeResize();
}

TEST_F(ParquetMilStd1553F1Test, AppendRTtoRTModeCodeIncrementAndWriteFalseDoNotZeroData)
{
    // Initialize parameters for RTtoRT, not mode code
    calcwrdcnt = 20;
    mode_code = 0;
    msg.RR = 1;
    msg.length = calcwrdcnt * 2; // bytes
    msg.sub_addr1 = 2;
    msg.word_count1 = calcwrdcnt;
    msg.word_count2 = calcwrdcnt;

    ManagedPath outf("test.parquet");
    bool truncate = true;
    uint16_t thread_id = 2;
    EXPECT_CALL(mock_pq_ctx_, OpenForWrite(outf.string(), truncate)).WillOnce(Return(true));
    EXPECT_CALL(mock_pq_ctx_, SetupRowCountTracking(pq1553_.DEFAULT_ROW_GROUP_COUNT, 
        pq1553_.DEFAULT_BUFFER_SIZE_MULTIPLIER, true, "MilStd1553F1")).WillOnce(Return(true));
    EXPECT_CALL(mock_pq_ctx_, IncrementAndWrite(thread_id)).WillOnce(Return(false));
 
    ASSERT_TRUE(pq1553_.Initialize(outf, thread_id));
    pq1553_.Append(time, doy, &chan_spec, &msg, data, channel_id, 
        calcwrdcnt, payload_incomplete, statwrd_ptr1_, statwrd_ptr2_);

    ValidateAppendedMetadataRTtoRT();
    ValidateAppended1553Words(data_vec, pq1553_.data_, calcwrdcnt, 0);
}

TEST_F(ParquetMilStd1553F1Test, AppendRTtoRTModeCodeIncrementAndWriteTrueZeroData)
{
    // Initialize parameters for RTtoRT, mode code true
    calcwrdcnt = 20;
    mode_code = 1;
    msg.RR = 1;
    msg.length = calcwrdcnt * 2; // bytes
    msg.sub_addr1 = 0;
    msg.word_count1 = calcwrdcnt;
    msg.word_count2 = calcwrdcnt;

    ManagedPath outf("test.parquet");
    bool truncate = true;
    uint16_t thread_id = 2;
    EXPECT_CALL(mock_pq_ctx_, OpenForWrite(outf.string(), truncate)).WillOnce(Return(true));
    EXPECT_CALL(mock_pq_ctx_, SetupRowCountTracking(pq1553_.DEFAULT_ROW_GROUP_COUNT, 
        pq1553_.DEFAULT_BUFFER_SIZE_MULTIPLIER, true, "MilStd1553F1")).WillOnce(Return(true));
    EXPECT_CALL(mock_pq_ctx_, IncrementAndWrite(thread_id)).WillOnce(Return(true));
 
    ASSERT_TRUE(pq1553_.Initialize(outf, thread_id));
    pq1553_.Append(time, doy, &chan_spec, &msg, data, channel_id, 
        calcwrdcnt, payload_incomplete, statwrd_ptr1_, statwrd_ptr2_);

    ValidateAppendedMetadataRTtoRT();

    // Zero input data
    std::fill(data_vec.begin(), data_vec.end(), 0);
    ValidateAppended1553Words(data_vec, pq1553_.data_, calcwrdcnt, 0);
}

TEST_F(ParquetMilStd1553F1Test, AppendRTtoBCModeCodeIncrementAndWriteFalse)
{
    // Initialize parameters for RTtoBC, not mode code
    calcwrdcnt = 9;
    mode_code = 1;
    msg.RR = 0;
    msg.tx1 = 1; // RT to BC
    msg.length = calcwrdcnt * 2; // bytes
    msg.sub_addr1 = 0;
    msg.word_count1 = calcwrdcnt;
    msg.word_count2 = calcwrdcnt;

    ManagedPath outf("test.parquet");
    bool truncate = true;
    uint16_t thread_id = 2;
    EXPECT_CALL(mock_pq_ctx_, OpenForWrite(outf.string(), truncate)).WillOnce(Return(true));
    EXPECT_CALL(mock_pq_ctx_, SetupRowCountTracking(pq1553_.DEFAULT_ROW_GROUP_COUNT, 
        pq1553_.DEFAULT_BUFFER_SIZE_MULTIPLIER, true, "MilStd1553F1")).WillOnce(Return(true));
    EXPECT_CALL(mock_pq_ctx_, IncrementAndWrite(thread_id)).WillOnce(Return(false));
 
    ASSERT_TRUE(pq1553_.Initialize(outf, thread_id));
    pq1553_.Append(time, doy, &chan_spec, &msg, data, channel_id, 
        calcwrdcnt, payload_incomplete, statwrd_ptr1_, statwrd_ptr2_);

    ValidateAppendedMetadataRTtoBC();
    ValidateAppended1553Words(data_vec, pq1553_.data_, calcwrdcnt, 0);
}

TEST_F(ParquetMilStd1553F1Test, AppendBCtoRTModeCodeIncrementAndWriteFalse)
{
    // Initialize parameters for BCtoRT, not mode code
    calcwrdcnt = 17;
    mode_code = 1;
    msg.RR = 0;
    msg.tx1 = 0; // BC to RT 
    msg.length = calcwrdcnt * 2; // bytes
    msg.sub_addr1 = 0;
    msg.word_count1 = calcwrdcnt;
    msg.word_count2 = calcwrdcnt;

    ManagedPath outf("test.parquet");
    bool truncate = true;
    uint16_t thread_id = 2;
    EXPECT_CALL(mock_pq_ctx_, OpenForWrite(outf.string(), truncate)).WillOnce(Return(true));
    EXPECT_CALL(mock_pq_ctx_, SetupRowCountTracking(pq1553_.DEFAULT_ROW_GROUP_COUNT, 
        pq1553_.DEFAULT_BUFFER_SIZE_MULTIPLIER, true, "MilStd1553F1")).WillOnce(Return(true));
    EXPECT_CALL(mock_pq_ctx_, IncrementAndWrite(thread_id)).WillOnce(Return(false));
 
    ASSERT_TRUE(pq1553_.Initialize(outf, thread_id));
    pq1553_.Append(time, doy, &chan_spec, &msg, data, channel_id, 
        calcwrdcnt, payload_incomplete, statwrd_ptr1_, statwrd_ptr2_);

    ValidateAppendedMetadataBCtoRT();
    ValidateAppended1553Words(data_vec, pq1553_.data_, calcwrdcnt, 0);
}

TEST_F(ParquetMilStd1553F1Test, AppendBCtoRTModeCodeIncrementAndWriteFalseCalcwrdcntZero)
{
    // Initialize parameters for BCtoRT, not mode code
    calcwrdcnt = 0; // set to zero
    mode_code = 1;
    msg.RR = 0;
    msg.tx1 = 0; // BC to RT 
    msg.length = calcwrdcnt * 2; // bytes
    msg.sub_addr1 = 0;
    msg.word_count1 = calcwrdcnt;
    msg.word_count2 = calcwrdcnt;

    ManagedPath outf("test.parquet");
    bool truncate = true;
    uint16_t thread_id = 2;
    EXPECT_CALL(mock_pq_ctx_, OpenForWrite(outf.string(), truncate)).WillOnce(Return(true));
    EXPECT_CALL(mock_pq_ctx_, SetupRowCountTracking(pq1553_.DEFAULT_ROW_GROUP_COUNT, 
        pq1553_.DEFAULT_BUFFER_SIZE_MULTIPLIER, true, "MilStd1553F1")).WillOnce(Return(true));
    EXPECT_CALL(mock_pq_ctx_, IncrementAndWrite(thread_id)).WillOnce(Return(false));
 
    ASSERT_TRUE(pq1553_.Initialize(outf, thread_id));
    pq1553_.Append(time, doy, &chan_spec, &msg, data, channel_id, 
        calcwrdcnt, payload_incomplete, statwrd_ptr1_, statwrd_ptr2_);

    ValidateAppendedMetadataBCtoRT();
    ValidateAppended1553Words(data_vec, pq1553_.data_, calcwrdcnt, 0);
}

TEST_F(ParquetMilStd1553F1Test, AppendStatusWord2Null)
{
    calcwrdcnt = 23; 
    mode_code = 0;
    msg.RR = 0;
    msg.length = calcwrdcnt * 2; // bytes
    msg.word_count1 = calcwrdcnt;
    msg.word_count2 = calcwrdcnt;

    statwrd1_.terminal = 1;
    statwrd1_.bcastrcv = 1;
    statwrd1_.dynbusctrl = 1;
    statwrd1_.busy = 1;
    statwrd1_.rtaddr = 24;
    statwrd1_.msgerr = 1;
    statwrd1_.subsys = 1;
    statwrd1_.svcreq = 1;
    statwrd1_.instr = 1;

    statwrd_ptr2_ = nullptr;

    ManagedPath outf("test.parquet");
    bool truncate = true;
    uint16_t thread_id = 2;
    EXPECT_CALL(mock_pq_ctx_, OpenForWrite(outf.string(), truncate)).WillOnce(Return(true));
    EXPECT_CALL(mock_pq_ctx_, SetupRowCountTracking(pq1553_.DEFAULT_ROW_GROUP_COUNT, 
        pq1553_.DEFAULT_BUFFER_SIZE_MULTIPLIER, true, "MilStd1553F1")).WillOnce(Return(true));
    EXPECT_CALL(mock_pq_ctx_, IncrementAndWrite(thread_id)).WillOnce(Return(false));
 
    ASSERT_TRUE(pq1553_.Initialize(outf, thread_id));
    pq1553_.Append(time, doy, &chan_spec, &msg, data, channel_id, 
        calcwrdcnt, payload_incomplete, statwrd_ptr1_, statwrd_ptr2_);

    ValidateStatusWord1();
    ValidateStatusWord2Null();
}

TEST_F(ParquetMilStd1553F1Test, AppendStatusWord1Null)
{
    calcwrdcnt = 23; 
    mode_code = 0;
    msg.RR = 0;
    msg.length = calcwrdcnt * 2; // bytes
    msg.word_count1 = calcwrdcnt;
    msg.word_count2 = calcwrdcnt;

    statwrd2_.terminal = 1;
    statwrd2_.bcastrcv = 1;
    statwrd2_.dynbusctrl = 1;
    statwrd2_.busy = 1;
    statwrd2_.rtaddr = 17;
    statwrd2_.msgerr = 1;
    statwrd2_.subsys = 1;
    statwrd2_.svcreq = 1;
    statwrd2_.instr = 1;

    statwrd_ptr1_ = nullptr;

    ManagedPath outf("test.parquet");
    bool truncate = true;
    uint16_t thread_id = 2;
    EXPECT_CALL(mock_pq_ctx_, OpenForWrite(outf.string(), truncate)).WillOnce(Return(true));
    EXPECT_CALL(mock_pq_ctx_, SetupRowCountTracking(pq1553_.DEFAULT_ROW_GROUP_COUNT, 
        pq1553_.DEFAULT_BUFFER_SIZE_MULTIPLIER, true, "MilStd1553F1")).WillOnce(Return(true));
    EXPECT_CALL(mock_pq_ctx_, IncrementAndWrite(thread_id)).WillOnce(Return(false));
 
    ASSERT_TRUE(pq1553_.Initialize(outf, thread_id));
    pq1553_.Append(time, doy, &chan_spec, &msg, data, channel_id, 
        calcwrdcnt, payload_incomplete, statwrd_ptr1_, statwrd_ptr2_);

    ValidateStatusWord2();
    ValidateStatusWord1Null();
}

TEST_F(ParquetMilStd1553F1Test, AppendStatusWordsNonNull)
{
    calcwrdcnt = 23; 
    mode_code = 0;
    msg.RR = 0;
    msg.length = calcwrdcnt * 2; // bytes
    msg.word_count1 = calcwrdcnt;
    msg.word_count2 = calcwrdcnt;

    statwrd1_.terminal = 1;
    statwrd1_.bcastrcv = 1;
    statwrd1_.dynbusctrl = 1;
    statwrd1_.busy = 1;
    statwrd1_.rtaddr = 24;
    statwrd1_.msgerr = 1;
    statwrd1_.subsys = 1;
    statwrd1_.svcreq = 1;
    statwrd1_.instr = 1;

    statwrd2_.terminal = 1;
    statwrd2_.bcastrcv = 1;
    statwrd2_.dynbusctrl = 1;
    statwrd2_.busy = 1;
    statwrd2_.rtaddr = 17;
    statwrd2_.msgerr = 1;
    statwrd2_.subsys = 1;
    statwrd2_.svcreq = 1;
    statwrd2_.instr = 1;

    ManagedPath outf("test.parquet");
    bool truncate = true;
    uint16_t thread_id = 2;
    EXPECT_CALL(mock_pq_ctx_, OpenForWrite(outf.string(), truncate)).WillOnce(Return(true));
    EXPECT_CALL(mock_pq_ctx_, SetupRowCountTracking(pq1553_.DEFAULT_ROW_GROUP_COUNT, 
        pq1553_.DEFAULT_BUFFER_SIZE_MULTIPLIER, true, "MilStd1553F1")).WillOnce(Return(true));
    EXPECT_CALL(mock_pq_ctx_, IncrementAndWrite(thread_id)).WillOnce(Return(false));
 
    ASSERT_TRUE(pq1553_.Initialize(outf, thread_id));
    pq1553_.Append(time, doy, &chan_spec, &msg, data, channel_id, 
        calcwrdcnt, payload_incomplete, statwrd_ptr1_, statwrd_ptr2_);

    ValidateStatusWord1();
    ValidateStatusWord2();
}