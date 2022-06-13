#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "parser_paths.h"

class ParserPathsTest : public ::testing::Test
{
   protected:
    ParserPaths pp_;
    ManagedPath ch10_path_;
    ManagedPath output_dir_;
    ManagedPath base_output_dir_;
    ManagedPath base_name_;
    ManagedPath full_output_dir_;
    std::map<Ch10PacketType, bool> pkt_enabled_map_;
    std::map<Ch10PacketType, ManagedPath> output_dir_map_;
    uint16_t worker_count_;

    ParserPathsTest() : pp_(), ch10_path_{"path.ch10"}, output_dir_{"outdir"},
    base_output_dir_(), base_name_(), full_output_dir_(), worker_count_(0)
    {}
};

TEST_F(ParserPathsTest, CreateOutputPathsSetPaths)
{
    ASSERT_TRUE(pp_.CreateOutputPaths(ch10_path_, output_dir_, pkt_enabled_map_, worker_count_));
    EXPECT_EQ(ch10_path_.RawString(), pp_.GetCh10Path().RawString());
    EXPECT_EQ(output_dir_.RawString(), pp_.GetOutputDir().RawString());
    EXPECT_THAT(pkt_enabled_map_, ::testing::ContainerEq(pp_.GetCh10PacketTypeEnabledMap()));
}

TEST_F(ParserPathsTest, CreateOutputPathsTMATSPath)
{
    ASSERT_TRUE(pp_.CreateOutputPaths(ch10_path_, output_dir_, pkt_enabled_map_, worker_count_));
    ManagedPath expected = output_dir_.CreatePathObject(ch10_path_, "_" + 
        ch10packettype_to_string_map.at(Ch10PacketType::COMPUTER_GENERATED_DATA_F1) + ".txt");
    EXPECT_EQ(expected.RawString(), pp_.GetTMATSOutputPath().RawString());
}

TEST_F(ParserPathsTest, CreateOutputPathsTDPPath)
{
    ASSERT_TRUE(pp_.CreateOutputPaths(ch10_path_, output_dir_, pkt_enabled_map_, worker_count_));
    ManagedPath expected = output_dir_.CreatePathObject(ch10_path_, "_" + 
        ch10packettype_to_string_map.at(Ch10PacketType::TIME_DATA_F1) + ".parquet");
    EXPECT_EQ(expected.RawString(), pp_.GetTDPOutputPath().RawString());
}

TEST_F(ParserPathsTest, CreateCh10PacketOutputDirsEmptyOutputOnFailure)
{
    pkt_enabled_map_[Ch10PacketType::MILSTD1553_F1] = true;
    pkt_enabled_map_[Ch10PacketType::VIDEO_DATA_F0] = true;

    // Create empty base_output_dir_ to evoke failure.
    base_output_dir_ = ManagedPath(std::string(""));
    ASSERT_FALSE(pp_.CreateCh10PacketOutputDirs(base_output_dir_, base_name_,
                                            pkt_enabled_map_, output_dir_map_, false));
    EXPECT_EQ(output_dir_map_.size(), 0);
}

TEST_F(ParserPathsTest, CreateCh10PacketOutputDirsCorrectDirs)
{
    pkt_enabled_map_[Ch10PacketType::MILSTD1553_F1] = true;
    pkt_enabled_map_[Ch10PacketType::VIDEO_DATA_F0] = true;

    base_name_ = ManagedPath(std::string("my_data"));
    std::string expected_video = (base_output_dir_ / base_name_).RawString() + "_" +
                                 ch10packettype_to_string_map.at(Ch10PacketType::VIDEO_DATA_F0) +
                                 ".parquet";
    std::string expected_1553 = (base_output_dir_ / base_name_).RawString() + "_" + 
                                ch10packettype_to_string_map.at(Ch10PacketType::MILSTD1553_F1) + 
                                ".parquet";
    ASSERT_TRUE(pp_.CreateCh10PacketOutputDirs(base_output_dir_, base_name_,
                                            pkt_enabled_map_, output_dir_map_, false));
    EXPECT_EQ(output_dir_map_.size(), 2);
    EXPECT_EQ(expected_video, output_dir_map_.at(Ch10PacketType::VIDEO_DATA_F0));
    EXPECT_EQ(expected_1553, output_dir_map_.at(Ch10PacketType::MILSTD1553_F1));
}

