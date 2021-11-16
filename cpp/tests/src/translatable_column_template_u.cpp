#include <random>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "translatable_column_template.h"

class TranslatableColumnTemplateTest : public ::testing::Test
{
   protected:
    std::string name_;
    std::shared_ptr<arrow::DataType> arrow_type_;
    size_t words_per_value_;
    bool result_;
    size_t row_group_size_;
    size_t payload_offset_;
    uint8_t* data_;
    ICDElement icd_elem_;
    size_t thread_index_;

   public:
    TranslatableColumnTemplateTest() : name_(""), arrow_type_(nullptr), result_(false), words_per_value_(0), row_group_size_(1000), payload_offset_(0), icd_elem_(), thread_index_(0)
    {
        icd_elem_.elem_word_count_ = words_per_value_;
        icd_elem_.offset_ = payload_offset_;
    }

    template <typename RawType>
    std::vector<RawType> CreateRawDataVector(size_t element_count);
};

template <typename RawType>
std::vector<RawType> TranslatableColumnTemplateTest::CreateRawDataVector(size_t element_count)
{
    std::uniform_real_distribution<double> dist(0., 100.);
    unsigned seed = 10;
    std::default_random_engine generator(seed);
    std::vector<RawType> vec(element_count);
    for (size_t i = 0; i < element_count; i++)
    {
        vec[i] = static_cast<RawType>(dist(generator));
    }
    return vec;
}

TEST_F(TranslatableColumnTemplateTest, ConfigureBaseConfigureInvalid)
{
    // name_ is an empty string.
    arrow_type_ = arrow::float32();
    icd_elem_.elem_word_count_ = 2;
    TranslatableColumnTemplate<uint16_t, int32_t> col;
    result_ = col.Configure(name_, true, arrow_type_, row_group_size_, icd_elem_, thread_index_);
    EXPECT_FALSE(result_);
}

TEST_F(TranslatableColumnTemplateTest, ConfigureAllocateVectorsNonRidealong)
{
    arrow_type_ = arrow::float32();
    name_ = "col1";
    icd_elem_.elem_word_count_ = 4;
    bool ridealong = false;
    TranslatableColumnTemplate<uint16_t, int32_t> col;
    result_ = col.Configure(name_, ridealong, arrow_type_, row_group_size_, icd_elem_, thread_index_);
    EXPECT_TRUE(result_);

    size_t raw_data_vec_size = icd_elem_.elem_word_count_ * row_group_size_;
    EXPECT_EQ(raw_data_vec_size, col.GetRawDataVectorSize());
    EXPECT_EQ(row_group_size_, col.GetTranslatedDataVectorSize());
}

TEST_F(TranslatableColumnTemplateTest, ConfigureAllocateVectorsRidealong)
{
    arrow_type_ = arrow::float32();
    name_ = "col1";
    icd_elem_.elem_word_count_ = 4;
    bool ridealong = true;
    TranslatableColumnTemplate<uint16_t, int32_t> col;
    result_ = col.Configure(name_, ridealong, arrow_type_, row_group_size_, icd_elem_, thread_index_);
    EXPECT_TRUE(result_);

    size_t raw_data_vec_size = 0;
    EXPECT_EQ(raw_data_vec_size, col.GetRawDataVectorSize());
    EXPECT_EQ(row_group_size_, col.GetTranslatedDataVectorSize());
}

TEST_F(TranslatableColumnTemplateTest, AppendRawDataRidealongNotValid)
{
    // Configure ridealong column
    arrow_type_ = arrow::float32();
    name_ = "col1";
    bool ridealong = true;
    TranslatableColumnTemplate<uint16_t, int32_t> col;
    icd_elem_.elem_word_count_ = 1;
    result_ = col.Configure(name_, ridealong, arrow_type_, row_group_size_, icd_elem_, thread_index_);
    EXPECT_TRUE(result_);

    size_t count = 10;
    result_ = col.AppendRawData(data_, count);
    EXPECT_FALSE(result_);
}

