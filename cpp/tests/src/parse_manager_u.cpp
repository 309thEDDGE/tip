#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "parse_manager.h"
#include "parser_config_params.h"
#include "ch10_header_format.h"
#include "ch10_1553f1_msg_hdr_format.h"

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
    std::string input_path = "";
    std::string output_path = "";
    //ParseManager pm = ParseManager(input_path, output_path, &config);
    ParseManager pm;
    std::string tmats_filename_ = "_TMATS.txt";
    std::ifstream file;
    std::string line;
    ManagedPath base_output_dir_;
    ManagedPath base_name_;
    ManagedPath full_output_dir_;
    ManagedPath tmats_path_;
    bool result_;
    std::map<Ch10PacketType, bool> pkt_enabled_map_;
    std::map<Ch10PacketType, std::string> append_str_map_;
    std::map<Ch10PacketType, ManagedPath> output_dir_map_;
    std::map<std::string, std::string> TMATsChannelIDToSourceMap_;
    std::map<std::string, std::string> TMATsChannelIDToTypeMap_;
    WorkerConfig worker_config_;
    BinBuff* bb_ptr_;
    MockBinBuffParseManager mock_bb_;

    ParseManagerTest() : result_(false),
                         tmats_path_(),
                         mock_bb_(),
                         bb_ptr_(&mock_bb_),
                         file()
    {
        tmats_path_ /= tmats_filename_;
    }

    void RemoveFile()
    {
        // delete previous file if it exists
        file.open(tmats_filename_);
        if (file.good())
        {
            file.close();
            remove(tmats_filename_.c_str());
        }
        file.close();
    }

    ~ParseManagerTest()
    {
    }

    template <typename Map>
    bool map_compare(Map const& lhs, Map const& rhs)
    {
        return lhs.size() == rhs.size() && std::equal(lhs.begin(), lhs.end(),
                                                      rhs.begin());
    }

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
            "worker_shift_wait_ms: 200\n"};

        return config.InitializeWithConfigString(config_yaml);
    }

    void SetUp() override
    {
        // delete file if it was there when the test starts
        std::remove(tmats_filename_.c_str());
    }
};

TEST_F(ParseManagerTest, NoTMATSLeftFromPriorTests)
{
    file.open(tmats_filename_);
    ASSERT_FALSE(file.good());
    RemoveFile();
}

TEST_F(ParseManagerTest, NoTMATSPresent)
{
    std::vector<std::string> tmats;
    pm.ProcessTMATS(tmats, tmats_path_, TMATsChannelIDToSourceMap_,
                    TMATsChannelIDToTypeMap_);
    file.open(tmats_filename_);
    // file shouldn't exist if tmats did
    // not exist
    ASSERT_FALSE(file.good());
    RemoveFile();
}

TEST_F(ParseManagerTest, TMATSWritten)
{
    std::vector<std::string> tmats = {"line1\nline2\n", "line3\nline4\n"};
    pm.ProcessTMATS(tmats, tmats_path_, TMATsChannelIDToSourceMap_,
                    TMATsChannelIDToTypeMap_);
    file.open(tmats_filename_);
    ASSERT_TRUE(file.good());
    std::ostringstream ss;
    ss << file.rdbuf();
    std::string test = ss.str();
    ASSERT_EQ(test, "line1\nline2\nline3\nline4\n");
    RemoveFile();
}

TEST_F(ParseManagerTest, TMATSParsed)
{
    // R-x\TK1-n:channelID
    // R-x\DSI-n:Source
    // R-x\CDT-n:Type
    std::vector<std::string> tmats = {
        "line2;\n\n",
        "R-1\\TK1-1:1;\n\n",
        "R-2\\TK1-2:2;\n\n",
        "R-2\\DSI-2:Bus2;\n\n",
        "R-1\\DSI-1:Bus1;\n",
        "R-3\\TK1-3:3;\n\n",
        "R-1\\CDT-1:type1;\n\n",
        "R-2\\CDT-2:type2;\n\n",
        "R-3\\CDT-3:type3;\n",
        "R-3\\DSI-3:Bus3;\n\n",
        "comment;\n",
        "Junk-1\\Junk1-1;\n\n",
    };
    pm.ProcessTMATS(tmats, tmats_path_, TMATsChannelIDToSourceMap_,
                    TMATsChannelIDToTypeMap_);

    std::map<std::string, std::string> source_map_truth =
        {{"1", "Bus1"}, {"2", "Bus2"}, {"3", "Bus3"}};

    std::map<std::string, std::string> type_map_truth =
        {{"1", "type1"}, {"2", "type2"}, {"3", "type3"}};

    ASSERT_TRUE(map_compare(TMATsChannelIDToSourceMap_, source_map_truth));
    ASSERT_TRUE(map_compare(TMATsChannelIDToTypeMap_, type_map_truth));
    RemoveFile();
}

