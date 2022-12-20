#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ch10_tdpf1_hdr_format.h"
#include "parquet_tdpf1.h"
#include "parquet_context_mock.h"

using ::testing::Return;
using ::testing::Exactly;
using ::testing::_;
using ::testing::NiceMock;

class ParquetTDPF1Test : public ::testing::Test
{
   protected:
    NiceMock<MockParquetContext> mock_pq_ctx_;
    ParquetTDPF1 pqtdp_;
    ManagedPath outf_;
    bool truncate_;
    uint16_t thread_id_;

    uint64_t time_stamp_;
    TDF1CSDWFmt tdp_;

    public:
    ParquetTDPF1Test() : mock_pq_ctx_(), pqtdp_(&mock_pq_ctx_), outf_("test.parquet"), 
        truncate_(true), thread_id_(7), tdp_{}, time_stamp_(192760278)
    {
        tdp_.src = 2;
        tdp_.time_fmt = 9;
        tdp_.leap_year = 1;
        tdp_.date_fmt = 0;
    }

    void ValidateInitializeAddField()
    {
        ::testing::Sequence seq;
        EXPECT_CALL(mock_pq_ctx_, AddField(arrow::int64(), "time", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(arrow::int8(), "src", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(arrow::int8(), "time_fmt", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(arrow::boolean(), "leap_yr", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(arrow::boolean(), "date_fmt", 0)).InSequence(seq).WillOnce(Return(true));
    }

    void ValidateInitializeSetMemoryLocation()
    {
        ::testing::Sequence seq;
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI64(_, "time", nullptr)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI8(_, "src", nullptr)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI8(_, "time_fmt", nullptr)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "leap_yr", nullptr)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "date_fmt", nullptr)).InSequence(seq).WillOnce(Return(true));
    }

    void ValidateInitializeAddFieldFalse()
    {
        ::testing::Sequence seq;
        EXPECT_CALL(mock_pq_ctx_, AddField(arrow::int64(), "time", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(arrow::int8(), "src", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(arrow::int8(), "time_fmt", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(arrow::boolean(), "leap_yr", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(arrow::boolean(), "date_fmt", 0)).InSequence(seq).WillOnce(Return(false));
    }

    void ValidateInitializeSetMemoryLocationFalse()
    {
        ::testing::Sequence seq;
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI64(_, "time", nullptr)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI8(_, "src", nullptr)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI8(_, "time_fmt", nullptr)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "leap_yr", nullptr)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "date_fmt", nullptr)).InSequence(seq).WillOnce(Return(false));
    }


    void ValidateInitializeResize()
    {
        size_t expected_size = ParquetTDPF1::GetRowGroupRowCount() * 
            ParquetTDPF1::GetRowGroupBufferCount();
        EXPECT_EQ(expected_size, pqtdp_.time_stamp_.size());
        EXPECT_EQ(expected_size, pqtdp_.src_.size());
        EXPECT_EQ(expected_size, pqtdp_.time_fmt_.size());
        EXPECT_EQ(expected_size, pqtdp_.leap_year_.size());
        EXPECT_EQ(expected_size, pqtdp_.date_fmt_.size());
    }

    void ValidateAppendedData()
    {
        EXPECT_EQ(time_stamp_, pqtdp_.time_stamp_.at(0));
        EXPECT_EQ(tdp_.src, pqtdp_.src_.at(0));
        EXPECT_EQ(tdp_.time_fmt, pqtdp_.time_fmt_.at(0));
        EXPECT_EQ(tdp_.leap_year, pqtdp_.leap_year_.at(0));
        EXPECT_EQ(tdp_.date_fmt, pqtdp_.date_fmt_.at(0));
    }
};

TEST_F(ParquetTDPF1Test, Initialize)
{
    ValidateInitializeAddField();

    ValidateInitializeSetMemoryLocation();

    EXPECT_CALL(mock_pq_ctx_, OpenForWrite(outf_.string(), truncate_)).WillOnce(Return(true));
    EXPECT_CALL(mock_pq_ctx_, SetupRowCountTracking(pqtdp_.GetRowGroupRowCount(), 
        pqtdp_.GetRowGroupBufferCount(), true, "TIME_DATA_F1")).WillOnce(Return(true));
    EXPECT_CALL(mock_pq_ctx_, EnableEmptyFileDeletion(outf_.string())).Times(Exactly(1));

    ASSERT_EQ(EX_OK, pqtdp_.Initialize(outf_, thread_id_));
    EXPECT_EQ(thread_id_, pqtdp_.thread_id_);
    EXPECT_EQ(outf_.string(), pqtdp_.outfile_);
    ValidateInitializeResize();
}

TEST_F(ParquetTDPF1Test, InitializeSetupRowCountTrackingFail)
{
    ValidateInitializeAddField();
    ValidateInitializeSetMemoryLocation();

    EXPECT_CALL(mock_pq_ctx_, OpenForWrite(outf_.string(), truncate_)).WillOnce(Return(true));
    EXPECT_CALL(mock_pq_ctx_, SetupRowCountTracking(pqtdp_.GetRowGroupRowCount(), 
        pqtdp_.GetRowGroupBufferCount(), true, "TIME_DATA_F1")).WillOnce(Return(false));

    ASSERT_EQ(EX_SOFTWARE, pqtdp_.Initialize(outf_, thread_id_));
    EXPECT_EQ(thread_id_, pqtdp_.thread_id_);

    ValidateInitializeResize();
}

TEST_F(ParquetTDPF1Test, InitializeOpenForWriteFail)
{
    ValidateInitializeAddField();
    ValidateInitializeSetMemoryLocation();

    EXPECT_CALL(mock_pq_ctx_, OpenForWrite(outf_.string(), truncate_)).WillOnce(Return(false));

    ASSERT_EQ(EX_IOERR, pqtdp_.Initialize(outf_, thread_id_)); EXPECT_EQ(thread_id_, pqtdp_.thread_id_);
    EXPECT_EQ(thread_id_, pqtdp_.thread_id_);
    ValidateInitializeResize();
}

TEST_F(ParquetTDPF1Test, AppendIncrementAndWriteFalse)
{
    EXPECT_CALL(mock_pq_ctx_, OpenForWrite(outf_.string(), truncate_)).WillOnce(Return(true));
    EXPECT_CALL(mock_pq_ctx_, SetupRowCountTracking(pqtdp_.GetRowGroupRowCount(), 
        pqtdp_.GetRowGroupBufferCount(), true, "TIME_DATA_F1")).WillOnce(Return(true));
    EXPECT_CALL(mock_pq_ctx_, IncrementAndWrite(thread_id_)).WillOnce(Return(false));
    ASSERT_EQ(EX_OK, pqtdp_.Initialize(outf_, thread_id_));
    
    pqtdp_.Append(time_stamp_, tdp_);

    ValidateAppendedData();
}

TEST_F(ParquetTDPF1Test, AppendIncrementAndWriteTrue)
{
    EXPECT_CALL(mock_pq_ctx_, OpenForWrite(outf_.string(), truncate_)).WillOnce(Return(true));
    EXPECT_CALL(mock_pq_ctx_, SetupRowCountTracking(pqtdp_.GetRowGroupRowCount(), 
        pqtdp_.GetRowGroupBufferCount(), true, "TIME_DATA_F1")).WillOnce(Return(true));
    EXPECT_CALL(mock_pq_ctx_, IncrementAndWrite(thread_id_)).WillOnce(Return(true));
    ASSERT_EQ(EX_OK, pqtdp_.Initialize(outf_, thread_id_));
    
    pqtdp_.Append(time_stamp_, tdp_);

    ValidateAppendedData();
}
