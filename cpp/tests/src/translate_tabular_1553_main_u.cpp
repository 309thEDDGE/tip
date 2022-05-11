#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <string>
#include <vector>
#include <ostream>

// Order of includes is important here
#include "translate_tabular_1553_main.h"
#include "argument_validation_mock.h"
#include "file_reader_mock.h"
#include "dts1553_mock.h"
#include "managed_path_mock.h"
#include "tip_md_document_mock.h"
#include "managed_path.h"

using namespace transtab1553;

using ::testing::Return;
using ::testing::Exactly;
using ::testing::_;
using ::testing::NiceMock;

class TranslateTabular1553MainTest : public ::testing::Test
{
protected:
    NiceMock<MockArgumentValidation> mock_av_;
    NiceMock<MockFileReader> mock_fr_;
    NiceMock<MockDTS1553> mock_dts1553_;
    NiceMock<MockManagedPath> mock_managed_path_;
    NiceMock<MockTIPMDDocument> mock_tip_doc_;
    std::string str_input_path_;
    std::string str_icd_path_;
    std::string str_out_path_;
    std::string str_conf_path_;
    std::string str_log_path_;
    ManagedPath input_path_;
    ManagedPath icd_path_;
    ManagedPath out_path_;
    ManagedPath conf_path_;
    ManagedPath conf_schema_path_;
    ManagedPath icd_schema_path_;
    ManagedPath log_path_;
    
    ManagedPath temp_conf_base_path_;
    ManagedPath temp_conf_path_;
    ManagedPath temp_schemas_path_;

public:
    // Make the default output path (out_path_) equal to a real path. In this case, simply
    // set it to the cwd. This will allow the line in ValidatePaths, 
    // "ManagedPath default_output_dir = input_path.parent_path();", to avoid failure when
    // parent_path() is called. 
    TranslateTabular1553MainTest() : mock_av_(), str_input_path_(""), str_out_path_(""), str_conf_path_(""),
        str_log_path_(""), input_path_(), out_path_(str_out_path_), 
        conf_path_(), log_path_(str_log_path_), conf_schema_path_(""), str_icd_path_(""),
        icd_path_(str_icd_path_), icd_schema_path_(), temp_conf_base_path_({"temp_conf"}), temp_conf_path_(),
        temp_schemas_path_(), mock_fr_(), mock_dts1553_(), mock_managed_path_(), mock_tip_doc_()
    {}

};

TEST_F(TranslateTabular1553MainTest, ValidatePathsCheckExtensionParsedDataFail)
{
    std::vector<std::string> exts{"parquet"};
    EXPECT_CALL(mock_av_, CheckExtension(str_input_path_, exts)).WillOnce(Return(false));

    ASSERT_FALSE(ValidatePaths(str_input_path_, str_icd_path_, str_out_path_, 
        str_log_path_, input_path_, icd_path_, out_path_, log_path_, &mock_av_));
}

TEST_F(TranslateTabular1553MainTest, ValidatePathsValidateDirectoryPathFail)
{
    std::vector<std::string> exts{"parquet"};
    EXPECT_CALL(mock_av_, CheckExtension(str_input_path_, exts)).WillOnce(Return(true));
    EXPECT_CALL(mock_av_, ValidateDirectoryPath(str_input_path_, input_path_)).
        WillOnce(Return(false));

    ASSERT_FALSE(ValidatePaths(str_input_path_, str_icd_path_, str_out_path_, 
        str_log_path_, input_path_, icd_path_, out_path_, log_path_, &mock_av_));
}

TEST_F(TranslateTabular1553MainTest, ValidatePathsCheckExtensionICDFail)
{
    std::vector<std::string> exts{"parquet"};
    EXPECT_CALL(mock_av_, CheckExtension(str_input_path_, exts)).WillOnce(Return(true));
    EXPECT_CALL(mock_av_, ValidateDirectoryPath(str_input_path_, input_path_)).
        WillOnce(Return(true));

    std::vector<std::string> icd_exts{"txt", "csv", "yaml", "yml"};
    EXPECT_CALL(mock_av_, CheckExtension(str_input_path_, icd_exts)).WillOnce(Return(false));

    ASSERT_FALSE(ValidatePaths(str_input_path_, str_icd_path_, str_out_path_, 
        str_log_path_, input_path_, icd_path_, out_path_, log_path_, &mock_av_));
}

TEST_F(TranslateTabular1553MainTest, ValidatePathsValidateInputFilePathICDFail)
{
    std::vector<std::string> exts{"parquet"};
    EXPECT_CALL(mock_av_, CheckExtension(str_input_path_, exts)).WillOnce(Return(true));
    EXPECT_CALL(mock_av_, ValidateDirectoryPath(str_input_path_, input_path_)).
        WillOnce(Return(true));

    std::vector<std::string> icd_exts{"txt", "csv", "yaml", "yml"};
    EXPECT_CALL(mock_av_, CheckExtension(str_input_path_, icd_exts)).WillOnce(Return(true));
    EXPECT_CALL(mock_av_, ValidateInputFilePath(str_icd_path_, icd_path_)).WillOnce(Return(false));

    ASSERT_FALSE(ValidatePaths(str_input_path_, str_icd_path_, str_out_path_, 
        str_log_path_, input_path_, icd_path_, out_path_, log_path_, &mock_av_));
}

