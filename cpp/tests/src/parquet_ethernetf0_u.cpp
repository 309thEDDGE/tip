#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "parquet_context.h"
#include "parquet_ethernetf0.h"
#include "ethernet_data.h"
#include "parquet_context_mock.h"

using ::testing::Return;
using ::testing::Exactly;
using ::testing::_;
using ::testing::NiceMock;

class ParquetEthernetF0Test : public ::testing::Test
{
   protected:
    NiceMock<MockParquetContext> mock_pq_ctx_;
    ParquetEthernetF0 pq_eth_;
    ManagedPath outf_;
    bool truncate_;
    uint16_t thread_id_;

    uint64_t time_stamp_;
    uint32_t channel_id_;
    EthernetData eth_data_;


    public:
    ParquetEthernetF0Test() : mock_pq_ctx_(), pq_eth_(&mock_pq_ctx_), outf_("test.parquet"),
        truncate_(true), thread_id_(4), time_stamp_(871212463200), channel_id_(31),
        eth_data_()
    {
        eth_data_.payload_size_ = 78;
        eth_data_.dst_mac_addr_ = "abcd-ABCD";
        eth_data_.src_mac_addr_ = "lmno-LMNO";
        eth_data_.ethertype_ = 10;
        eth_data_.frame_format_ = 1;
        eth_data_.dsap_ = 4;
        eth_data_.ssap_ = 21;
        eth_data_.snd_seq_number_ = 98;
        eth_data_.rcv_seq_number_ = 210;
        eth_data_.dst_ip_addr_ = "123.456.168.3";
        eth_data_.src_ip_addr_ = "123.456.168.23";
        eth_data_.id_ = 881;
        eth_data_.protocol_ = 55;
        eth_data_.offset_ = 33;
        eth_data_.dst_port_ = 8081;
        eth_data_.src_port_ = 3015;
    }

