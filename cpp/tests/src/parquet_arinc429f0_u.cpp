#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ch10_arinc429f0_msg_hdr_format.h"
#include "parquet_arinc429f0.h"
#include "parquet_context_mock.h"

using ::testing::Return;
using ::testing::Exactly;
using ::testing::_;
using ::testing::NiceMock;

class ParquetARINC429F0Test : public ::testing::Test
{
   protected:
    NiceMock<MockParquetContext> mock_pq_ctx_;
    ParquetARINC429F0 pq429_;
    ManagedPath outf_;
    bool truncate_;
    uint16_t thread_id_;

    uint64_t time_stamp_;
    uint8_t doy_;
    uint32_t channel_id_;
    ARINC429F0MsgFmt msg_;

    public:
    ParquetARINC429F0Test() : mock_pq_ctx_(), pq429_(&mock_pq_ctx_), outf_("test.parquet"),
        truncate_(true), thread_id_(7), msg_{}, channel_id_(10), time_stamp_(192760278),
        doy_(1)
    {
        msg_.gap = 44391;
        msg_.BS = 1;
        msg_.PE = 0;
        msg_.FE = 0;
        msg_.bus = 11;
        msg_.label = 117;
        msg_.SDI = 2;
        msg_.data = 488130;
        msg_.SSM = 1;
        msg_.parity = 0;
    }