TEST_F(TranslatableColumnTemplateTest, AppendRawDataInsufficientElementCountOffset0)
{
    // Configure ridealong column
    arrow_type_ = arrow::float32();
    name_ = "col1";
    icd_elem_.elem_word_count_ = 2;
    bool ridealong = false;
    icd_elem_.offset_ = 0;
    TranslatableColumnTemplate<uint16_t, int32_t> col;
    result_ = col.Configure(name_, ridealong, arrow_type_, row_group_size_, icd_elem_, thread_index_);
    EXPECT_TRUE(result_);

    // words_per_value > count
    size_t count = 1;
    result_ = col.AppendRawData(data_, count);
    EXPECT_FALSE(result_);
}

TEST_F(TranslatableColumnTemplateTest, AppendRawDataInsufficientElementCountNonZeroOffset)
{
    // Configure ridealong column
    arrow_type_ = arrow::float32();
    name_ = "col1";
    icd_elem_.elem_word_count_ = 2;
    bool ridealong = false;
    icd_elem_.offset_ = 3;
    TranslatableColumnTemplate<uint16_t, int32_t> col;
    result_ = col.Configure(name_, ridealong, arrow_type_, row_group_size_, icd_elem_, thread_index_);
    EXPECT_TRUE(result_);

    // offset_ + words_per_value_ > count
    size_t count = 4;
    result_ = col.AppendRawData(data_, count);
    EXPECT_FALSE(result_);
}

TEST_F(TranslatableColumnTemplateTest, AppendRawDataNotConfigured)
{
    // Configure not called. raw_data_ vector will have size zero.
    size_t count = 4;
    TranslatableColumnTemplate<uint16_t, int32_t> col;
    result_ = col.AppendRawData(data_, count);
    EXPECT_FALSE(result_);
}

TEST_F(TranslatableColumnTemplateTest, AppendRawDataInsufficientDestVectorSize)
{
    // Configure ridealong column
    arrow_type_ = arrow::float32();
    name_ = "col1";
    icd_elem_.elem_word_count_ = 2;
    bool ridealong = false;
    icd_elem_.offset_ = 3;
    row_group_size_ = 0;  // allocated vector too small given append count and words_per_value
    TranslatableColumnTemplate<uint16_t, int32_t> col;
    result_ = col.Configure(name_, ridealong, arrow_type_, row_group_size_, icd_elem_, thread_index_);
    EXPECT_TRUE(result_);

    size_t count = 40;
    result_ = col.AppendRawData(data_, count);
    EXPECT_FALSE(result_);
}

TEST_F(TranslatableColumnTemplateTest, AppendRawData)
{
    // Configure ridealong column
    arrow_type_ = arrow::float32();
    name_ = "col1";
    icd_elem_.elem_word_count_ = 2;
    bool ridealong = false;
    icd_elem_.offset_ = 3;
    TranslatableColumnTemplate<uint16_t, int32_t> col;
    result_ = col.Configure(name_, ridealong, arrow_type_, row_group_size_, icd_elem_, thread_index_);
    EXPECT_TRUE(result_);

    size_t count = 6;
    std::vector<uint16_t> raw_data1 = CreateRawDataVector<uint16_t>(count);
    data_ = (uint8_t*)raw_data1.data();
    result_ = col.AppendRawData(data_, count);
    EXPECT_TRUE(result_);
    EXPECT_EQ(col.raw_data.at(0), raw_data1[3]);
    EXPECT_EQ(col.raw_data.at(1), raw_data1[4]);

    std::vector<uint16_t> raw_data2 = CreateRawDataVector<uint16_t>(count);
    data_ = (uint8_t*)raw_data2.data();
    result_ = col.AppendRawData(data_, count);
    EXPECT_TRUE(result_);
    EXPECT_EQ(col.raw_data.at(2), raw_data2[3]);
    EXPECT_EQ(col.raw_data.at(3), raw_data2[4]);
}