TEST_F(ParseManagerTest, ConvertCh10PacketTypeMapEmptyMap)
{
    std::map<std::string, std::string> input_map;
    std::map<Ch10PacketType, bool> output_map;
    bool res = pm.ConvertCh10PacketTypeMap(input_map, output_map);
    EXPECT_FALSE(res);
}

TEST_F(ParseManagerTest, ConvertCh10PacketTypeMapInvalidPacketName)
{
    std::map<std::string, std::string> input_map = {
        {"MILSTD1553_FORMAT1", "true"},
        {"VIDEO_FORMAT", "true"}  // VIDEO_FORMAT0 is possible, not without trailing "0"
    };
    std::map<Ch10PacketType, bool> output_map;
    bool res = pm.ConvertCh10PacketTypeMap(input_map, output_map);
    EXPECT_FALSE(res);
    EXPECT_EQ(output_map.size(), 0);
}

TEST_F(ParseManagerTest, ConvertCh10PacketTypeMapInvalidBooleanString)
{
    std::map<std::string, std::string> input_map = {
        {"MILSTD1553_FORMAT1", "tru"},  // "tru" is not a valid boolean string
        {"VIDEO_FORMAT0", "true"}};
    std::map<Ch10PacketType, bool> output_map;
    bool res = pm.ConvertCh10PacketTypeMap(input_map, output_map);
    EXPECT_FALSE(res);
    EXPECT_EQ(output_map.size(), 0);
}

TEST_F(ParseManagerTest, ConvertCh10PacketTypeMapCorrectMapping)
{
    std::map<std::string, std::string> input_map = {
        {"MILSTD1553_FORMAT1", "false"},
        {"VIDEO_FORMAT0", "true"}};
    std::map<Ch10PacketType, bool> output_map;
    bool res = pm.ConvertCh10PacketTypeMap(input_map, output_map);
    EXPECT_TRUE(res);
    EXPECT_EQ(output_map.count(Ch10PacketType::MILSTD1553_F1), 1);
    EXPECT_EQ(output_map.count(Ch10PacketType::VIDEO_DATA_F0), 1);

    EXPECT_EQ(output_map.at(Ch10PacketType::MILSTD1553_F1), false);
    EXPECT_EQ(output_map.at(Ch10PacketType::VIDEO_DATA_F0), true);

    input_map["MILSTD1553_FORMAT1"] = "True";
    input_map["VIDEO_FORMAT0"] = "fAlse";
    output_map.clear();
    res = pm.ConvertCh10PacketTypeMap(input_map, output_map);
    EXPECT_TRUE(res);
    EXPECT_EQ(output_map.count(Ch10PacketType::MILSTD1553_F1), 1);
    EXPECT_EQ(output_map.count(Ch10PacketType::VIDEO_DATA_F0), 1);

    EXPECT_EQ(output_map.at(Ch10PacketType::MILSTD1553_F1), true);
    EXPECT_EQ(output_map.at(Ch10PacketType::VIDEO_DATA_F0), false);
}

TEST_F(ParseManagerTest, CreateCh10PacketOutputDirsMissingAppendStr)
{
    pkt_enabled_map_[Ch10PacketType::MILSTD1553_F1] = true;
    pkt_enabled_map_[Ch10PacketType::VIDEO_DATA_F0] = false;

    // No append string entry for 1553
    append_str_map_[Ch10PacketType::VIDEO_DATA_F0] = "_video.parquet";

    result_ = pm.CreateCh10PacketOutputDirs(base_output_dir_, base_name_,
                                            pkt_enabled_map_, append_str_map_, output_dir_map_, false);
    EXPECT_FALSE(result_);
}

TEST_F(ParseManagerTest, CreateCh10PacketOutputDirsEmptyOutputOnFailure)
{
    pkt_enabled_map_[Ch10PacketType::MILSTD1553_F1] = true;
    pkt_enabled_map_[Ch10PacketType::VIDEO_DATA_F0] = true;

    append_str_map_[Ch10PacketType::VIDEO_DATA_F0] = "_video.parquet";
    append_str_map_[Ch10PacketType::MILSTD1553_F1] = "_1553.parquet";

    // Create empty base_output_dir_ to evoke failure.
    base_output_dir_ = ManagedPath(std::string(""));
    result_ = pm.CreateCh10PacketOutputDirs(base_output_dir_, base_name_,
                                            pkt_enabled_map_, append_str_map_, output_dir_map_, false);
    EXPECT_FALSE(result_);
    EXPECT_EQ(output_dir_map_.size(), 0);
}

