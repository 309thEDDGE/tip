#include <cstdio>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "parser_config_params.h"
#include "parser_metadata.h"
#include "md_category_map.h"
#include "tmats_data.h"
#include "sha256_tools.h"
#include "ch10_context_mock.h"
#include "parser_metadata_mock.h"
#include "tmats_data_mock.h"
#include "file_reader.h"
#include "parser_paths_mock.h"
#include "tip_md_document_mock.h"
#include "ch10_packet_type_specific_metadata_mock.h"
#include "parquet_tdpf1_mock.h"

using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::DoAll;
using ::testing::SetArgReferee;
using ::testing::_;
using ::testing::InSequence;
using ::testing::Sequence;

class ParserMetadataTest : public ::testing::Test
{
    protected:
        std::string tmats_filename_ = "_TMATS.txt";
        ManagedPath tmats_path_;
        ParserConfigParams config_;
        bool result_;
        ParserMetadata pm_;
        ParserMetadataFunctions pf_;
        std::ifstream file_;

        ParserMetadataTest() : pm_(), tmats_path_{tmats_filename_}, file_(), config_(),
            result_(false), pf_()
        {}

        void SetUp() override
        {
            // delete file if it was there when the test starts
            std::remove(tmats_filename_.c_str());
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
                "worker_shift_wait_ms: 200\n"
                "stdout_log_level: info\n"};

            return config_.InitializeWithConfigString(config_yaml);
        }


        template <typename Map>
        bool map_compare(Map const& lhs, Map const& rhs)
        {
            return lhs.size() == rhs.size() && std::equal(lhs.begin(), lhs.end(),
                                                        rhs.begin());
        }

        void RemoveFile()
        {
            // delete previous file if it exists
            file_.open(tmats_filename_);
            if (file_.good())
            {
                file_.close();
                remove(tmats_filename_.c_str());
            }
            file_.close();
        }

};

TEST_F(ParserMetadataTest, NoTMATSPresent)
{
    std::vector<std::string> tmats;
    TMATSData tmats_data;
    std::set<Ch10PacketType> parsed_pkt_types;
    pf_.ProcessTMATS(tmats, tmats_path_, tmats_data, parsed_pkt_types);
    file_.open(tmats_filename_);
    // file shouldn't exist if tmats did
    // not exist
    ASSERT_FALSE(file_.good());
    RemoveFile();
}

TEST_F(ParserMetadataTest, TMATSWritten)
{
    std::vector<std::string> tmats = {"line1\nline2\n", "line3\nline4\n"};
    TMATSData tmats_data;
    std::set<Ch10PacketType> parsed_pkt_types;
    pf_.ProcessTMATS(tmats, tmats_path_, tmats_data, parsed_pkt_types);
    file_.open(tmats_filename_);
    ASSERT_TRUE(file_.good());
    std::ostringstream ss;
    ss << file_.rdbuf();
    std::string test = ss.str();
    ASSERT_EQ(test, "line1\nline2\nline3\nline4\n");
    RemoveFile();
}

TEST_F(ParserMetadataTest, TMATSParsed)
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
    TMATSData tmats_data;
    std::set<Ch10PacketType> parsed_pkt_types;
    pf_.ProcessTMATS(tmats, tmats_path_, tmats_data, parsed_pkt_types);

    std::map<std::string, std::string> source_map_truth =
        {{"1", "Bus1"}, {"2", "Bus2"}, {"3", "Bus3"}};

    std::map<std::string, std::string> type_map_truth =
        {{"1", "type1"}, {"2", "type2"}, {"3", "type3"}};

    ASSERT_TRUE(map_compare(tmats_data.chanid_to_source_map, source_map_truth));
    ASSERT_TRUE(map_compare(tmats_data.chanid_to_type_map, type_map_truth));
    RemoveFile();
}

TEST_F(ParserMetadataTest, NoTMATSLeftFromPriorTests)
{
    file_.open(tmats_filename_);
    ASSERT_FALSE(file_.good());
    RemoveFile();
}

