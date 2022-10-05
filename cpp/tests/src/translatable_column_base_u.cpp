#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "translatable_column_base.h"

class TranslatableColumnBaseTest : public ::testing::Test
{
   protected:
    TranslatableColumnBase col_;
    std::string name_;
    std::shared_ptr<arrow::DataType> arrow_type_;
    size_t words_per_value_;
    size_t payload_offset_;
    bool result_;
    ICDElement icd_elem_;
    size_t thread_index_;

   public:
    TranslatableColumnBaseTest() : col_(), name_(""), arrow_type_(nullptr), result_(false), words_per_value_(0), payload_offset_(0), icd_elem_(), thread_index_(0)
    {
    }
};

TEST_F(TranslatableColumnBaseTest, ConfigureInvalid)
{
    // name_ is an empty string.
    arrow_type_ = arrow::float32();
    words_per_value_ = 2;
    result_ = col_.Configure(name_, true, arrow_type_, icd_elem_, thread_index_);
    EXPECT_FALSE(result_);

    // arrow_type_ is null
    arrow_type_ = nullptr;
    name_ = "col1";
    result_ = col_.Configure(name_, true, arrow_type_, icd_elem_, thread_index_);
    EXPECT_FALSE(result_);

    // words_per_value_ = 0
    arrow_type_ = arrow::int16();
    icd_elem_.elem_word_count_ = 0;
    result_ = col_.Configure(name_, true, arrow_type_, icd_elem_, thread_index_);
    EXPECT_FALSE(result_);
}

TEST_F(TranslatableColumnBaseTest, Configure)
{
    arrow_type_ = arrow::float32();
    name_ = "col1";
    icd_elem_.elem_word_count_ = 4;
    bool is_ridealong = true;
    result_ = col_.Configure(name_, is_ridealong, arrow_type_, icd_elem_, thread_index_);
    EXPECT_TRUE(result_);

    EXPECT_EQ(name_, col_.col_name);
    EXPECT_EQ(is_ridealong, col_.is_ridealong);
    EXPECT_EQ(icd_elem_.elem_word_count_, col_.words_per_translated_value);
    EXPECT_EQ(thread_index_, col_.thread_index);
}