#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "parse_manager.h"
#include "tmats_data.h"
#include "parser_config_params.h"
#include "ch10_header_format.h"
#include "ch10_1553f1_msg_hdr_format.h"
#include "parquet_tdpf1_mock.h"
#include "ch10_tdpf1_hdr_format.h"

using ::testing::Return;

// Mock BinBuff
class MockBinBuffParseManager : public BinBuff
{
   public:
    MockBinBuffParseManager() : BinBuff() {}
    MOCK_METHOD4(Initialize, uint64_t(std::ifstream& file,
                                      const uint64_t& file_size, const uint64_t& read_pos,
                                      const uint64_t& read_count));
};

class ParseManagerTest : public ::testing::Test
{
   protected:
    ParserConfigParams config;
    ParseManager pm;
    std::ifstream file;
    std::string line;
    bool result_;
    std::map<Ch10PacketType, bool> pkt_enabled_map_;
    std::map<Ch10PacketType, ManagedPath> output_dir_map_;
    WorkerConfig worker_config_;
    BinBuff* bb_ptr_;
    MockBinBuffParseManager mock_bb_;

    ParseManagerTest() : result_(false),
                         mock_bb_(),
                         bb_ptr_(&mock_bb_),
                         file()
    {}

    bool InitializeParserConfig()
    {
        std::string config_yaml = {
            "ch10_packet_type:\n"
            "  MILSTD1553_FORMAT1: true\n"
            "  VIDEO_FORMAT0 : true\n"
            "parse_chunk_bytes: 500\n"
            "parse_thread_count: 1\n"
            "max_chunk_read_count: 1000\n"
            "worker_offset_wait_ms: 200\n"
            "worker_shift_wait_ms: 200\n"
            "stdout_log_level: info\n"};

        return config.InitializeWithConfigString(config_yaml);
    }
};

TEST_F(ParseManagerTest, AllocateResourcesValidateCalculatedParams)
{
    EXPECT_TRUE(InitializeParserConfig());
    uint64_t file_size = 1e9;
    result_ = pm.AllocateResources(config, file_size);
    EXPECT_TRUE(result_);
    EXPECT_EQ(config.parse_chunk_bytes_ * 1e6, pm.worker_chunk_size_bytes);

    uint16_t expected_worker_count = int(ceil(float(file_size) /
                                              float(pm.worker_chunk_size_bytes)));
    EXPECT_EQ(expected_worker_count, pm.worker_count);
}

TEST_F(ParseManagerTest, AllocateResourcesConfirmVectorAllocations)
{
    EXPECT_TRUE(InitializeParserConfig());
    uint64_t file_size = 1e9;
    result_ = pm.AllocateResources(config, file_size);
    EXPECT_TRUE(result_);
    EXPECT_EQ(pm.worker_count, pm.workers_vec.size());
    EXPECT_EQ(pm.worker_count, pm.threads_vec.size());
    EXPECT_EQ(pm.worker_count, pm.worker_config_vec.size());
}

TEST_F(ParseManagerTest, AllocateResourcesConfirmParseWorkerAllocation)
{
    EXPECT_TRUE(InitializeParserConfig());
    uint64_t file_size = 1e9;
    result_ = pm.AllocateResources(config, file_size);
    EXPECT_TRUE(result_);
    EXPECT_EQ(pm.worker_count, pm.workers_vec.size());
    for (uint16_t worker_ind = 0; worker_ind < pm.worker_count; worker_ind++)
    {
        EXPECT_TRUE(pm.workers_vec[worker_ind].get() != nullptr);
    }
}

TEST_F(ParseManagerTest, ConfigureWorkerNotFinalWorker)
{
    pkt_enabled_map_[Ch10PacketType::MILSTD1553_F1] = true;

    // Note that elsewhere this map is used as the base path for various packet types,
    // whereas the map with the same prototype is used in ConfigureWorker to hold
    // the output paths specific to a worker. Here I'll use this map to hold some
    // arbitrary path for the sake of testing.
    output_dir_map_[Ch10PacketType::MILSTD1553_F1] = ManagedPath(std::string("test"));

    uint16_t worker_index = 5;
    uint16_t worker_count = 10;
    uint64_t read_pos = 13456641;
    uint64_t read_size = 250e6;
    std::streamsize actual_read_size = 0;
    uint64_t total_size = 500e6;

    EXPECT_CALL(mock_bb_, Initialize(::testing::Ref(file), total_size, read_pos, read_size))
        .Times(1)
        .WillOnce(::testing::Return(read_size));

    result_ = pm.ConfigureWorker(worker_config_, worker_index, worker_count, read_pos,
                                 read_size, total_size, bb_ptr_, file, actual_read_size,
                                 output_dir_map_, pkt_enabled_map_);
    EXPECT_TRUE(result_);

    EXPECT_EQ(false, worker_config_.final_worker_);
    EXPECT_EQ(worker_index, worker_config_.worker_index_);
    EXPECT_EQ(read_pos, worker_config_.start_position_);
    EXPECT_EQ(false, worker_config_.append_mode_);
    EXPECT_EQ(actual_read_size, read_size);
    EXPECT_EQ(output_dir_map_, worker_config_.output_file_paths_);
    EXPECT_EQ(pkt_enabled_map_, worker_config_.ch10_packet_type_map_);
}