TEST_F(ParserMetadataTest, RecordProvenanceData)
{
    TIPMDDocument tipdoc;
    ManagedPath ch10path{"my.ch10"};
    std::string pkt_label = "TESTLABEL";

    ProvenanceData prov;
    prov.hash = "98346thwrg;oijhswgap98uyqw34t";
    prov.time = "2022-06-07 21:53:30";
    prov.tip_version = "c8b76f7d";

    pf_.RecordProvenanceData(&tipdoc, ch10path, pkt_label, prov);    

    std::string expected_type = "parsed_" + pkt_label;
    EXPECT_TRUE(tipdoc.type_category_->node.IsScalar());
    EXPECT_EQ(expected_type, tipdoc.type_category_->node.Scalar());

    std::string expected_uid = CalcSHA256(prov.hash + prov.time + prov.tip_version + pkt_label);
    EXPECT_TRUE(tipdoc.uid_category_->node.IsScalar());
    EXPECT_EQ(expected_uid, tipdoc.uid_category_->node.Scalar());

    YAML::Node prov_node = tipdoc.prov_category_->node;
    EXPECT_TRUE(prov_node.IsMap());
    YAML::Node resource_node_seq = prov_node["resource"];
    ASSERT_FALSE(resource_node_seq.IsNull());
    ASSERT_TRUE(resource_node_seq.IsSequence());
    ASSERT_EQ(1, resource_node_seq.size());
    YAML::Node resource_node_map = resource_node_seq[0];
    ASSERT_TRUE(resource_node_map.IsMap());
    std::map<std::string, std::string> resource_map = 
        resource_node_map.as<std::map<std::string, std::string>>();
    ASSERT_EQ(3, resource_map.size());
    std::map<std::string, std::string> expected_resource_map{
        {"type", "CH10"},
        {"path", ch10path.RawString()},
        {"uid", prov.hash} 
    };
    EXPECT_THAT(expected_resource_map, ::testing::ContainerEq(resource_map));

    YAML::Node prov_time_node = prov_node["time"];
    ASSERT_TRUE(prov_time_node.IsScalar());
    EXPECT_EQ(prov.time, prov_time_node.Scalar());

    YAML::Node prov_version_node = prov_node["version"];
    ASSERT_TRUE(prov_version_node.IsScalar());
    EXPECT_EQ(prov.tip_version, prov_version_node.Scalar());
}

TEST_F(ParserMetadataTest, RecordUserConfigData)
{
    std::shared_ptr<MDCategoryMap> cat_map = std::make_shared<MDCategoryMap>("config");

    ParserConfigParams config;
    std::map<std::string, std::string> pkt_type_map{
        {"entry1", "val1"},
        {"entry2", "val2"}
    };
    config.ch10_packet_type_map_ = pkt_type_map;

    int chunk_bytes = 4983828;
    config.parse_chunk_bytes_ = chunk_bytes;

    int thread_count = 5;
    config.parse_thread_count_ = thread_count;

    int read_count = 500;
    config.max_chunk_read_count_ = read_count;

    int offset_wait = 344;
    config.worker_offset_wait_ms_ = offset_wait;

    int shift_wait = 54832;
    config.worker_shift_wait_ms_ = shift_wait;

    std::string log_level = "warn";
    config.stdout_log_level_ = log_level;

    pf_.RecordUserConfigData(cat_map, config);

    ASSERT_TRUE(cat_map->node.IsMap());
    YAML::Node pkt_type_map_node = cat_map->node["ch10_packet_type"];
    ASSERT_TRUE(pkt_type_map_node.IsMap());
    EXPECT_THAT(pkt_type_map, ::testing::ContainerEq(
        pkt_type_map_node.as<std::map<std::string, std::string>>()));

    YAML::Node chunk_bytes_node = cat_map->node["parse_chunk_bytes"];
    ASSERT_TRUE(chunk_bytes_node.IsScalar());
    EXPECT_EQ(std::to_string(chunk_bytes), chunk_bytes_node.Scalar());

    YAML::Node thread_count_node = cat_map->node["parse_thread_count"];
    ASSERT_TRUE(thread_count_node.IsScalar());
    EXPECT_EQ(std::to_string(thread_count), thread_count_node.Scalar());

    YAML::Node read_count_node = cat_map->node["max_chunk_read_count"];
    ASSERT_TRUE(read_count_node.IsScalar());
    EXPECT_EQ(std::to_string(read_count), read_count_node.Scalar());

    YAML::Node offset_wait_node = cat_map->node["worker_offset_wait_ms"];
    ASSERT_TRUE(offset_wait_node.IsScalar());
    EXPECT_EQ(std::to_string(offset_wait), offset_wait_node.Scalar());

    YAML::Node shift_wait_node = cat_map->node["worker_shift_wait_ms"];
    ASSERT_TRUE(shift_wait_node.IsScalar());
    EXPECT_EQ(std::to_string(shift_wait), shift_wait_node.Scalar());

    YAML::Node log_level_node = cat_map->node["stdout_log_level"];
    ASSERT_TRUE(log_level_node.IsScalar());
    EXPECT_EQ(log_level, log_level_node.Scalar());
}

