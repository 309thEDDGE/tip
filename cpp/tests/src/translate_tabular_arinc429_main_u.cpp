#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <string>
#include <vector>
#include <ostream>
#include <unordered_map>

// Order of includes is important here
#include "translate_tabular_arinc429_main.h"
#include "argument_validation_mock.h"
#include "file_reader_mock.h"
// #include "dts1553_mock.h"
#include "dts429_mock.h"
#include "arinc429_data_mock.h"
#include "organize_429_icd_mock.h"
#include "managed_path_mock.h"
#include "tip_md_document_mock.h"
#include "managed_path.h"

#include "yaml-cpp/yaml.h"
#include "icd_element.h"


using namespace transtab429;

using ::testing::Return;
using ::testing::Exactly;
using ::testing::_;
using ::testing::NiceMock;

class TranslateTabularARINC429MainTest : public ::testing::Test
{
  protected:
    NiceMock<MockArgumentValidation> mock_av_;
    NiceMock<MockFileReader> mock_fr_;
    NiceMock<MockDTS429> mock_dts429_;
    NiceMock<MockOrganize429ICD> mock_org_429_;
    NiceMock<MockARINC429Data> mock_data_429_;
    NiceMock<MockARINC429Data>* mock_data_429_ptr_;
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

    std::vector<std::string> icd_lines_;
    size_t arinc_message_count_;
    YAML::Node parser_md_node_;
    std::vector<std::string> subchan_name_lookup_misses_;

    std::unordered_map<std::string, std::vector<ICDElement>> word_elements_;
    std::unordered_map<uint16_t,std::unordered_map<uint16_t, std::unordered_map<
                        uint16_t,std::unordered_map<int8_t, size_t>>>> organized_lookup_map_;
    std::vector<std::vector<std::vector<ICDElement>>> element_table_;

  public:
    // Make the default output path (out_path_) equal to a real path. In this case, simply
    // set it to the cwd. This will allow the line in ValidatePaths,
    // "ManagedPath default_output_dir = input_path.parent_path();", to avoid failure when
    // parent_path() is called.
    TranslateTabularARINC429MainTest() : mock_av_(), str_input_path_(""), str_out_path_(""), str_conf_path_(""),
        str_log_path_(""), input_path_(), out_path_(str_out_path_),
        conf_path_(), log_path_(str_log_path_), conf_schema_path_(""), str_icd_path_(""),
        icd_path_(str_icd_path_), icd_schema_path_(), temp_conf_base_path_({"temp_conf"}), temp_conf_path_(),
        temp_schemas_path_(), mock_fr_(),  mock_managed_path_(), mock_tip_doc_(), mock_dts429_(), mock_org_429_(),
        mock_data_429_(), mock_data_429_ptr_(&mock_data_429_)
    {}

};

TEST_F(TranslateTabularARINC429MainTest, ValidatePathsCheckExtensionParsedDataFail)
{
    std::vector<std::string> exts{"parquet"};
    EXPECT_CALL(mock_av_, CheckExtension(str_input_path_, exts)).WillOnce(Return(false));

    ASSERT_FALSE(ValidatePaths(str_input_path_, str_icd_path_, str_out_path_, 
        str_log_path_, input_path_, icd_path_, out_path_, log_path_, &mock_av_));
}

TEST_F(TranslateTabularARINC429MainTest, ValidatePathsValidateDirectoryPathFail)
{
    std::vector<std::string> exts{"parquet"};
    EXPECT_CALL(mock_av_, CheckExtension(str_input_path_, exts)).WillOnce(Return(true));
    EXPECT_CALL(mock_av_, ValidateDirectoryPath(str_input_path_, input_path_)).
        WillOnce(Return(false));

    ASSERT_FALSE(ValidatePaths(str_input_path_, str_icd_path_, str_out_path_, 
        str_log_path_, input_path_, icd_path_, out_path_, log_path_, &mock_av_));
}

TEST_F(TranslateTabularARINC429MainTest, ValidatePathsCheckExtensionICDFail)
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

TEST_F(TranslateTabularARINC429MainTest, ValidatePathsValidateInputFilePathICDFail)
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

TEST_F(TranslateTabularARINC429MainTest, ValidatePathsValidateDefaultOutputDirectoryFail)
{
    std::vector<std::string> exts{"parquet"};
    EXPECT_CALL(mock_av_, CheckExtension(str_input_path_, exts)).WillOnce(Return(true));
    EXPECT_CALL(mock_av_, ValidateDirectoryPath(str_input_path_, input_path_)).
        WillOnce(Return(true));

    std::vector<std::string> icd_exts{"txt", "csv", "yaml", "yml"};
    EXPECT_CALL(mock_av_, CheckExtension(str_input_path_, icd_exts)).WillOnce(Return(true));
    EXPECT_CALL(mock_av_, ValidateInputFilePath(str_icd_path_, icd_path_)).WillOnce(Return(true));

    EXPECT_CALL(mock_av_, ValidateDirectoryPath(str_out_path_, out_path_)).WillOnce(Return(false));

    ASSERT_FALSE(ValidatePaths(str_input_path_, str_icd_path_, str_out_path_, 
        str_log_path_, input_path_, icd_path_, out_path_, log_path_, &mock_av_));
}

TEST_F(TranslateTabularARINC429MainTest, ValidatePathsValidateDefaultInputFilePathConfFail)
{
    ::testing::Sequence seq;
    std::vector<std::string> exts{"parquet"};
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

TEST_F(TranslateTabularARINC429MainTest, ValidatePathsSucceed)
{
    ::testing::Sequence seq;
    std::vector<std::string> exts{"parquet"};
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

TEST_F(TranslateTabularARINC429MainTest, IngestICDIngestLinesFail)
{
    // Note - using arguments with ICDElement in mocks causing unexplained crash
    //        that occurs outside the code written for TIP
    EXPECT_CALL(mock_dts429_, IngestLines(icd_lines_, _)).WillOnce(Return(false));

    ASSERT_FALSE(IngestICD(&mock_dts429_, &mock_org_429_, mock_data_429_,
        icd_lines_, arinc_message_count_, parser_md_node_, subchan_name_lookup_misses_));

}


TEST_F(TranslateTabularARINC429MainTest, IngestICDOrganizeICDMapFail)
{
    // Note - using arguments with ICDElement in mocks causing unexplained crash
    //        that occurs outside the code written for TIP

    // for dts429 arguments
    EXPECT_CALL(mock_dts429_, IngestLines(icd_lines_, _)).WillOnce(Return(true));

    // for organize_429_icd arguments
    EXPECT_CALL(mock_org_429_, OrganizeICDMap(_,_,organized_lookup_map_,_)).WillOnce(Return(false));

    ASSERT_FALSE(IngestICD(&mock_dts429_, &mock_org_429_, mock_data_429_,
        icd_lines_, arinc_message_count_, parser_md_node_, subchan_name_lookup_misses_));

}

TEST_F(TranslateTabularARINC429MainTest, IngestICDSuccess)
{
    // Note - using arguments with ICDElement in mocks causing unexplained crash
    //        that occurs outside the code written for TIP

    // for dts429 arguments
    EXPECT_CALL(mock_dts429_, IngestLines(icd_lines_, _)).WillOnce(Return(true));

    // for organize_429_icd arguments
    EXPECT_CALL(mock_org_429_, OrganizeICDMap(_,_,organized_lookup_map_,_)).WillOnce(Return(true));

    ASSERT_TRUE(IngestICD(&mock_dts429_, &mock_org_429_, mock_data_429_,
        icd_lines_, arinc_message_count_, parser_md_node_, subchan_name_lookup_misses_));

}