TEST_F(ParseManagerTest, CreateCh10PacketOutputDirsCorrectDirs)
{
    pkt_enabled_map_[Ch10PacketType::MILSTD1553_F1] = true;
    pkt_enabled_map_[Ch10PacketType::VIDEO_DATA_F0] = true;

    append_str_map_[Ch10PacketType::VIDEO_DATA_F0] = "_video.parquet";
    append_str_map_[Ch10PacketType::MILSTD1553_F1] = "_1553.parquet";

    base_name_ = ManagedPath(std::string("my_data"));
    std::string expected_video = (base_output_dir_ / base_name_).RawString() +
                                 append_str_map_.at(Ch10PacketType::VIDEO_DATA_F0);
    std::string expected_1553 = (base_output_dir_ / base_name_).RawString() +
                                append_str_map_.at(Ch10PacketType::MILSTD1553_F1);
    result_ = pm.CreateCh10PacketOutputDirs(base_output_dir_, base_name_,
                                            pkt_enabled_map_, append_str_map_, output_dir_map_, false);
    EXPECT_TRUE(result_);
    EXPECT_EQ(output_dir_map_.size(), 2);
    EXPECT_EQ(expected_video, output_dir_map_.at(Ch10PacketType::VIDEO_DATA_F0));
    EXPECT_EQ(expected_1553, output_dir_map_.at(Ch10PacketType::MILSTD1553_F1));
}

TEST_F(ParseManagerTest, CreateCh10PacketWorkerFileNamesEmptyDirMap)
{
    // output_dir_map_ is empty by default
    uint16_t worker_count = 3;
    std::vector<std::map<Ch10PacketType, ManagedPath>> vec_mapped_paths;
    std::string ext = "";

    pm.CreateCh10PacketWorkerFileNames(worker_count, output_dir_map_,
                                       vec_mapped_paths, ext);
    EXPECT_EQ(0, vec_mapped_paths.size());
}

TEST_F(ParseManagerTest, CreateCh10PacketWorkerFileNamesEmptyExtension)
{
    output_dir_map_[Ch10PacketType::VIDEO_DATA_F0] = ManagedPath() / "video_data";
    uint16_t worker_count = 3;
    std::vector<std::map<Ch10PacketType, ManagedPath>> vec_mapped_paths;
    std::string ext = "";
    ManagedPath expected = ManagedPath() / "video_data" / "000";
    pm.CreateCh10PacketWorkerFileNames(worker_count, output_dir_map_,
                                       vec_mapped_paths, ext);
    EXPECT_EQ(worker_count, vec_mapped_paths.size());
    EXPECT_EQ(expected.RawString(), vec_mapped_paths[0].at(
                                                           Ch10PacketType::VIDEO_DATA_F0)
                                        .RawString());
}

TEST_F(ParseManagerTest, CreateCh10PacketWorkerFileNamesNonEmptyExtension)
{
    output_dir_map_[Ch10PacketType::VIDEO_DATA_F0] = ManagedPath() / "video_data";
    uint16_t worker_count = 3;
    std::vector<std::map<Ch10PacketType, ManagedPath>> vec_mapped_paths;
    std::string ext = "Extension";
    std::string full_ext = ".";
    full_ext += ext;
    ManagedPath expected = ManagedPath() / "video_data" / ("000" + full_ext);
    pm.CreateCh10PacketWorkerFileNames(worker_count, output_dir_map_,
                                       vec_mapped_paths, ext);
    EXPECT_EQ(worker_count, vec_mapped_paths.size());
    EXPECT_EQ(expected.RawString(), vec_mapped_paths[0].at(
                                                           Ch10PacketType::VIDEO_DATA_F0)
                                        .RawString());
}