TEST_F(ParseManagerTest, ConfigureWorkerFinalWorker)
{
    pkt_enabled_map_[Ch10PacketType::MILSTD1553_F1] = true;

    // Note that elsewhere this map is used as the base path for various packet types,
    // whereas the map with the same prototype is used in ConfigureWorker to hold
    // the output paths specific to a worker. Here I'll use this map to hold some
    // arbitrary path for the sake of testing.
    output_dir_map_[Ch10PacketType::MILSTD1553_F1] = ManagedPath(std::string("test"));

    uint16_t worker_index = 9;  // = worker_count - 1
    uint16_t worker_count = 10;
    uint64_t read_pos = 13456641;
    uint64_t read_size = 250e6;
    std::streamsize actual_read_size = 0;
    uint64_t total_size = 500e6;

    EXPECT_CALL(mock_bb_, Initialize(::testing::Ref(file), total_size, read_pos, read_size))
        .Times(1)
        .WillOnce(::testing::Return(read_size));

    result_ = pm.ConfigureWorker(worker_config_, worker_index, worker_count, read_pos,
                                 read_size, total_size, bb_ptr_, file, actual_read_size,
                                 output_dir_map_, pkt_enabled_map_);
    EXPECT_TRUE(result_);

    EXPECT_EQ(true, worker_config_.final_worker_);
    EXPECT_EQ(worker_index, worker_config_.worker_index_);
    EXPECT_EQ(read_pos, worker_config_.start_position_);
    EXPECT_EQ(false, worker_config_.append_mode_);
    EXPECT_EQ(actual_read_size, read_size);
    EXPECT_EQ(output_dir_map_, worker_config_.output_file_paths_);
    EXPECT_EQ(pkt_enabled_map_, worker_config_.ch10_packet_type_map_);
}

TEST_F(ParseManagerTest, ConfigureWorkeIndexLarge)
{
    uint16_t worker_index = 10;  // > worker_count - 1
    uint16_t worker_count = 10;
    uint64_t read_pos = 13456641;
    uint64_t read_size = 250e6;
    std::streamsize actual_read_size = 0;
    uint64_t total_size = 500e6;

    result_ = pm.ConfigureWorker(worker_config_, worker_index, worker_count, read_pos,
                                 read_size, total_size, bb_ptr_, file, actual_read_size,
                                 output_dir_map_, pkt_enabled_map_);
    EXPECT_FALSE(result_);
}

TEST_F(ParseManagerTest, ConfigureWorkerBufferNoInitError)
{
    uint16_t worker_index = 5;
    uint16_t worker_count = 10;
    uint64_t read_pos = 13456641;
    uint64_t read_size = 250e6;
    std::streamsize actual_read_size = 0;
    uint64_t total_size = 500e6;

    // Note the use of ::testing::Ref here! This took a couple hours of digging
    // and this test won't compile without it. Apparently in the EXPECT_CALL
    // construction, all of the expected values are passed by value, which of
    // course breaks when trying to pass something that doesn't have a copy
    // constructor such as std::ifstream. It was very confusing because the
    // mocked function for Initialize is defined with a std::ifstream& pass
    // by reference. That is different that what is used when one constructs
    // the expected values within the test.
    EXPECT_CALL(mock_bb_, Initialize(::testing::Ref(file), total_size, read_pos, read_size))
        .Times(1)
        .WillOnce(::testing::Return(read_size));

    result_ = pm.ConfigureWorker(worker_config_, worker_index, worker_count, read_pos,
                                 read_size, total_size, bb_ptr_, file, actual_read_size,
                                 output_dir_map_, pkt_enabled_map_);
    EXPECT_TRUE(result_);

    EXPECT_EQ(actual_read_size, read_size);
}