TEST_F(ParserMetadataTest, ProcessTMATSForTypeFilterTMATSTypeFail)
{
    TIPMDDocument tipdoc;
    Ch10PacketType pkt_type = Ch10PacketType::ARINC429_F0;
    MockTMATSData tmats_data;

    cmap chanid_type_map{{"11", "1553IN"}, {"21", "429IN"}};
    cmap chanid_type_filtered{{"typ1", "val1"}, {"typ2", "val2"}};
    cmap chanid_src_filtered{{"data1", "res1"}, {"data2", "res2"}};

    EXPECT_CALL(tmats_data, GetChannelIDToTypeMap()).WillOnce(ReturnRef(chanid_type_map));
    EXPECT_CALL(tmats_data, FilterTMATSType(chanid_type_map, pkt_type, _)).
        WillOnce(DoAll(SetArgReferee<2>(chanid_type_filtered), Return(false)));

    EXPECT_FALSE(pf_.ProcessTMATSForType(&tmats_data, &tipdoc, pkt_type));
}

TEST_F(ParserMetadataTest, ProcessTMATSForTypePass)
{
    TIPMDDocument tipdoc;
    Ch10PacketType pkt_type = Ch10PacketType::ARINC429_F0;
    MockTMATSData tmats_data;

    cmap chanid_type_map{{"11", "1553IN"}, {"21", "429IN"}};
    cmap chanid_type_filtered{{"typ1", "val1"}, {"typ2", "val2"}};
    cmap chanid_src_filtered{{"data1", "res1"}, {"data2", "res2"}};
    cmap chanid_src_map{{"11", "sourceA"}, {"21", "sourceB"}};

    EXPECT_CALL(tmats_data, GetChannelIDToTypeMap()).WillOnce(ReturnRef(chanid_type_map));
    EXPECT_CALL(tmats_data, FilterTMATSType(chanid_type_map, pkt_type, _)).
        WillOnce(DoAll(SetArgReferee<2>(chanid_type_filtered), Return(true)));
    EXPECT_CALL(tmats_data, GetChannelIDToSourceMap()).WillOnce(ReturnRef(chanid_src_map));
    EXPECT_CALL(tmats_data, FilterByChannelIDToType(chanid_type_filtered, chanid_src_map)).
        WillOnce(Return(chanid_src_filtered));

    EXPECT_TRUE(pf_.ProcessTMATSForType(&tmats_data, &tipdoc, pkt_type));

    YAML::Node chanid_src_node = tipdoc.runtime_category_->node["tmats_chanid_to_source"];
    ASSERT_TRUE(chanid_src_node.IsMap());
    EXPECT_THAT(chanid_src_filtered, ::testing::ContainerEq(chanid_src_node.as<cmap>()));

    YAML::Node chanid_type_node = tipdoc.runtime_category_->node["tmats_chanid_to_type"];
    ASSERT_TRUE(chanid_type_node.IsMap());
    EXPECT_THAT(chanid_type_filtered, ::testing::ContainerEq(chanid_type_node.as<cmap>()));
}

