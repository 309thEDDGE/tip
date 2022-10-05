#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "parquet_context.h"
#include "ch10_videof0_header_format.h"
#include "parquet_videodataf0.h"
#include "parquet_context_mock.h"

using ::testing::Return;
using ::testing::Exactly;
using ::testing::_;
using ::testing::NiceMock;

class ParquetVideoDataF0Test : public ::testing::Test
{
   protected:
    NiceMock<MockParquetContext> mock_pq_ctx_;
    ParquetVideoDataF0 pq_video_;
    ManagedPath outf_;
    bool truncate_;
    uint16_t thread_id_;

    uint64_t time_stamp_;
    uint8_t doy_;
    uint32_t channel_id_;
    Ch10VideoF0HeaderFormat vid_flags_;
    std::vector<video_datum> video_data_;
    video_datum* video_data_ptr_;

    public:
    ParquetVideoDataF0Test() : mock_pq_ctx_(), pq_video_(&mock_pq_ctx_), outf_("test.parquet"),
        truncate_(true), thread_id_(3), time_stamp_(5488519294345), doy_(0), channel_id_(12),
        vid_flags_{}, video_data_(TransportStream_DATA_COUNT), video_data_ptr_(video_data_.data())
    {
        vid_flags_.BA = 0;
        vid_flags_.ET = 1;
        vid_flags_.IPH = 0;
        vid_flags_.KLV = 1;
        vid_flags_.PL = 0;
        vid_flags_.SRS = 0;

        for(size_t i = 0; i < video_data_.size(); i++)
            video_data_[i] = static_cast<video_datum>(i);
    }

    void ValidateInitializeAddField()
    {
        ::testing::Sequence seq;
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "doy", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "ET", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "IPH", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "KLV", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "PL", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "SRS", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "data", TransportStream_DATA_COUNT)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "time", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "channelid", 0)).InSequence(seq).WillOnce(Return(true));
    }

    void ValidateInitializeAddFieldFalse()
    {
        ::testing::Sequence seq;
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "doy", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "ET", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "IPH", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "KLV", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "PL", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "SRS", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "data", TransportStream_DATA_COUNT)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "time", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "channelid", 0)).InSequence(seq).WillOnce(Return(false));
    }

    void ValidateInitializeSetMemoryLocation()
    {
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "doy", nullptr)).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "ET", nullptr)).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "IPH", nullptr)).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "KLV", nullptr)).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI16(_, "PL", nullptr)).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "SRS", nullptr)).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI32(_, "data", nullptr)).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI64(_, "time", nullptr)).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI32(_, "channelid", nullptr)).WillOnce(Return(true));
    }

    void ValidateInitializeSetMemoryLocationFalse()
    {
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "doy", nullptr)).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "ET", nullptr)).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "IPH", nullptr)).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "KLV", nullptr)).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI16(_, "PL", nullptr)).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocUI8(_, "SRS", nullptr)).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI32(_, "data", nullptr)).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI64(_, "time", nullptr)).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI32(_, "channelid", nullptr)).WillOnce(Return(false));
    }


    void ValidateInitializeResize()
    {
        size_t expected_size = ParquetVideoDataF0::DEFAULT_ROW_GROUP_COUNT_VIDEO * 
            ParquetVideoDataF0::DEFAULT_BUFFER_SIZE_MULTIPLIER_VIDEO;
        EXPECT_EQ(expected_size, pq_video_.time_.size());
        EXPECT_EQ(expected_size, pq_video_.doy_.size());
        EXPECT_EQ(expected_size, pq_video_.ET_.size());
        EXPECT_EQ(expected_size, pq_video_.IPH_.size());
        EXPECT_EQ(expected_size, pq_video_.KLV_.size());
        EXPECT_EQ(expected_size, pq_video_.PL_.size());
        EXPECT_EQ(expected_size, pq_video_.SRS_.size());
        EXPECT_EQ(expected_size, pq_video_.channel_id_.size());
        EXPECT_EQ(expected_size * TransportStream_DATA_COUNT, pq_video_.video_data_.size());
    }

    void ValidateAppendedData()
    {
        EXPECT_EQ(doy_, pq_video_.doy_.at(0));
        EXPECT_EQ(vid_flags_.ET, pq_video_.ET_.at(0));
        EXPECT_EQ(vid_flags_.IPH, pq_video_.IPH_.at(0));
        EXPECT_EQ(vid_flags_.KLV, pq_video_.KLV_.at(0));
        EXPECT_EQ(vid_flags_.PL, pq_video_.PL_.at(0));
        EXPECT_EQ(vid_flags_.SRS, pq_video_.SRS_.at(0));
        EXPECT_EQ(time_stamp_, pq_video_.time_.at(0));
        EXPECT_EQ(channel_id_, pq_video_.channel_id_.at(0));

        for(size_t i = 0; i < TransportStream_DATA_COUNT; i++)
            EXPECT_EQ(video_data_.at(i), pq_video_.video_data_.at(i));
    }

};