TEST_F(ParseManagerTest, ConfigureWorkerBufferInitError)
{
    uint16_t worker_index = 5;
    uint16_t worker_count = 10;
    uint64_t read_pos = 13456641;

    // Note: read_size > total_size. This value isn't actually used to determine
    // error or cause the error condition to be returned from BinBuff::Initialize.
    // I've set the value in example that it would cause an error condition in practice.
    // The error is set explicitly in EXPECT_CALL below.
    uint64_t read_size = 501e6;
    std::streamsize actual_read_size = 0;
    uint64_t total_size = 500e6;

    EXPECT_CALL(mock_bb_, Initialize(::testing::Ref(file), total_size, read_pos, read_size))
        .Times(1)
        .WillOnce(::testing::Return(UINT64_MAX));

    result_ = pm.ConfigureWorker(worker_config_, worker_index, worker_count, read_pos,
                                 read_size, total_size, bb_ptr_, file, actual_read_size,
                                 output_dir_map_, pkt_enabled_map_);
    EXPECT_FALSE(result_);

    EXPECT_EQ(UINT64_MAX, actual_read_size);
}

TEST_F(ParseManagerTest, ConfigureWorkerUnequalReadSizeLastWorker)
{
    uint16_t worker_index = 9;  // Last worker
    uint16_t worker_count = 10;
    uint64_t read_pos = 13456641;
    uint64_t read_size = 20e6;
    std::streamsize actual_read_size = 0;
    uint64_t total_size = 500e6;

    // Returning 15e6 < read_size = 20e6
    EXPECT_CALL(mock_bb_, Initialize(::testing::Ref(file), total_size, read_pos, read_size))
        .Times(1)
        .WillOnce(::testing::Return(15e6));

    result_ = pm.ConfigureWorker(worker_config_, worker_index, worker_count, read_pos,
                                 read_size, total_size, bb_ptr_, file, actual_read_size,
                                 output_dir_map_, pkt_enabled_map_);
    EXPECT_TRUE(result_);

    EXPECT_EQ(15e6, actual_read_size);
}

TEST_F(ParseManagerTest, ConfigureWorkerUnequalReadSizeNotLastWorker)
{
    uint16_t worker_index = 6;  // not ast worker
    uint16_t worker_count = 10;
    uint64_t read_pos = 13456641;
    uint64_t read_size = 20e6;
    std::streamsize actual_read_size = 0;
    uint64_t total_size = 500e6;

    // Returning 15e6 < read_size = 20e6
    EXPECT_CALL(mock_bb_, Initialize(::testing::Ref(file), total_size, read_pos, read_size))
        .Times(1)
        .WillOnce(::testing::Return(15e6));

    result_ = pm.ConfigureWorker(worker_config_, worker_index, worker_count, read_pos,
                                 read_size, total_size, bb_ptr_, file, actual_read_size,
                                 output_dir_map_, pkt_enabled_map_);
    EXPECT_FALSE(result_);

    EXPECT_EQ(15e6, actual_read_size);
}

TEST_F(ParseManagerTest, ConfigureAppendWorkerNoInitError)
{
    worker_config_.last_position_ = 4311993045;
    uint16_t worker_index = 1;
    uint64_t read_size = 250e6;
    std::streamsize actual_read_size = 0;
    uint64_t total_size = 500e6;

    EXPECT_CALL(mock_bb_, Initialize(::testing::Ref(file), total_size,
                                     worker_config_.last_position_, read_size))
        .Times(1)
        .WillOnce(::testing::Return(read_size));
    result_ = pm.ConfigureAppendWorker(worker_config_, worker_index, read_size, total_size,
                                       bb_ptr_, file, actual_read_size);
    EXPECT_TRUE(result_);
    EXPECT_EQ(worker_config_.last_position_, worker_config_.start_position_);
    EXPECT_EQ(read_size, actual_read_size);
}

TEST_F(ParseManagerTest, ConfigureAppendWorkerInitError)
{
    worker_config_.last_position_ = 4311993045;
    uint16_t worker_index = 1;
    uint64_t read_size = 250e6;
    std::streamsize actual_read_size = 0;
    uint64_t total_size = 500e6;

    EXPECT_CALL(mock_bb_, Initialize(::testing::Ref(file), total_size,
                                     worker_config_.last_position_, read_size))
        .Times(1)
        .WillOnce(::testing::Return(UINT64_MAX));
    result_ = pm.ConfigureAppendWorker(worker_config_, worker_index, read_size, total_size,
                                       bb_ptr_, file, actual_read_size);
    EXPECT_FALSE(result_);
    EXPECT_EQ(UINT64_MAX, actual_read_size);
}