TEST_F(ParserMetadataTest, WriteStringToFileParentNotExist)
{
    ManagedPath output{"my", "test", "output", "BABEADEFEA", "file.txt"};
    std::string outdata = "blah blah nothhing here!";

    ASSERT_FALSE(output.parent_path().is_directory());
    ASSERT_FALSE(pf_.WriteStringToFile(output, outdata));
}

TEST_F(ParserMetadataTest, WriteStringToFile)
{
    ManagedPath output{"parser_metadata_test_file.txt"};
    std::string outdata = "blah blah nothhing here!";

    ASSERT_TRUE(output.parent_path().is_directory());
    ASSERT_TRUE(pf_.WriteStringToFile(output, outdata));

    FileReader fr;
    ASSERT_EQ(0, fr.ReadFile(output.RawString()));

    // Add a newline to expected string since GetDocumentAsString
    // adds a newline to each "line" it reads in.
    EXPECT_EQ(outdata + "\n", fr.GetDocumentAsString());

    ASSERT_TRUE(output.remove());
}

TEST_F(ParserMetadataTest, RecordCh10PktTypeSpecificMetadataUnusedType)
{
    // Not currently implemented for type-specific processing.
    Ch10PacketType pkt_type = Ch10PacketType::CAN_BUS; 
    Ch10Context ctx1;
    Ch10Context ctx2;
    std::vector<const Ch10Context*> ctx_vec{&ctx1, &ctx2};

    MDCategoryMap runtime("runtime");
    TMATSData tmats;
    Ch10PacketTypeSpecificMetadata spec_md;

    ASSERT_TRUE(pf_.RecordCh10PktTypeSpecificMetadata(pkt_type, ctx_vec, &runtime,
        &tmats, &spec_md));
}

TEST_F(ParserMetadataTest, RecordCh10PktTypeSpecificMetadataExec1553F1)
{
    Ch10PacketType pkt_type = Ch10PacketType::MILSTD1553_F1; 
    Ch10Context ctx1;
    Ch10Context ctx2;
    std::vector<const Ch10Context*> ctx_vec{&ctx1, &ctx2};

    MDCategoryMap runtime("runtime");
    TMATSData tmats;
    MockCh10PacketTypeSpecificMetadata spec_md;

    Sequence seq;

    EXPECT_CALL(spec_md, RecordMilStd1553F1SpecificMetadata(ctx_vec, &runtime)).
        InSequence(seq).WillOnce(Return(false));

    EXPECT_CALL(spec_md, RecordMilStd1553F1SpecificMetadata(ctx_vec, &runtime)).
        InSequence(seq).WillOnce(Return(true));

    ASSERT_FALSE(pf_.RecordCh10PktTypeSpecificMetadata(pkt_type, ctx_vec, &runtime,
        &tmats, &spec_md));

    ASSERT_TRUE(pf_.RecordCh10PktTypeSpecificMetadata(pkt_type, ctx_vec, &runtime,
        &tmats, &spec_md));
}

TEST_F(ParserMetadataTest, RecordCh10PktTypeSpecificMetadataExecVideoF0)
{
    Ch10PacketType pkt_type = Ch10PacketType::VIDEO_DATA_F0; 
    Ch10Context ctx1;
    Ch10Context ctx2;
    std::vector<const Ch10Context*> ctx_vec{&ctx1, &ctx2};

    MDCategoryMap runtime("runtime");
    TMATSData tmats;
    MockCh10PacketTypeSpecificMetadata spec_md;

    Sequence seq;

    EXPECT_CALL(spec_md, RecordVideoDataF0SpecificMetadata(ctx_vec, &runtime)).
        InSequence(seq).WillOnce(Return(false));

    EXPECT_CALL(spec_md, RecordVideoDataF0SpecificMetadata(ctx_vec, &runtime)).
        InSequence(seq).WillOnce(Return(true));

    ASSERT_FALSE(pf_.RecordCh10PktTypeSpecificMetadata(pkt_type, ctx_vec, &runtime,
        &tmats, &spec_md));

    ASSERT_TRUE(pf_.RecordCh10PktTypeSpecificMetadata(pkt_type, ctx_vec, &runtime,
        &tmats, &spec_md));
}