TEST_F(ParquetVideoDataF0Test, Initialize)
{
    ValidateInitializeAddField();
    ValidateInitializeSetMemoryLocation();

    EXPECT_CALL(mock_pq_ctx_, OpenForWrite(outf_.string(), truncate_)).WillOnce(Return(true));

    EXPECT_CALL(mock_pq_ctx_, SetupRowCountTracking(pq_video_.DEFAULT_ROW_GROUP_COUNT_VIDEO, 
        pq_video_.DEFAULT_BUFFER_SIZE_MULTIPLIER_VIDEO, true, "VideoDataF0")).WillOnce(Return(true));

    EXPECT_CALL(mock_pq_ctx_, EnableEmptyFileDeletion(outf_.string())).Times(Exactly(1));

    ASSERT_TRUE(pq_video_.Initialize(outf_, thread_id_));
    EXPECT_EQ(thread_id_, pq_video_.thread_id_);
    EXPECT_EQ(outf_.string(), pq_video_.outfile_);

    ValidateInitializeResize();
}

TEST_F(ParquetVideoDataF0Test, InitializeFalse)
{
    ValidateInitializeAddFieldFalse();
    ValidateInitializeSetMemoryLocationFalse();

    EXPECT_CALL(mock_pq_ctx_, OpenForWrite(outf_.string(), truncate_)).WillOnce(Return(true));

    EXPECT_CALL(mock_pq_ctx_, SetupRowCountTracking(pq_video_.DEFAULT_ROW_GROUP_COUNT_VIDEO, 
        pq_video_.DEFAULT_BUFFER_SIZE_MULTIPLIER_VIDEO, true, "VideoDataF0")).WillOnce(Return(true));

    EXPECT_CALL(mock_pq_ctx_, EnableEmptyFileDeletion(outf_.string())).Times(Exactly(1));

    ASSERT_TRUE(pq_video_.Initialize(outf_, thread_id_));
    EXPECT_EQ(thread_id_, pq_video_.thread_id_);

    ValidateInitializeResize();
}

TEST_F(ParquetVideoDataF0Test, InitializeOpenForWriteFail)
{
    ValidateInitializeAddField();
    ValidateInitializeSetMemoryLocation();

    EXPECT_CALL(mock_pq_ctx_, OpenForWrite(outf_.string(), truncate_)).WillOnce(Return(false));
    ASSERT_FALSE(pq_video_.Initialize(outf_, thread_id_));
    EXPECT_EQ(thread_id_, pq_video_.thread_id_);

    ValidateInitializeResize();
}

TEST_F(ParquetVideoDataF0Test, InitializeSetupRowCountTrackingFail)
{
    ValidateInitializeAddField();
    ValidateInitializeSetMemoryLocation();

    EXPECT_CALL(mock_pq_ctx_, OpenForWrite(outf_.string(), truncate_)).WillOnce(Return(true));

    EXPECT_CALL(mock_pq_ctx_, SetupRowCountTracking(pq_video_.DEFAULT_ROW_GROUP_COUNT_VIDEO, 
        pq_video_.DEFAULT_BUFFER_SIZE_MULTIPLIER_VIDEO, true, "VideoDataF0")).WillOnce(Return(false));

    ASSERT_FALSE(pq_video_.Initialize(outf_, thread_id_));
    EXPECT_EQ(thread_id_, pq_video_.thread_id_);

    ValidateInitializeResize();
}

TEST_F(ParquetVideoDataF0Test, AppendIncrementAndWriteFalse)
{
    EXPECT_CALL(mock_pq_ctx_, OpenForWrite(outf_.string(), truncate_)).WillOnce(Return(true));
    EXPECT_CALL(mock_pq_ctx_, SetupRowCountTracking(pq_video_.DEFAULT_ROW_GROUP_COUNT_VIDEO, 
        pq_video_.DEFAULT_BUFFER_SIZE_MULTIPLIER_VIDEO, true, "VideoDataF0")).WillOnce(Return(true));
    EXPECT_CALL(mock_pq_ctx_, IncrementAndWrite(thread_id_)).WillOnce(Return(false));
    ASSERT_TRUE(pq_video_.Initialize(outf_, thread_id_));
    pq_video_.Append(time_stamp_, doy_, channel_id_, vid_flags_, video_data_ptr_);

    ValidateAppendedData();
}

TEST_F(ParquetVideoDataF0Test, AppendIncrementAndWriteTrue)
{
    EXPECT_CALL(mock_pq_ctx_, OpenForWrite(outf_.string(), truncate_)).WillOnce(Return(true));
    EXPECT_CALL(mock_pq_ctx_, SetupRowCountTracking(pq_video_.DEFAULT_ROW_GROUP_COUNT_VIDEO, 
        pq_video_.DEFAULT_BUFFER_SIZE_MULTIPLIER_VIDEO, true, "VideoDataF0")).WillOnce(Return(true));
    EXPECT_CALL(mock_pq_ctx_, IncrementAndWrite(thread_id_)).WillOnce(Return(true));
    ASSERT_TRUE(pq_video_.Initialize(outf_, thread_id_));
    pq_video_.Append(time_stamp_, doy_, channel_id_, vid_flags_, video_data_ptr_);

    ValidateAppendedData();
}