TEST_F(TranslatableColumnTemplateTest, AppendRidealongDataNonRidealongNotValid)
{
    // Configure ridealong column
    arrow_type_ = arrow::float32();
    name_ = "col1";
    icd_elem_.elem_word_count_ = 4;
    bool ridealong = false;
    TranslatableColumnTemplate<uint16_t, int32_t> col;
    result_ = col.Configure(name_, ridealong, arrow_type_, row_group_size_, icd_elem_, thread_index_);
    EXPECT_TRUE(result_);

    size_t count = 10;
    result_ = col.AppendRidealongData(data_, count);
    EXPECT_FALSE(result_);
}

TEST_F(TranslatableColumnTemplateTest, AppendRidealongDataNotConfigured)
{
    // Configure not called. raw_data_ vector will have size zero.
    size_t count = 4;
    bool ridealong = true;
    TranslatableColumnTemplate<uint16_t, int32_t> col;
    result_ = col.AppendRidealongData(data_, count);
    EXPECT_FALSE(result_);
}

TEST_F(TranslatableColumnTemplateTest, AppendRidealongDataInsufficientDestVectorSize)
{
    // Configure ridealong column
    arrow_type_ = arrow::float32();
    name_ = "col1";
    icd_elem_.elem_word_count_ = 1;
    bool ridealong = true;
    icd_elem_.offset_ = 3;
    row_group_size_ = 1;  // allocated vector too small given append count and words_per_value
    TranslatableColumnTemplate<uint16_t, int32_t> col;
    result_ = col.Configure(name_, ridealong, arrow_type_, row_group_size_, icd_elem_, thread_index_);
    EXPECT_TRUE(result_);

    size_t count = 40;
    std::vector<int32_t> ridealong_data = CreateRawDataVector<int32_t>(count);
    data_ = reinterpret_cast<uint8_t*>(ridealong_data.data());
    result_ = col.AppendRidealongData(data_, count);
    EXPECT_TRUE(result_);

    result_ = col.AppendRidealongData(data_, count);
    EXPECT_FALSE(result_);
}

TEST_F(TranslatableColumnTemplateTest, AppendRidealongData)
{
    // Configure ridealong column
    arrow_type_ = arrow::float32();
    name_ = "col1";
    icd_elem_.elem_word_count_ = 2;
    bool ridealong = true;
    row_group_size_ = 4;
    TranslatableColumnTemplate<uint16_t, int32_t> col;
    result_ = col.Configure(name_, ridealong, arrow_type_, row_group_size_, icd_elem_, thread_index_);
    EXPECT_TRUE(result_);

    size_t count = 6;
    std::vector<int32_t> raw_data1 = CreateRawDataVector<int32_t>(count);
    data_ = reinterpret_cast<uint8_t*>(raw_data1.data());
    result_ = col.AppendRidealongData(data_, count);
    EXPECT_TRUE(result_);
    EXPECT_EQ(col.translated_data.at(0), raw_data1[0]);

    std::vector<int32_t> raw_data2 = CreateRawDataVector<int32_t>(count);
    data_ = reinterpret_cast<uint8_t*>(raw_data2.data());
    result_ = col.AppendRidealongData(data_, count);
    EXPECT_TRUE(result_);
    EXPECT_EQ(col.translated_data.at(1), raw_data2[0]);
}

TEST_F(TranslatableColumnTemplateTest, TranslateIgnoreRidealongCols)
{
    // Configure ridealong column
    arrow_type_ = arrow::float32();
    name_ = "col1";
    icd_elem_.elem_word_count_ = 1;
    bool ridealong = true;
    icd_elem_.offset_ = 3;
    row_group_size_ = 2;
    ICDTranslate icd_translate;
    TranslatableColumnTemplate<uint16_t, int32_t> col;
    result_ = col.Configure(name_, ridealong, arrow_type_, row_group_size_, icd_elem_, thread_index_);
    ASSERT_TRUE(result_);
    result_ = col.Translate(icd_translate);
    EXPECT_FALSE(result_);
}