TEST_F(ParserMetadataTest, RecordCh10PktTypeSpecificMetadataExecARINC429F0)
{
    Ch10PacketType pkt_type = Ch10PacketType::ARINC429_F0; 
    Ch10Context ctx1;
    Ch10Context ctx2;
    std::vector<const Ch10Context*> ctx_vec{&ctx1, &ctx2};

    MDCategoryMap runtime("runtime");
    TMATSData tmats;
    MockCh10PacketTypeSpecificMetadata spec_md;

    Sequence seq;

    EXPECT_CALL(spec_md, RecordARINC429F0SpecificMetadata(ctx_vec, &runtime, &tmats)).
        InSequence(seq).WillOnce(Return(false));

    EXPECT_CALL(spec_md, RecordARINC429F0SpecificMetadata(ctx_vec, &runtime, &tmats)).
        InSequence(seq).WillOnce(Return(true));

    ASSERT_FALSE(pf_.RecordCh10PktTypeSpecificMetadata(pkt_type, ctx_vec, &runtime,
        &tmats, &spec_md));

    ASSERT_TRUE(pf_.RecordCh10PktTypeSpecificMetadata(pkt_type, ctx_vec, &runtime,
        &tmats, &spec_md));
}

TEST_F(ParserMetadataTest, RecordMetadataForPktTypeFailAtPktTypeSpecificMetadata)
{
    ManagedPath md_filename("my_metadata");
    Ch10PacketType pkt_type = Ch10PacketType::MILSTD1553_F1;
    std::string pkt_type_label = ch10packettype_to_string_map.at(pkt_type);
    ManagedPath mil1553_outpath{"default", "output"};
    std::map<Ch10PacketType, ManagedPath> outdir_map{{pkt_type, mil1553_outpath}};

    Ch10Context ctx1;
    Ch10Context ctx2;
    std::vector<const Ch10Context*> ctx_vec{&ctx1, &ctx2};

    MockTIPMDDocument tip_md;
    ProvenanceData prov_data;
    prov_data.hash = "blahd4309823549089y34t4khwer";
    prov_data.time = "dsf;lfd;jadf;j";
    prov_data.tip_version = "d43283ti9hewg2";
    ParserConfigParams config;
    TMATSData tmats;
    MockParserPaths parser_paths;
    MockParserMetadataFunctions funcs;

    EXPECT_CALL(parser_paths, GetCh10PacketTypeOutputDirMap()).
        WillOnce(ReturnRef(outdir_map));

    ManagedPath ch10_path("test_ch10_path");
    EXPECT_CALL(parser_paths, GetCh10Path()).WillOnce(ReturnRef(ch10_path));

    ManagedPath md_file_path = mil1553_outpath / md_filename;

    EXPECT_CALL(funcs, RecordProvenanceData(&tip_md, ch10_path, pkt_type_label, 
        prov_data));

    std::shared_ptr<MDCategoryMap> config_cat = std::make_shared<MDCategoryMap>("config");
    EXPECT_CALL(tip_md, GetConfigCategory()).WillOnce(Return(config_cat));
    EXPECT_CALL(funcs, RecordUserConfigData(config_cat, config));

    std::shared_ptr<MDCategoryMap> runtime_cat = std::make_shared<MDCategoryMap>("runtime");
    EXPECT_CALL(tip_md, GetRuntimeCategory()).WillOnce(Return(runtime_cat));

    EXPECT_CALL(funcs, RecordCh10PktTypeSpecificMetadata(pkt_type, ctx_vec,
        runtime_cat.get(), &tmats)).WillOnce(Return(false));

    EXPECT_EQ(70, pm_.RecordMetadataForPktType(md_filename, pkt_type, &parser_paths, 
        config, prov_data, &tmats, ctx_vec, &tip_md, &funcs));
}