TEST_F(TranslateTabular1553MainTest, ValidatePathsValidateOutDirectoryFail)
{
    std::vector<std::string> exts{"parquet"};
    ::testing::Sequence seq;
    EXPECT_CALL(mock_av_, CheckExtension(str_input_path_, exts)).
        InSequence(seq).WillOnce(Return(true));
    EXPECT_CALL(mock_av_, ValidateDirectoryPath(str_input_path_, input_path_)).
        InSequence(seq).WillOnce(Return(true));

    std::vector<std::string> icd_exts{"txt", "csv", "yaml", "yml"};
    EXPECT_CALL(mock_av_, CheckExtension(str_input_path_, icd_exts)).
        InSequence(seq).WillOnce(Return(true));
    EXPECT_CALL(mock_av_, ValidateInputFilePath(str_icd_path_, icd_path_)).
        InSequence(seq).WillOnce(Return(true));

    EXPECT_CALL(mock_av_, ValidateDirectoryPath(str_out_path_, out_path_)).
        InSequence(seq).WillOnce(Return(false));

    ASSERT_FALSE(ValidatePaths(str_input_path_, str_icd_path_, str_out_path_, 
        str_log_path_, input_path_, icd_path_, out_path_, log_path_, &mock_av_));
}

TEST_F(TranslateTabular1553MainTest, ValidatePathsValidateLogDirectoryFail)
{
    std::vector<std::string> exts{"parquet"};
    ::testing::Sequence seq;
    EXPECT_CALL(mock_av_, CheckExtension(str_input_path_, exts)).
        InSequence(seq).WillOnce(Return(true));
    EXPECT_CALL(mock_av_, ValidateDirectoryPath(str_input_path_, input_path_)).
        InSequence(seq).WillOnce(Return(true));

    std::vector<std::string> icd_exts{"txt", "csv", "yaml", "yml"};
    EXPECT_CALL(mock_av_, CheckExtension(str_input_path_, icd_exts)).
        InSequence(seq).WillOnce(Return(true));
    EXPECT_CALL(mock_av_, ValidateInputFilePath(str_icd_path_, icd_path_)).
        InSequence(seq).WillOnce(Return(true));

    EXPECT_CALL(mock_av_, ValidateDirectoryPath(str_out_path_, out_path_)).
        InSequence(seq).WillOnce(Return(true));

    EXPECT_CALL(mock_av_, ValidateDirectoryPath(str_log_path_, log_path_)).
        InSequence(seq).WillOnce(Return(false));

    ASSERT_FALSE(ValidatePaths(str_input_path_, str_icd_path_, str_out_path_, 
        str_log_path_, input_path_, icd_path_, out_path_, log_path_, &mock_av_));
}

TEST_F(TranslateTabular1553MainTest, ValidatePathsSucceed)
{
    std::vector<std::string> exts{"parquet"};
    ::testing::Sequence seq;
    EXPECT_CALL(mock_av_, CheckExtension(str_input_path_, exts)).
        InSequence(seq).WillOnce(Return(true));
    EXPECT_CALL(mock_av_, ValidateDirectoryPath(str_input_path_, input_path_)).
        InSequence(seq).WillOnce(Return(true));

    std::vector<std::string> icd_exts{"txt", "csv", "yaml", "yml"};
    EXPECT_CALL(mock_av_, CheckExtension(str_input_path_, icd_exts)).
        InSequence(seq).WillOnce(Return(true));
    EXPECT_CALL(mock_av_, ValidateInputFilePath(str_icd_path_, icd_path_)).
        InSequence(seq).WillOnce(Return(true));

    EXPECT_CALL(mock_av_, ValidateDirectoryPath(str_out_path_, out_path_)).
        InSequence(seq).WillOnce(Return(true));

    EXPECT_CALL(mock_av_, ValidateDirectoryPath(str_log_path_, log_path_)).
        InSequence(seq).WillOnce(Return(true));

    ASSERT_TRUE(ValidatePaths(str_input_path_, str_icd_path_, str_out_path_, 
        str_log_path_, input_path_, icd_path_, out_path_, log_path_, &mock_av_));
}

TEST_F(TranslateTabular1553MainTest, IngestICDReadFileFail)
{
    EXPECT_CALL(mock_fr_, ReadFile(icd_path_.string())).WillOnce(Return(1));

    std::map<std::string, std::string> msg_name_subs;
    std::map<std::string, std::string> elem_name_subs;
    ASSERT_FALSE(IngestICD(&mock_dts1553_, icd_path_, msg_name_subs, 
        elem_name_subs, &mock_fr_));
}

