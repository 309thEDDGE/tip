#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <string>
#include <vector>

// Order of includes is important here
#include "ch10_parse_main.h"
#include "argument_validation_mock.h"
#include "managed_path.h"
#include "parse_manager_mock.h"
#include "parser_config_params.h"

using ::testing::Return;
using ::testing::Exactly;
using ::testing::_;
using ::testing::NiceMock;

class Ch10ParseMainTest : public ::testing::Test
{
protected:
    NiceMock<MockArgumentValidation> mock_av_;
    NiceMock<MockParseManager> mock_pm_;
    std::string str_input_path_;
    std::string str_out_path_;
    std::string str_conf_path_;
    std::string str_log_path_;
    ManagedPath input_path_;
    ManagedPath out_path_;
    ManagedPath conf_path_;
    ManagedPath schema_path_;
    ManagedPath log_path_;

public:
    // Make the default output path (out_path_) equal to a real path. In this case, simply
    // set it to the cwd. This will allow the line in ValidatePaths, 
    // "output_path = input_path.absolute().parent_path();", to avoid failure when
    // absolute() is called. 
    Ch10ParseMainTest() : mock_av_(), str_input_path_(""), str_out_path_(""), str_conf_path_(""),
        str_log_path_(""), input_path_(), out_path_(str_out_path_), 
        conf_path_(str_conf_path_), log_path_(str_log_path_), schema_path_(""), mock_pm_()
    {}

};

TEST_F(Ch10ParseMainTest, ValidatePathsCheckExtensionFail)
{
    std::vector<std::string> exts{"ch10", "c10"};
    EXPECT_CALL(mock_av_, CheckExtension(str_input_path_, _)).WillOnce(Return(false));

    ASSERT_EQ(65, ValidatePaths(str_input_path_, str_out_path_, str_log_path_, 
        input_path_, out_path_, log_path_, &mock_av_));
}

TEST_F(Ch10ParseMainTest, ValidatePathsValidateInputFilePathFail)
{
    std::vector<std::string> exts{"ch10", "c10"};
    EXPECT_CALL(mock_av_, CheckExtension(str_input_path_, _)).WillOnce(Return(true));
    EXPECT_CALL(mock_av_, ValidateInputFilePath(str_input_path_, input_path_)).WillOnce(Return(false));

    ASSERT_EQ(66, ValidatePaths(str_input_path_, str_out_path_, str_log_path_, 
        input_path_, out_path_, log_path_, &mock_av_));
}

TEST_F(Ch10ParseMainTest, ValidatePathsValidateDirectoryPathFail)
{
    // Must not be empty string
    str_out_path_ = "blahblah";

    std::vector<std::string> exts{"ch10", "c10"};
    EXPECT_CALL(mock_av_, CheckExtension(str_input_path_, _)).WillOnce(Return(true));
    EXPECT_CALL(mock_av_, ValidateInputFilePath(str_input_path_, input_path_)).WillOnce(Return(true));
    EXPECT_CALL(mock_av_, ValidateDirectoryPath(str_out_path_, out_path_)).WillOnce(Return(false));

    ASSERT_EQ(69, ValidatePaths(str_input_path_, str_out_path_, str_log_path_, 
        input_path_, out_path_, log_path_, &mock_av_));
}

TEST_F(Ch10ParseMainTest, ValidatePathsValidateDefaultOutputDirectoryFalse)
{
    std::vector<std::string> exts{"ch10", "c10"};
    EXPECT_CALL(mock_av_, CheckExtension(str_input_path_, _)).WillOnce(Return(true));
    EXPECT_CALL(mock_av_, ValidateInputFilePath(str_input_path_, input_path_)).WillOnce(Return(true));

    EXPECT_CALL(mock_av_, ValidateDirectoryPath(str_log_path_, log_path_)).WillOnce(Return(false));

    ASSERT_EQ(69, ValidatePaths(str_input_path_, str_out_path_, str_log_path_, 
        input_path_, out_path_, log_path_, &mock_av_));
}

TEST_F(Ch10ParseMainTest, ValidatePathsTrue)
{
    std::vector<std::string> exts{"ch10", "c10"};
    EXPECT_CALL(mock_av_, CheckExtension(str_input_path_, _)).WillOnce(Return(true));
    EXPECT_CALL(mock_av_, ValidateInputFilePath(str_input_path_, input_path_)).WillOnce(Return(true));

    EXPECT_CALL(mock_av_, ValidateDirectoryPath(str_log_path_, log_path_)).WillOnce(Return(true));

    ASSERT_EQ(0, ValidatePaths(str_input_path_, str_out_path_, str_log_path_, 
        input_path_, out_path_, log_path_, &mock_av_));
}