TEST_F(ParserMetadataTest, RecordMetadataForPktTypeProcessTMATSFail)
{
    ManagedPath md_filename("my_metadata");
    Ch10PacketType pkt_type = Ch10PacketType::MILSTD1553_F1;
    std::string pkt_type_label = ch10packettype_to_string_map.at(pkt_type);
    ManagedPath mil1553_outpath{"default", "output"};
    std::map<Ch10PacketType, ManagedPath> outdir_map{{pkt_type, mil1553_outpath}};

    Ch10Context ctx1;
    Ch10Context ctx2;
    std::vector<const Ch10Context*> ctx_vec{&ctx1, &ctx2};

    MockTIPMDDocument tip_md;
    ProvenanceData prov_data;
    prov_data.hash = "blahd4309823549089y34t4khwer";
    prov_data.time = "dsf;lfd;jadf;j";
    prov_data.tip_version = "d43283ti9hewg2";
    ParserConfigParams config;
    TMATSData tmats;
    MockParserPaths parser_paths;
    MockParserMetadataFunctions funcs;

    EXPECT_CALL(parser_paths, GetCh10PacketTypeOutputDirMap()).
        WillOnce(ReturnRef(outdir_map));

    ManagedPath ch10_path("test_ch10_path");
    EXPECT_CALL(parser_paths, GetCh10Path()).WillOnce(ReturnRef(ch10_path));

    ManagedPath md_file_path = mil1553_outpath / md_filename;

    EXPECT_CALL(funcs, RecordProvenanceData(&tip_md, ch10_path, pkt_type_label, 
        prov_data));

    std::shared_ptr<MDCategoryMap> config_cat = std::make_shared<MDCategoryMap>("config");
    EXPECT_CALL(tip_md, GetConfigCategory()).WillOnce(Return(config_cat));
    EXPECT_CALL(funcs, RecordUserConfigData(config_cat, config));

    std::shared_ptr<MDCategoryMap> runtime_cat = std::make_shared<MDCategoryMap>("runtime");
    EXPECT_CALL(tip_md, GetRuntimeCategory()).WillOnce(Return(runtime_cat));

    EXPECT_CALL(funcs, RecordCh10PktTypeSpecificMetadata(pkt_type, ctx_vec,
        runtime_cat.get(), &tmats)).WillOnce(Return(true));

    EXPECT_CALL(funcs, ProcessTMATSForType(&tmats, &tip_md, pkt_type)).
        WillOnce(Return(false));

    EXPECT_EQ(70, pm_.RecordMetadataForPktType(md_filename, pkt_type, &parser_paths, 
        config, prov_data, &tmats, ctx_vec, &tip_md, &funcs));
}