TEST_F(TranslateTabular1553MainTest, IngestICDIngestLinesFail)
{
    std::map<std::string, std::string> msg_name_subs;
    std::map<std::string, std::string> elem_name_subs;

    EXPECT_CALL(mock_fr_, ReadFile(icd_path_.string())).WillOnce(Return(0));

    std::vector<std::string> get_lines;
    EXPECT_CALL(mock_fr_, GetLines()).WillOnce(Return(get_lines));

    EXPECT_CALL(mock_dts1553_, IngestLines(icd_path_, get_lines, msg_name_subs, elem_name_subs)).
        WillOnce(Return(false));

    ASSERT_FALSE(IngestICD(&mock_dts1553_, icd_path_, msg_name_subs, 
        elem_name_subs, &mock_fr_));
}

TEST_F(TranslateTabular1553MainTest, IngestICDSucceed)
{
    std::map<std::string, std::string> msg_name_subs;
    std::map<std::string, std::string> elem_name_subs;

    EXPECT_CALL(mock_fr_, ReadFile(icd_path_.string())).WillOnce(Return(0));

    std::vector<std::string> get_lines;
    EXPECT_CALL(mock_fr_, GetLines()).WillOnce(Return(get_lines));

    EXPECT_CALL(mock_dts1553_, IngestLines(icd_path_, get_lines, msg_name_subs, elem_name_subs)).
        WillOnce(Return(true));

    ASSERT_TRUE(IngestICD(&mock_dts1553_, icd_path_, msg_name_subs, 
        elem_name_subs, &mock_fr_));
}

TEST_F(TranslateTabular1553MainTest, GetParsed1553MetadataFileNotExist)
{
    EXPECT_CALL(mock_managed_path_, is_regular_file()).WillOnce(Return(false));

    ASSERT_FALSE(GetParsed1553Metadata(&mock_managed_path_, &mock_tip_doc_, &mock_fr_));
}

TEST_F(TranslateTabular1553MainTest, GetParsed1553MetadataReadMDFail)
{
    EXPECT_CALL(mock_managed_path_, is_regular_file()).WillOnce(Return(true));
    EXPECT_CALL(mock_fr_, ReadFile(mock_managed_path_.string())).WillOnce(Return(false));

    ASSERT_FALSE(GetParsed1553Metadata(&mock_managed_path_, &mock_tip_doc_, &mock_fr_));
}

TEST_F(TranslateTabular1553MainTest, GetParsed1553MetadataReadDocumentFail)
{
    EXPECT_CALL(mock_managed_path_, is_regular_file()).WillOnce(Return(true));
    EXPECT_CALL(mock_fr_, ReadFile(mock_managed_path_.string())).WillOnce(Return(false));

    std::string doc;
    EXPECT_CALL(mock_fr_, GetDocumentAsString()).WillOnce(Return(doc));
    EXPECT_CALL(mock_tip_doc_, ReadDocument(doc)).WillOnce(Return(false));

    ASSERT_FALSE(GetParsed1553Metadata(&mock_managed_path_, &mock_tip_doc_, &mock_fr_));
}

TEST_F(TranslateTabular1553MainTest, GetParsed1553MetadataParsedMDIncorrectType)
{
    EXPECT_CALL(mock_managed_path_, is_regular_file()).WillOnce(Return(true));
    EXPECT_CALL(mock_fr_, ReadFile(mock_managed_path_.string())).WillOnce(Return(false));

    std::string doc;
    EXPECT_CALL(mock_fr_, GetDocumentAsString()).WillOnce(Return(doc));
    EXPECT_CALL(mock_tip_doc_, ReadDocument(doc)).WillOnce(Return(true));

    ASSERT_FALSE(GetParsed1553Metadata(&mock_managed_path_, &mock_tip_doc_, &mock_fr_));
}

TEST_F(TranslateTabular1553MainTest, GetParsed1553MetadataParsedMDCorrectType)
{
    std::string cat_name = "parsed_" + ch10packettype_to_string_map.at(Ch10PacketType::MILSTD1553_F1);
    mock_tip_doc_.type_category_->SetScalarValue(cat_name);
    EXPECT_CALL(mock_managed_path_, is_regular_file()).WillOnce(Return(true));
    EXPECT_CALL(mock_fr_, ReadFile(mock_managed_path_.string())).WillOnce(Return(false));

    std::string doc;
    EXPECT_CALL(mock_fr_, GetDocumentAsString()).WillOnce(Return(doc));
    EXPECT_CALL(mock_tip_doc_, ReadDocument(doc)).WillOnce(Return(true));

    ASSERT_TRUE(GetParsed1553Metadata(&mock_managed_path_, &mock_tip_doc_, &mock_fr_));
}