    void ValidateInitializeAddField()
    {
        ::testing::Sequence seq;
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "time", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "channelid", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "payload", static_cast<int>(pq_eth_.GetDataPayloadListElementCount()))).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "payload_sz", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "dstmac", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "srcmac", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "ethtype", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "llcfmt", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "llcdsap", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "llcssap", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "llcsndseqnum", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "llcrcvseqnum", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "dstip", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "srcip", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "ipid", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "ipproto", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "ipoffset", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "dstport", 0)).InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "srcport", 0)).InSequence(seq).WillOnce(Return(true));
    }

    void ValidateInitializeSetMemoryLocation()
    {
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI64(_, "time", nullptr)).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI32(_, "channelid", nullptr)).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI16(_, "payload", nullptr)).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI64(_, "payload_sz", nullptr)).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocString(_, "dstmac", nullptr)).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocString(_, "srcmac", nullptr)).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI32(_, "ethtype", nullptr)).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI16(_, "llcfmt", nullptr)).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI16(_, "llcdsap", nullptr)).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI16(_, "llcssap", nullptr)).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI16(_, "llcsndseqnum", nullptr)).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI16(_, "llcrcvseqnum", nullptr)).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocString(_, "dstip", nullptr)).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocString(_, "srcip", nullptr)).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI32(_, "ipid", nullptr)).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI16(_, "ipproto", nullptr)).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI32(_, "ipoffset", nullptr)).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI32(_, "dstport", nullptr)).WillOnce(Return(true));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI32(_, "srcport", nullptr)).WillOnce(Return(true));
    }

    void ValidateInitializeAddFieldFalse()
    {
        ::testing::Sequence seq;
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "time", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "channelid", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "payload", static_cast<int>(pq_eth_.GetDataPayloadListElementCount()))).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "payload_sz", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "dstmac", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "srcmac", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "ethtype", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "llcfmt", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "llcdsap", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "llcssap", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "llcsndseqnum", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "llcrcvseqnum", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "dstip", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "srcip", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "ipid", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "ipproto", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "ipoffset", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "dstport", 0)).InSequence(seq).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, AddField(_, "srcport", 0)).InSequence(seq).WillOnce(Return(false));
    }

    void ValidateInitializeSetMemoryLocationFalse()
    {
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI64(_, "time", nullptr)).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI32(_, "channelid", nullptr)).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI16(_, "payload", nullptr)).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI64(_, "payload_sz", nullptr)).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocString(_, "dstmac", nullptr)).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocString(_, "srcmac", nullptr)).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI32(_, "ethtype", nullptr)).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI16(_, "llcfmt", nullptr)).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI16(_, "llcdsap", nullptr)).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI16(_, "llcssap", nullptr)).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI16(_, "llcsndseqnum", nullptr)).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI16(_, "llcrcvseqnum", nullptr)).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocString(_, "dstip", nullptr)).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocString(_, "srcip", nullptr)).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI32(_, "ipid", nullptr)).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI16(_, "ipproto", nullptr)).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI32(_, "ipoffset", nullptr)).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI32(_, "dstport", nullptr)).WillOnce(Return(false));
        EXPECT_CALL(mock_pq_ctx_, SetMemLocI32(_, "srcport", nullptr)).WillOnce(Return(false));
    }

    void ValidateInitializeResize()
    {
        EXPECT_EQ(ParquetEthernetF0::GetRowGroupRowCount() * ParquetEthernetF0::GetRowGroupBufferCount(), pq_eth_.time_stamp_.size());
        EXPECT_EQ(ParquetEthernetF0::GetRowGroupRowCount() * ParquetEthernetF0::GetRowGroupBufferCount(), pq_eth_.channel_id_.size());
        EXPECT_EQ(ParquetEthernetF0::GetRowGroupRowCount() * ParquetEthernetF0::GetRowGroupBufferCount() * pq_eth_.GetDataPayloadListElementCount(), pq_eth_.payload_.size());
        EXPECT_EQ(ParquetEthernetF0::GetRowGroupRowCount() * ParquetEthernetF0::GetRowGroupBufferCount(), pq_eth_.payload_size_.size());
        EXPECT_EQ(ParquetEthernetF0::GetRowGroupRowCount() * ParquetEthernetF0::GetRowGroupBufferCount(), pq_eth_.dst_mac_addr_.size());
        EXPECT_EQ(ParquetEthernetF0::GetRowGroupRowCount() * ParquetEthernetF0::GetRowGroupBufferCount(), pq_eth_.src_mac_addr_.size());
        EXPECT_EQ(ParquetEthernetF0::GetRowGroupRowCount() * ParquetEthernetF0::GetRowGroupBufferCount(), pq_eth_.ethertype_.size());
        EXPECT_EQ(ParquetEthernetF0::GetRowGroupRowCount() * ParquetEthernetF0::GetRowGroupBufferCount(), pq_eth_.frame_format_.size());
        EXPECT_EQ(ParquetEthernetF0::GetRowGroupRowCount() * ParquetEthernetF0::GetRowGroupBufferCount(), pq_eth_.dsap_.size());
        EXPECT_EQ(ParquetEthernetF0::GetRowGroupRowCount() * ParquetEthernetF0::GetRowGroupBufferCount(), pq_eth_.ssap_.size());
        EXPECT_EQ(ParquetEthernetF0::GetRowGroupRowCount() * ParquetEthernetF0::GetRowGroupBufferCount(), pq_eth_.snd_seq_number_.size());
        EXPECT_EQ(ParquetEthernetF0::GetRowGroupRowCount() * ParquetEthernetF0::GetRowGroupBufferCount(), pq_eth_.rcv_seq_number_.size());
        EXPECT_EQ(ParquetEthernetF0::GetRowGroupRowCount() * ParquetEthernetF0::GetRowGroupBufferCount(), pq_eth_.dst_ip_addr_.size());
        EXPECT_EQ(ParquetEthernetF0::GetRowGroupRowCount() * ParquetEthernetF0::GetRowGroupBufferCount(), pq_eth_.src_ip_addr_.size());
        EXPECT_EQ(ParquetEthernetF0::GetRowGroupRowCount() * ParquetEthernetF0::GetRowGroupBufferCount(), pq_eth_.id_.size());
        EXPECT_EQ(ParquetEthernetF0::GetRowGroupRowCount() * ParquetEthernetF0::GetRowGroupBufferCount(), pq_eth_.protocol_.size());
        EXPECT_EQ(ParquetEthernetF0::GetRowGroupRowCount() * ParquetEthernetF0::GetRowGroupBufferCount(), pq_eth_.offset_.size());
        EXPECT_EQ(ParquetEthernetF0::GetRowGroupRowCount() * ParquetEthernetF0::GetRowGroupBufferCount(), pq_eth_.dst_port_.size());
        EXPECT_EQ(ParquetEthernetF0::GetRowGroupRowCount() * ParquetEthernetF0::GetRowGroupBufferCount(), pq_eth_.src_port_.size());
    }

    void ValidateAppendedData()
    {
        EXPECT_EQ(time_stamp_, pq_eth_.time_stamp_.at(0));
        EXPECT_EQ(channel_id_, pq_eth_.channel_id_.at(0));
        EXPECT_EQ(eth_data_.payload_size_, pq_eth_.payload_size_.at(0));
        EXPECT_EQ(eth_data_.dst_mac_addr_, pq_eth_.dst_mac_addr_.at(0));
        EXPECT_EQ(eth_data_.src_mac_addr_, pq_eth_.src_mac_addr_.at(0));
        EXPECT_EQ(eth_data_.ethertype_, pq_eth_.ethertype_.at(0));
        EXPECT_EQ(eth_data_.frame_format_, pq_eth_.frame_format_.at(0));
        EXPECT_EQ(eth_data_.dsap_, pq_eth_.dsap_.at(0));
        EXPECT_EQ(eth_data_.ssap_, pq_eth_.ssap_.at(0));
        EXPECT_EQ(eth_data_.snd_seq_number_, pq_eth_.snd_seq_number_.at(0));
        EXPECT_EQ(eth_data_.rcv_seq_number_, pq_eth_.rcv_seq_number_.at(0));
        EXPECT_EQ(eth_data_.dst_ip_addr_, pq_eth_.dst_ip_addr_.at(0));
        EXPECT_EQ(eth_data_.src_ip_addr_, pq_eth_.src_ip_addr_.at(0));
        EXPECT_EQ(eth_data_.id_, pq_eth_.id_.at(0));
        EXPECT_EQ(eth_data_.protocol_, pq_eth_.protocol_.at(0));
        EXPECT_EQ(eth_data_.offset_, pq_eth_.offset_.at(0));
        EXPECT_EQ(eth_data_.dst_port_, pq_eth_.dst_port_.at(0));
        EXPECT_EQ(eth_data_.src_port_, pq_eth_.src_port_.at(0));

        for(size_t i = 0; i < ParquetEthernetF0::GetDataPayloadListElementCount(); i++)
            EXPECT_EQ(eth_data_.payload_.at(i), pq_eth_.payload_.at(i));
    }

};