TEST_F(ParserMetadataTest, RecordMetadataForPktTypeWriteStringToFileFail)
{
    ManagedPath md_filename("my_metadata");
    Ch10PacketType pkt_type = Ch10PacketType::MILSTD1553_F1;
    std::string pkt_type_label = ch10packettype_to_string_map.at(pkt_type);
    ManagedPath mil1553_outpath{"default", "output"};
    std::map<Ch10PacketType, ManagedPath> outdir_map{{pkt_type, mil1553_outpath}};

    Ch10Context ctx1;
    Ch10Context ctx2;
    std::vector<const Ch10Context*> ctx_vec{&ctx1, &ctx2};

    MockTIPMDDocument tip_md;
    ProvenanceData prov_data;
    prov_data.hash = "blahd4309823549089y34t4khwer";
    prov_data.time = "dsf;lfd;jadf;j";
    prov_data.tip_version = "d43283ti9hewg2";
    ParserConfigParams config;
    TMATSData tmats;
    MockParserPaths parser_paths;
    MockParserMetadataFunctions funcs;

    EXPECT_CALL(parser_paths, GetCh10PacketTypeOutputDirMap()).
        WillOnce(ReturnRef(outdir_map));

    ManagedPath ch10_path("test_ch10_path");
    EXPECT_CALL(parser_paths, GetCh10Path()).WillOnce(ReturnRef(ch10_path));

    ManagedPath md_file_path = mil1553_outpath / md_filename;

    EXPECT_CALL(funcs, RecordProvenanceData(&tip_md, ch10_path, pkt_type_label, 
        prov_data));

    std::shared_ptr<MDCategoryMap> config_cat = std::make_shared<MDCategoryMap>("config");
    EXPECT_CALL(tip_md, GetConfigCategory()).WillOnce(Return(config_cat));
    EXPECT_CALL(funcs, RecordUserConfigData(config_cat, config));

    std::shared_ptr<MDCategoryMap> runtime_cat = std::make_shared<MDCategoryMap>("runtime");
    EXPECT_CALL(tip_md, GetRuntimeCategory()).WillOnce(Return(runtime_cat));

    EXPECT_CALL(funcs, RecordCh10PktTypeSpecificMetadata(pkt_type, ctx_vec,
        runtime_cat.get(), &tmats)).WillOnce(Return(true));

    EXPECT_CALL(funcs, ProcessTMATSForType(&tmats, &tip_md, pkt_type)).
        WillOnce(Return(true));

    EXPECT_CALL(tip_md, CreateDocument()).WillOnce(Return(true));
    std::string md_string = "this is the value of the metadata";
    EXPECT_CALL(tip_md, GetMetadataString()).WillOnce(Return(md_string));

    EXPECT_CALL(funcs, WriteStringToFile(md_file_path, md_string)).WillOnce(Return(false));

    EXPECT_EQ(74, pm_.RecordMetadataForPktType(md_filename, pkt_type, &parser_paths, 
        config, prov_data, &tmats, ctx_vec, &tip_md, &funcs));
}

TEST_F(ParserMetadataTest, RecordMetadataForPktType)
{
    ManagedPath md_filename("my_metadata");
    Ch10PacketType pkt_type = Ch10PacketType::MILSTD1553_F1;
    std::string pkt_type_label = ch10packettype_to_string_map.at(pkt_type);
    ManagedPath mil1553_outpath{"default", "output"};
    std::map<Ch10PacketType, ManagedPath> outdir_map{{pkt_type, mil1553_outpath}};

    Ch10Context ctx1;
    Ch10Context ctx2;
    std::vector<const Ch10Context*> ctx_vec{&ctx1, &ctx2};

    MockTIPMDDocument tip_md;
    ProvenanceData prov_data;
    prov_data.hash = "blahd4309823549089y34t4khwer";
    prov_data.time = "dsf;lfd;jadf;j";
    prov_data.tip_version = "d43283ti9hewg2";
    ParserConfigParams config;
    TMATSData tmats;
    MockParserPaths parser_paths;
    MockParserMetadataFunctions funcs;

    EXPECT_CALL(parser_paths, GetCh10PacketTypeOutputDirMap()).
        WillOnce(ReturnRef(outdir_map));

    ManagedPath ch10_path("test_ch10_path");
    EXPECT_CALL(parser_paths, GetCh10Path()).WillOnce(ReturnRef(ch10_path));

    ManagedPath md_file_path = mil1553_outpath / md_filename;

    EXPECT_CALL(funcs, RecordProvenanceData(&tip_md, ch10_path, pkt_type_label, 
        prov_data));

    std::shared_ptr<MDCategoryMap> config_cat = std::make_shared<MDCategoryMap>("config");
    EXPECT_CALL(tip_md, GetConfigCategory()).WillOnce(Return(config_cat));
    EXPECT_CALL(funcs, RecordUserConfigData(config_cat, config));

    std::shared_ptr<MDCategoryMap> runtime_cat = std::make_shared<MDCategoryMap>("runtime");
    EXPECT_CALL(tip_md, GetRuntimeCategory()).WillOnce(Return(runtime_cat));

    EXPECT_CALL(funcs, RecordCh10PktTypeSpecificMetadata(pkt_type, ctx_vec,
        runtime_cat.get(), &tmats)).WillOnce(Return(true));

    EXPECT_CALL(funcs, ProcessTMATSForType(&tmats, &tip_md, pkt_type)).
        WillOnce(Return(true));

    EXPECT_CALL(tip_md, CreateDocument()).WillOnce(Return(true));
    std::string md_string = "this is the value of the metadata";
    EXPECT_CALL(tip_md, GetMetadataString()).WillOnce(Return(md_string));

    EXPECT_CALL(funcs, WriteStringToFile(md_file_path, md_string)).WillOnce(Return(true));

    EXPECT_EQ(0, pm_.RecordMetadataForPktType(md_filename, pkt_type, &parser_paths, 
        config, prov_data, &tmats, ctx_vec, &tip_md, &funcs));
}