TEST_F(ParserPathsTest, CreateOutputPathsCh10PacketOutputDirsCreated)
{
    pkt_enabled_map_[Ch10PacketType::MILSTD1553_F1] = true;
    pkt_enabled_map_[Ch10PacketType::VIDEO_DATA_F0] = true;

    ch10_path_ = ManagedPath("file.txt");
    output_dir_ = ManagedPath{"ParserPaths_out"};
    ASSERT_TRUE(output_dir_.create_directory());
    ASSERT_TRUE(pp_.CreateOutputPaths(ch10_path_, output_dir_, pkt_enabled_map_, worker_count_));
    EXPECT_EQ(2, pp_.GetCh10PacketTypeOutputDirMap().size());
    ASSERT_TRUE(output_dir_.RemoveTree());
}

TEST_F(ParserPathsTest, CreateCh10PacketWorkerFileNamesEmptyDirMap)
{
    // output_dir_map_ is empty by default
    uint16_t worker_count = 3;
    std::vector<std::map<Ch10PacketType, ManagedPath>> vec_mapped_paths;
    std::string ext = "";

    pp_.CreateCh10PacketWorkerFileNames(worker_count, output_dir_map_,
                                       vec_mapped_paths, ext);
    EXPECT_EQ(0, vec_mapped_paths.size());
}

TEST_F(ParserPathsTest, CreateCh10PacketWorkerFileNamesEmptyExtension)
{
    output_dir_map_[Ch10PacketType::VIDEO_DATA_F0] = ManagedPath() / "video_data";
    uint16_t worker_count = 3;
    std::vector<std::map<Ch10PacketType, ManagedPath>> vec_mapped_paths;
    std::string ext = "";
    ManagedPath expected = ManagedPath() / "video_data" / "000";
    pp_.CreateCh10PacketWorkerFileNames(worker_count, output_dir_map_,
                                       vec_mapped_paths, ext);
    EXPECT_EQ(worker_count, vec_mapped_paths.size());
    EXPECT_EQ(expected.RawString(), vec_mapped_paths[0].at(
                                                           Ch10PacketType::VIDEO_DATA_F0)
                                        .RawString());
}

TEST_F(ParserPathsTest, CreateCh10PacketWorkerFileNamesNonEmptyExtension)
{
    output_dir_map_[Ch10PacketType::VIDEO_DATA_F0] = ManagedPath() / "video_data";
    uint16_t worker_count = 3;
    std::vector<std::map<Ch10PacketType, ManagedPath>> vec_mapped_paths;
    std::string ext = "Extension";
    std::string full_ext = ".";
    full_ext += ext;
    ManagedPath expected = ManagedPath() / "video_data" / ("000" + full_ext);
    pp_.CreateCh10PacketWorkerFileNames(worker_count, output_dir_map_,
                                       vec_mapped_paths, ext);
    EXPECT_EQ(worker_count, vec_mapped_paths.size());
    EXPECT_EQ(expected.RawString(), vec_mapped_paths[0].at(
                                                           Ch10PacketType::VIDEO_DATA_F0)
                                        .RawString());
}

TEST_F(ParserPathsTest, CreateCh10PacketWorkerFileNamesMultipleTypes)
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
    pp_.CreateCh10PacketWorkerFileNames(worker_count, output_dir_map_,
                                       vec_mapped_paths, ext);
    EXPECT_EQ(worker_count, vec_mapped_paths.size());
    EXPECT_EQ(expected1.RawString(), vec_mapped_paths[15].at(
                                                             Ch10PacketType::VIDEO_DATA_F0)
                                         .RawString());
    EXPECT_EQ(expected2.RawString(), vec_mapped_paths[5].at(
                                                            Ch10PacketType::MILSTD1553_F1)
                                         .RawString());
}

TEST_F(ParserPathsTest, CreateOutputPathsWorkerPathsCreated)
{
    worker_count_ = 12;
    ch10_path_ = ManagedPath("file.txt");
    output_dir_ = ManagedPath{"ParserPaths_out"};
    ASSERT_TRUE(output_dir_.create_directory());
    pkt_enabled_map_[Ch10PacketType::MILSTD1553_F1] = true;
    ASSERT_TRUE(pp_.CreateOutputPaths(ch10_path_, output_dir_, pkt_enabled_map_, worker_count_));
    EXPECT_EQ(worker_count_, pp_.GetWorkerPathVec().size()); 
    ASSERT_TRUE(output_dir_.RemoveTree());
}