TEST_F(ParquetEthernetF0Test, Initialize)
{
    ValidateInitializeAddField();
    ValidateInitializeSetMemoryLocation();

    EXPECT_CALL(mock_pq_ctx_, OpenForWrite(outf_.string(), truncate_)).WillOnce(Return(true));
    EXPECT_CALL(mock_pq_ctx_, SetupRowCountTracking(pq_eth_.GetRowGroupRowCount(), 
        pq_eth_.GetRowGroupBufferCount(), true, "EthernetF0")).WillOnce(Return(true));
    EXPECT_CALL(mock_pq_ctx_, EnableEmptyFileDeletion(outf_.string())).Times(Exactly(1));

    ASSERT_TRUE(pq_eth_.Initialize(outf_, thread_id_));
    EXPECT_EQ(thread_id_, pq_eth_.thread_id_);
    EXPECT_EQ(pq_eth_.payload_ptr_, pq_eth_.payload_.data());
    EXPECT_EQ(outf_.string(), pq_eth_.outfile_);
    ValidateInitializeResize();
}

TEST_F(ParquetEthernetF0Test, InitializeFalse)
{
    ValidateInitializeAddFieldFalse();
    ValidateInitializeSetMemoryLocationFalse();

    EXPECT_CALL(mock_pq_ctx_, OpenForWrite(outf_.string(), truncate_)).WillOnce(Return(true));
    EXPECT_CALL(mock_pq_ctx_, SetupRowCountTracking(pq_eth_.GetRowGroupRowCount(), 
        pq_eth_.GetRowGroupBufferCount(), true, "EthernetF0")).WillOnce(Return(true));
    EXPECT_CALL(mock_pq_ctx_, EnableEmptyFileDeletion(outf_.string())).Times(Exactly(1));

    ASSERT_TRUE(pq_eth_.Initialize(outf_, thread_id_));
    EXPECT_EQ(thread_id_, pq_eth_.thread_id_);
    EXPECT_EQ(pq_eth_.payload_ptr_, pq_eth_.payload_.data());
    ValidateInitializeResize();
}