TEST_F(ParseManagerTest, CreateCh10PacketWorkerFileNamesMultipleTypes)
{
    output_dir_map_[Ch10PacketType::VIDEO_DATA_F0] = ManagedPath() / "video_data";
    output_dir_map_[Ch10PacketType::MILSTD1553_F1] = ManagedPath() / "1553_data";
    uint16_t worker_count = 20;
    std::vector<std::map<Ch10PacketType, ManagedPath>> vec_mapped_paths;
    std::string ext = "pq";
    std::string full_ext = ".";
    full_ext += ext;
    ManagedPath expected1 = ManagedPath() / "video_data" / ("015" + full_ext);
    ManagedPath expected2 = ManagedPath() / "1553_data" / ("005" + full_ext);
    pm.CreateCh10PacketWorkerFileNames(worker_count, output_dir_map_,
                                       vec_mapped_paths, ext);
    EXPECT_EQ(worker_count, vec_mapped_paths.size());
    EXPECT_EQ(expected1.RawString(), vec_mapped_paths[15].at(
                                                             Ch10PacketType::VIDEO_DATA_F0)
                                         .RawString());
    EXPECT_EQ(expected2.RawString(), vec_mapped_paths[5].at(
                                                            Ch10PacketType::MILSTD1553_F1)
                                         .RawString());
}

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

TEST_F(ParseManagerTest, CombineChannelIDToLRUAddressesMetadataUnequalLengthVectors)
{
    std::map<uint32_t, std::set<uint16_t>> output;
    std::vector<std::map<uint32_t, std::set<uint16_t>>> chanid_lruaddr1_maps;
    std::vector<std::map<uint32_t, std::set<uint16_t>>> chanid_lruaddr2_maps;
    std::map<uint32_t, std::set<uint16_t>> map1_0;
    chanid_lruaddr1_maps.push_back(map1_0);

    result_ = pm.CombineChannelIDToLRUAddressesMetadata(output, chanid_lruaddr1_maps,
                                                        chanid_lruaddr2_maps);
    EXPECT_FALSE(result_);
}

TEST_F(ParseManagerTest, CombineChannelIDToLRUAddressesMetadata)
{
    std::map<uint32_t, std::set<uint16_t>> output;
    std::vector<std::map<uint32_t, std::set<uint16_t>>> chanid_lruaddr1_maps;
    std::vector<std::map<uint32_t, std::set<uint16_t>>> chanid_lruaddr2_maps;
    std::map<uint32_t, std::set<uint16_t>> map1_0 = {
        {5, {10, 12}},
        {10, {11, 13}}};
    std::map<uint32_t, std::set<uint16_t>> map1_1 = {
        {5, {10, 15}},
    };
    std::map<uint32_t, std::set<uint16_t>> map2_0 = {
        {6, {10, 12}},
        {7, {9, 10}}};
    std::map<uint32_t, std::set<uint16_t>> map2_1 = {
        {8, {1, 2}},
        {9, {3, 4}}};

    chanid_lruaddr1_maps.push_back(map1_0);
    chanid_lruaddr1_maps.push_back(map1_1);

    chanid_lruaddr2_maps.push_back(map2_0);
    chanid_lruaddr2_maps.push_back(map2_1);

    std::map<uint32_t, std::set<uint16_t>> expected = {
        {5, {10, 12, 15}},
        {6, {10, 12}},
        {7, {9, 10}},
        {8, {1, 2}},
        {9, {3, 4}},
        {10, {11, 13}}};

    result_ = pm.CombineChannelIDToLRUAddressesMetadata(output, chanid_lruaddr1_maps,
                                                        chanid_lruaddr2_maps);
    EXPECT_TRUE(result_);
    EXPECT_EQ(expected, output);
}