TEST_F(ParseManagerTest, ConfigureAppendWorkerAppendReadSizeLarge)
{
    /*
	This test is created to address the case in which the
	default append_read_size (100MB) is greater than the
	dangling bytes at the end of the current worker chunk
	to the end of the file.
	*/
    worker_config_.last_position_ = 150e6;
    uint16_t worker_index = 1;
    uint64_t read_size = 100e6;
    std::streamsize actual_read_size = 0;
    uint64_t total_size = 220e6;

    uint64_t expected_read_size = (220 - 150) * 1e6;
    EXPECT_CALL(mock_bb_, Initialize(::testing::Ref(file), total_size,
                                     worker_config_.last_position_, read_size))
        .Times(1)
        .WillOnce(::testing::Return(
            expected_read_size));
    result_ = pm.ConfigureAppendWorker(worker_config_, worker_index, read_size, total_size,
                                       bb_ptr_, file, actual_read_size);
    EXPECT_TRUE(result_);
    EXPECT_EQ(expected_read_size, actual_read_size);
}

TEST_F(ParseManagerTest, ConfigureAppendWorkerUnequalReadSize)
{
    worker_config_.last_position_ = 100e6;
    uint16_t worker_index = 1;
    uint64_t read_size = 250e6;
    std::streamsize actual_read_size = 0;
    uint64_t total_size = 500e6;

    // Return 260e6 != read_size
    EXPECT_CALL(mock_bb_, Initialize(::testing::Ref(file), total_size,
                                     worker_config_.last_position_, read_size))
        .Times(1)
        .WillOnce(::testing::Return(260e6));
    result_ = pm.ConfigureAppendWorker(worker_config_, worker_index, read_size, total_size,
                                       bb_ptr_, file, actual_read_size);
    EXPECT_FALSE(result_);
}

TEST_F(ParseManagerTest, WriteTDPDataInitializeFail)
{
    ManagedPath out_path({"test.parquet"});
    
    // ParquetTDPF1 INitialized
    ParquetContext ctx;
    MockParquetTDPF1 pqtdp(&ctx);
    EXPECT_CALL(pqtdp, Initialize(out_path, 0)).WillOnce(Return(false));

    Ch10Context ctx1(13344545, 0);
    Ch10Context ctx2(848348, 1);
    std::vector<const Ch10Context*> ctx_vec{&ctx1, &ctx2};

    EXPECT_FALSE(pm.WriteTDPData(ctx_vec, &pqtdp, out_path));
}

TEST_F(ParseManagerTest, WriteTDPData)
{
    ManagedPath out_path({"test.parquet"});
    
    // ParquetTDPF1 INitialized
    ParquetContext ctx;
    MockParquetTDPF1 pqtdp(&ctx);
    EXPECT_CALL(pqtdp, Initialize(out_path, 0)).WillOnce(Return(true));

    Ch10Context ctx1(13344545, 0);
    Ch10Context ctx2(848348, 1);

    TDF1CSDWFmt tdcsdw1{};
    uint64_t abs_time1 = 483882;
    uint8_t tdp_doy = 0;
    ctx1.UpdateWithTDPData(abs_time1, tdp_doy, true, tdcsdw1);

    uint64_t abs_time2 = 88112311102;
    TDF1CSDWFmt tdcsdw2{};
    tdcsdw2.date_fmt = 1;
    tdcsdw2.time_fmt = 3;
    ctx1.UpdateWithTDPData(abs_time2, tdp_doy, false, tdcsdw2);

    uint64_t abs_time3 = 123481201;
    TDF1CSDWFmt tdcsdw3{};
    tdcsdw3.date_fmt = 0;
    tdcsdw3.time_fmt = 2;
    ctx2.UpdateWithTDPData(abs_time3, tdp_doy, true, tdcsdw3);
    std::vector<const Ch10Context*> ctx_vec{&ctx1, &ctx2};

    EXPECT_CALL(pqtdp, Append(abs_time1, tdcsdw1));
    EXPECT_CALL(pqtdp, Append(abs_time2, tdcsdw2));
    EXPECT_CALL(pqtdp, Append(abs_time3, tdcsdw3));

    EXPECT_CALL(pqtdp, Close(0));

    EXPECT_TRUE(pm.WriteTDPData(ctx_vec, &pqtdp, out_path));
}