TEST_F(ParquetEthernetF0Test, InitializeOpenForWriteFail)
{
    ValidateInitializeAddField();
    ValidateInitializeSetMemoryLocation();

    EXPECT_CALL(mock_pq_ctx_, OpenForWrite(outf_.string(), truncate_)).WillOnce(Return(false));

    ASSERT_FALSE(pq_eth_.Initialize(outf_, thread_id_));
    EXPECT_EQ(thread_id_, pq_eth_.thread_id_);
    EXPECT_EQ(pq_eth_.payload_ptr_, pq_eth_.payload_.data());
    ValidateInitializeResize();
}

TEST_F(ParquetEthernetF0Test, InitializeSetupRowCountTrackingFail)
{
    ValidateInitializeAddField();
    ValidateInitializeSetMemoryLocation();

    EXPECT_CALL(mock_pq_ctx_, OpenForWrite(outf_.string(), truncate_)).WillOnce(Return(true));
    EXPECT_CALL(mock_pq_ctx_, SetupRowCountTracking(pq_eth_.GetRowGroupRowCount(), 
        pq_eth_.GetRowGroupBufferCount(), true, "EthernetF0")).WillOnce(Return(false));

    ASSERT_FALSE(pq_eth_.Initialize(outf_, thread_id_));
    EXPECT_EQ(thread_id_, pq_eth_.thread_id_);
    EXPECT_EQ(pq_eth_.payload_ptr_, pq_eth_.payload_.data());
    ValidateInitializeResize();
}

TEST_F(ParquetEthernetF0Test, AppendIncrementAndWriteFalse)
{
    EXPECT_CALL(mock_pq_ctx_, OpenForWrite(outf_.string(), truncate_)).WillOnce(Return(true));
    EXPECT_CALL(mock_pq_ctx_, SetupRowCountTracking(pq_eth_.GetRowGroupRowCount(), 
        pq_eth_.GetRowGroupBufferCount(), true, "EthernetF0")).WillOnce(Return(true));
    EXPECT_CALL(mock_pq_ctx_, IncrementAndWrite(thread_id_)).WillOnce(Return(false));

    ASSERT_TRUE(pq_eth_.Initialize(outf_, thread_id_));
    pq_eth_.Append(time_stamp_, channel_id_, &eth_data_);

    ValidateAppendedData();
}

TEST_F(ParquetEthernetF0Test, AppendIncrementAndWriteTrue)
{
    EXPECT_CALL(mock_pq_ctx_, OpenForWrite(outf_.string(), truncate_)).WillOnce(Return(true));
    EXPECT_CALL(mock_pq_ctx_, SetupRowCountTracking(pq_eth_.GetRowGroupRowCount(), 
        pq_eth_.GetRowGroupBufferCount(), true, "EthernetF0")).WillOnce(Return(true));
    EXPECT_CALL(mock_pq_ctx_, IncrementAndWrite(thread_id_)).WillOnce(Return(true));

    ASSERT_TRUE(pq_eth_.Initialize(outf_, thread_id_));
    pq_eth_.Append(time_stamp_, channel_id_, &eth_data_);

    // Zero comparison payload
    std::fill(eth_data_.payload_.data(), eth_data_.payload_.data() + pq_eth_.GetDataPayloadListElementCount(), 0);
    ValidateAppendedData();
}