    void ValidateInitializeAddField()
    {
        ::testing::Sequence seq;
        EXPECT_CALL(mock_pq_ctx_, AddField(arrow::int64(), "time", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(arrow::boolean(), "doy", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(arrow::int32(), "channelid", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(arrow::int32(), "gaptime", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(arrow::boolean(), "BS", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(arrow::boolean(), "PE", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(arrow::boolean(), "FE", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(arrow::int16(), "bus", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(arrow::int16(), "label", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(arrow::int8(), "SDI", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(arrow::int32(), "data", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(arrow::int8(), "SSM", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(arrow::boolean(), "parity", 0)).InSequence(seq).WillOnce(Return(true));
    }

    void ValidateInitializeSetMemoryLocation()
    {
        ::testing::Sequence seq;
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI64(_, "time", nullptr)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "doy", nullptr)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI32(_, "gaptime", nullptr)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "BS", nullptr)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "PE", nullptr)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "FE", nullptr)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI16(_, "bus", nullptr)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI16(_, "label", nullptr)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI8(_, "SDI", nullptr)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI32(_, "data", nullptr)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI8(_, "SSM", nullptr)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "parity", nullptr)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI32(_, "channelid", nullptr)).InSequence(seq).WillOnce(Return(true));
    }

    void ValidateInitializeAddFieldFalse()
    {
        ::testing::Sequence seq;
        EXPECT_CALL(mock_pq_ctx_, AddField(arrow::int64(), "time", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(arrow::boolean(), "doy", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(arrow::int32(), "channelid", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(arrow::int32(), "gaptime", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(arrow::boolean(), "BS", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(arrow::boolean(), "PE", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(arrow::boolean(), "FE", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(arrow::int16(), "bus", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(arrow::int16(), "label", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(arrow::int8(), "SDI", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(arrow::int32(), "data", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(arrow::int8(), "SSM", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(arrow::boolean(), "parity", 0)).InSequence(seq).WillOnce(Return(false));

    }

    void ValidateInitializeSetMemoryLocationFalse()
    {
        ::testing::Sequence seq;
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI64(_, "time", nullptr)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "doy", nullptr)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI32(_, "gaptime", nullptr)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "BS", nullptr)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "PE", nullptr)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "FE", nullptr)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI16(_, "bus", nullptr)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI16(_, "label", nullptr)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI8(_, "SDI", nullptr)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI32(_, "data", nullptr)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI8(_, "SSM", nullptr)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "parity", nullptr)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI32(_, "channelid", nullptr)).InSequence(seq).WillOnce(Return(false));
    }


    void ValidateInitializeResize()
    {
        size_t expected_size = ParquetARINC429F0::GetRowGroupRowCount() *
            ParquetARINC429F0::GetRowGroupBufferCount();
        EXPECT_EQ(expected_size, pq429_.time_stamp_.size());
        EXPECT_EQ(expected_size, pq429_.doy_.size());
        EXPECT_EQ(expected_size, pq429_.gap_time_.size());
        EXPECT_EQ(expected_size, pq429_.BS_.size());
        EXPECT_EQ(expected_size, pq429_.PE_.size());
        EXPECT_EQ(expected_size, pq429_.FE_.size());
        EXPECT_EQ(expected_size, pq429_.bus_.size());
        EXPECT_EQ(expected_size, pq429_.label_.size());
        EXPECT_EQ(expected_size, pq429_.SDI_.size());
        EXPECT_EQ(expected_size, pq429_.data_.size());
        EXPECT_EQ(expected_size, pq429_.SSM_.size());
        EXPECT_EQ(expected_size, pq429_.parity_.size());
        EXPECT_EQ(expected_size, pq429_.channel_id_.size());
    }

    void ValidateAppendedData()
    {
        EXPECT_EQ(time_stamp_, pq429_.time_stamp_.at(0));
        EXPECT_EQ(doy_, pq429_.doy_.at(0));
        EXPECT_EQ(msg_.gap, pq429_.gap_time_.at(0));
        EXPECT_EQ(msg_.BS, pq429_.BS_.at(0));
        EXPECT_EQ(msg_.PE, pq429_.PE_.at(0));
        EXPECT_EQ(msg_.FE, pq429_.FE_.at(0));
        EXPECT_EQ(msg_.bus, pq429_.bus_.at(0));
        EXPECT_EQ(256, pq429_.label_.at(0)); // 117 dec (msg.label) => 256 oct after EncodeARINC429Label()
        EXPECT_EQ(msg_.SDI, pq429_.SDI_.at(0));
        EXPECT_EQ(msg_.data, pq429_.data_.at(0));
        EXPECT_EQ(msg_.SSM, pq429_.SSM_.at(0));
        EXPECT_EQ(msg_.FE, pq429_.parity_.at(0));
        EXPECT_EQ(channel_id_, pq429_.channel_id_.at(0));
    }
};

TEST_F(ParquetARINC429F0Test, EncodeARINC429Label)
{
    uint32_t raw_label = 83;

    // Expect 83 dec => 312 oct
    EXPECT_EQ(pq429_.EncodeARINC429Label(raw_label), 312);

    raw_label = 154;
    // Expect 154 dec => 131 oct
    EXPECT_EQ(pq429_.EncodeARINC429Label(raw_label), 131);

    raw_label = 153;
    // Expect 153 dec == 231 oct
    EXPECT_EQ(pq429_.EncodeARINC429Label(raw_label), 231);
}

TEST_F(ParquetARINC429F0Test, Initialize)
{
    ValidateInitializeAddField();

    ValidateInitializeSetMemoryLocation();

    EXPECT_CALL(mock_pq_ctx_, OpenForWrite(outf_.string(), truncate_)).WillOnce(Return(true));
    EXPECT_CALL(mock_pq_ctx_, SetupRowCountTracking(pq429_.GetRowGroupRowCount(),
        pq429_.GetRowGroupBufferCount(), true, "ARINC429F0")).WillOnce(Return(true));
    EXPECT_CALL(mock_pq_ctx_, EnableEmptyFileDeletion(outf_.string())).Times(Exactly(1));

    ASSERT_EQ(0, pq429_.Initialize(outf_, thread_id_));
    EXPECT_EQ(thread_id_, pq429_.thread_id_);
    EXPECT_EQ(outf_.string(), pq429_.outfile_);
    ValidateInitializeResize();
}

TEST_F(ParquetARINC429F0Test, InitializeSetupRowCountTrackingFail)
{
    ValidateInitializeAddField();
    ValidateInitializeSetMemoryLocation();

    EXPECT_CALL(mock_pq_ctx_, OpenForWrite(outf_.string(), truncate_)).WillOnce(Return(true));
    EXPECT_CALL(mock_pq_ctx_, SetupRowCountTracking(pq429_.GetRowGroupRowCount(),
        pq429_.GetRowGroupBufferCount(), true, "ARINC429F0")).WillOnce(Return(false));

    ASSERT_EQ(70, pq429_.Initialize(outf_, thread_id_));
    EXPECT_EQ(thread_id_, pq429_.thread_id_);

    ValidateInitializeResize();
}

TEST_F(ParquetARINC429F0Test, InitializeOpenForWriteFail)
{
    ValidateInitializeAddField();
    ValidateInitializeSetMemoryLocation();

    EXPECT_CALL(mock_pq_ctx_, OpenForWrite(outf_.string(), truncate_)).WillOnce(Return(false));

    ASSERT_EQ(74, pq429_.Initialize(outf_, thread_id_));
    EXPECT_EQ(thread_id_, pq429_.thread_id_);

    ValidateInitializeResize();
}

TEST_F(ParquetARINC429F0Test, AppendIncrementAndWriteFalse)
{
    EXPECT_CALL(mock_pq_ctx_, OpenForWrite(outf_.string(), truncate_)).WillOnce(Return(true));
    EXPECT_CALL(mock_pq_ctx_, SetupRowCountTracking(pq429_.GetRowGroupRowCount(),
        pq429_.GetRowGroupBufferCount(), true, "ARINC429F0")).WillOnce(Return(true));
    EXPECT_CALL(mock_pq_ctx_, IncrementAndWrite(thread_id_)).WillOnce(Return(false));
    ASSERT_EQ(0, pq429_.Initialize(outf_, thread_id_));

    pq429_.Append(time_stamp_, doy_, &msg_, channel_id_);

    ValidateAppendedData();
}

TEST_F(ParquetARINC429F0Test, AppendIncrementAndWriteTrue)
{
    EXPECT_CALL(mock_pq_ctx_, OpenForWrite(outf_.string(), truncate_)).WillOnce(Return(true));
    EXPECT_CALL(mock_pq_ctx_, SetupRowCountTracking(pq429_.GetRowGroupRowCount(),
        pq429_.GetRowGroupBufferCount(), true, "ARINC429F0")).WillOnce(Return(true));
    EXPECT_CALL(mock_pq_ctx_, IncrementAndWrite(thread_id_)).WillOnce(Return(true));
    ASSERT_EQ(0, pq429_.Initialize(outf_, thread_id_));

    pq429_.Append(time_stamp_, doy_, &msg_, channel_id_);

    ValidateAppendedData();
}