TEST_F(ParseManagerTest, CombineChannelIDToCommandWordsMetadata)
{
    std::vector<std::map<uint32_t, std::set<uint32_t>>> chanid_commwords_maps;
    std::map<uint32_t, std::vector<std::vector<uint32_t>>> orig_commwords1 = {
        {16, {{10, 4211}, {752, 1511}}},
        {17, {{9, 23}}}};
    std::map<uint32_t, std::vector<std::vector<uint32_t>>> orig_commwords2 = {
        {16, {{10, 4211}, {1992, 1066}}},
        {18, {{900, 211}, {11, 9341}, {41, 55}}}};

    std::map<uint32_t, std::vector<std::vector<uint32_t>>>::const_iterator it;
    std::map<uint32_t, std::set<uint32_t>> chanid_commwords_map1;
    for (it = orig_commwords1.cbegin(); it != orig_commwords1.cend(); ++it)
    {
        std::set<uint32_t> temp_set;
        for (std::vector<std::vector<uint32_t>>::const_iterator it2 = it->second.cbegin();
             it2 != it->second.cend(); ++it2)
        {
            temp_set.insert((it2->at(0) << 16) + it2->at(1));
        }
        chanid_commwords_map1[it->first] = temp_set;
    }
    chanid_commwords_maps.push_back(chanid_commwords_map1);

    std::map<uint32_t, std::set<uint32_t>> chanid_commwords_map2;
    for (it = orig_commwords2.cbegin(); it != orig_commwords2.cend(); ++it)
    {
        std::set<uint32_t> temp_set;
        for (std::vector<std::vector<uint32_t>>::const_iterator it2 = it->second.cbegin();
             it2 != it->second.cend(); ++it2)
        {
            temp_set.insert((it2->at(0) << 16) + it2->at(1));
        }
        chanid_commwords_map2[it->first] = temp_set;
    }
    chanid_commwords_maps.push_back(chanid_commwords_map2);

    std::map<uint32_t, std::vector<std::vector<uint32_t>>> output_map;
    result_ = pm.CombineChannelIDToCommandWordsMetadata(output_map, chanid_commwords_maps);
    EXPECT_TRUE(result_);
    EXPECT_EQ(3, output_map.size());
    EXPECT_THAT(output_map.at(16),
                ::testing::Contains(::testing::ElementsAreArray({10, 4211})));
    EXPECT_THAT(output_map.at(16),
                ::testing::Contains(::testing::ElementsAreArray({752, 1511})));
    EXPECT_THAT(output_map.at(16),
                ::testing::Contains(::testing::ElementsAreArray({1992, 1066})));

    EXPECT_THAT(output_map.at(17),
                ::testing::Contains(::testing::ElementsAreArray({9, 23})));

    EXPECT_THAT(output_map.at(18),
                ::testing::Contains(::testing::ElementsAreArray({900, 211})));
    EXPECT_THAT(output_map.at(18),
                ::testing::Contains(::testing::ElementsAreArray({11, 9341})));
    EXPECT_THAT(output_map.at(18),
                ::testing::Contains(::testing::ElementsAreArray({41, 55})));
}

TEST_F(ParseManagerTest, CreateChannelIDToMinVideoTimestampsMetadata)
{
    std::map<uint16_t, uint64_t> output_map;
    std::vector<std::map<uint16_t, uint64_t>> input_maps = {
        {{12, 100}, {13, 120}, {14, 98}},
        {{12, 100}, {13, 110}, {14, 200}},
        {{12, 120}, {13, 108}, {14, 150}}};
    std::map<uint16_t, uint64_t> expected = {
        {12, 100}, {13, 108}, {14, 98}};
    pm.CreateChannelIDToMinVideoTimestampsMetadata(output_map,
                                                   input_maps);
    EXPECT_EQ(expected, output_map);
}

TEST_F(ParseManagerTest, CombineChannelIDToLabelsMetadata)
{
    std::map<uint32_t, std::set<uint16_t>> output;
    std::vector<std::map<uint32_t, std::set<uint16_t>>> chanid_labels_maps;

    std::map<uint32_t, std::set<uint16_t>> map1_0 = {
        {5, {194, 202}},
        {10, {193, 202}}};
    std::map<uint32_t, std::set<uint16_t>> map1_1 = {
        {5, {201}},
        {10, {67}}};

    chanid_labels_maps.push_back(map1_0);
    chanid_labels_maps.push_back(map1_1);

    std::map<uint32_t, std::set<uint16_t>> expected = {
        {5, {194, 201, 202}},
        {10, {67, 193, 202}}};

    result_ = pm.CombineChannelIDToLabelsMetadata(output,
                                                  chanid_labels_maps);
    EXPECT_TRUE(result_);
    EXPECT_EQ(expected, output);
}

TEST_F(ParseManagerTest, CombineChannelIDToBusNumbersMetadata)
{
    std::map<uint32_t, std::set<uint16_t>> output;
    std::vector<std::map<uint32_t, std::set<uint16_t>>> chanid_busnumbers_maps;

    std::map<uint32_t, std::set<uint16_t>> map1_0 = {
        {5, {1, 2}},
        {10, {1, 2, 3}}};
    std::map<uint32_t, std::set<uint16_t>> map1_1 = {
        {5, {5}},
        {10, {3, 4}}};

    chanid_busnumbers_maps.push_back(map1_0);
    chanid_busnumbers_maps.push_back(map1_1);

    std::map<uint32_t, std::set<uint16_t>> expected = {
        {5, {1, 2, 5}},
        {10, {1, 2, 3, 4}}};

    result_ = pm.CombineChannelIDToBusNumbersMetadata(output,
                                                  chanid_busnumbers_maps);
    EXPECT_TRUE(result_);
    EXPECT_EQ(expected, output);
}