TEST_F(ParserMetadataTest, WriteTDPDataInitializeFail)
{
    ManagedPath out_path{"test.parquet"};
    
    // ParquetTDPF1 INitialized
    ParquetContext ctx;
    MockParquetTDPF1 pqtdp(&ctx);
    EXPECT_CALL(pqtdp, Initialize(out_path, 0)).WillOnce(Return(70));

    Ch10Context ctx1(13344545, 0);
    Ch10Context ctx2(848348, 1);
    std::vector<const Ch10Context*> ctx_vec{&ctx1, &ctx2};

    EXPECT_EQ(70, pf_.WriteTDPData(ctx_vec, &pqtdp, out_path));
}

TEST_F(ParserMetadataTest, WriteTDPData)
{
    ManagedPath out_path{"test.parquet"};
    
    // ParquetTDPF1 INitialized
    ParquetContext ctx;
    MockParquetTDPF1 pqtdp(&ctx);
    EXPECT_CALL(pqtdp, Initialize(out_path, 0)).WillOnce(Return(0));

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

    EXPECT_EQ(0, pf_.WriteTDPData(ctx_vec, &pqtdp, out_path));
}

TEST_F(ParserMetadataTest, GatherTMATSData)
{
    MockCh10Context ctx1;
    MockCh10Context ctx2;
    std::vector<const Ch10Context*> ctx_vec{&ctx1, &ctx2};

    std::string tmats_matter1 = "here are some tmats data";
    std::string tmats_matter2 = "some more tmats data yay!";
    Sequence seq;

    EXPECT_CALL(ctx1, GetTMATSMatter()).InSequence(seq).WillOnce(Return(tmats_matter1));
    EXPECT_CALL(ctx2, GetTMATSMatter()).InSequence(seq).WillOnce(Return(tmats_matter2));
    
    std::vector<std::string> tmats_vec;
    pf_.GatherTMATSData(ctx_vec, tmats_vec);

    std::vector<std::string> expected{tmats_matter1, tmats_matter2};
    EXPECT_THAT(expected, ::testing::ContainerEq(tmats_vec));
}

TEST_F(ParserMetadataTest, AssembleParsedPacketTypesSet)
{
    MockCh10Context ctx1;
    MockCh10Context ctx2;
    std::vector<const Ch10Context*> ctx_vec{&ctx1, &ctx2};

    std::set<Ch10PacketType> parsed_types1{Ch10PacketType::MILSTD1553_F1, 
        Ch10PacketType::COMPUTER_GENERATED_DATA_F1};
    std::set<Ch10PacketType> parsed_types2{Ch10PacketType::MILSTD1553_F1, 
        Ch10PacketType::VIDEO_DATA_F0};
    
    EXPECT_CALL(ctx1, GetParsedPacketTypes()).WillOnce(ReturnRef(parsed_types1));
    EXPECT_CALL(ctx2, GetParsedPacketTypes()).WillOnce(ReturnRef(parsed_types2));

    std::set<Ch10PacketType> expected_parsed_packet_types{Ch10PacketType::MILSTD1553_F1, 
        Ch10PacketType::COMPUTER_GENERATED_DATA_F1, Ch10PacketType::VIDEO_DATA_F0};

    std::set<Ch10PacketType> parsed_packet_types;
    pf_.AssembleParsedPacketTypesSet(ctx_vec, parsed_packet_types);

    EXPECT_THAT(expected_parsed_packet_types, ::testing::ContainerEq(parsed_packet_types));
}