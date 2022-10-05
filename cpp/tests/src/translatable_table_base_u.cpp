#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "translatable_table_base.h"

class TranslatableTableBaseTest : public ::testing::Test
{
   protected:
    std::string name_;
    size_t row_group_size_;
    bool result_;
    std::vector<std::shared_ptr<TranslatableColumnBase>> columns_;
    TranslatableTableBase table_;
    std::string col_name_;
    bool is_ridealong_;
    std::shared_ptr<arrow::DataType> arrow_type_;
    size_t words_per_value_;
    size_t payload_offset_;
    ManagedPath output_path_;
    ICDElement icd_elem_;
    size_t thread_index_;

   public:
    TranslatableTableBaseTest() : name_(""), row_group_size_(0), table_(), result_(false), col_name_("col1"), is_ridealong_(false), arrow_type_(arrow::int32()), words_per_value_(2), payload_offset_(0), output_path_("special"), icd_elem_(), thread_index_(0)
    {
    }
};

TEST_F(TranslatableTableBaseTest, ConfigureInvalid)
{
    // Empty name string
    row_group_size_ = 1000;
    result_ = table_.Configure(name_, row_group_size_, output_path_, thread_index_);
    EXPECT_FALSE(result_);

    // row group size <= 0
    row_group_size_ = 0;
    name_ = "table";
    result_ = table_.Configure(name_, row_group_size_, output_path_, thread_index_);
    EXPECT_FALSE(result_);
}

TEST_F(TranslatableTableBaseTest, Configure)
{
    row_group_size_ = 1000;
    name_ = "table";
    thread_index_ = 22;
    result_ = table_.Configure(name_, row_group_size_, output_path_, thread_index_);
    EXPECT_TRUE(result_);

    EXPECT_EQ(name_, table_.name);
    EXPECT_EQ(row_group_size_, table_.row_group_size);
    EXPECT_EQ(output_path_.RawString(), table_.output_path.RawString());
    EXPECT_EQ(thread_index_, table_.thread_index);
    EXPECT_TRUE(table_.is_valid);
}

TEST_F(TranslatableTableBaseTest, AppendTranslatableColumnNotConfigured)
{
    result_ = table_.AppendTranslatableColumn<uint16_t, float>(col_name_, is_ridealong_,
                                                               arrow_type_, icd_elem_, columns_);
    EXPECT_FALSE(result_);
}

TEST_F(TranslatableTableBaseTest, AppendTranslatableColumnNewColConfigureFail)
{
    name_ = "table";
    row_group_size_ = 1000;
    result_ = table_.Configure(name_, row_group_size_, output_path_, thread_index_);
    EXPECT_TRUE(result_);
    EXPECT_TRUE(table_.is_valid);

    // col_name_ is empty string
    col_name_ = "";
    result_ = table_.AppendTranslatableColumn<uint16_t, float>(col_name_, is_ridealong_,
                                                               arrow_type_, icd_elem_, columns_);
    EXPECT_FALSE(result_);
    EXPECT_FALSE(table_.is_valid);
}

TEST_F(TranslatableTableBaseTest, AppendTranslatableColumnValidCol)
{
    name_ = "table";
    row_group_size_ = 1000;
    result_ = table_.Configure(name_, row_group_size_, output_path_, thread_index_);
    EXPECT_TRUE(result_);

    icd_elem_.elem_word_count_ = 2;
    result_ = table_.AppendTranslatableColumn<uint16_t, float>(col_name_, is_ridealong_,
                                                               arrow_type_, icd_elem_, columns_);
    EXPECT_TRUE(result_);

    ASSERT_EQ(1, columns_.size());
    size_t raw_data_vec_size = icd_elem_.elem_word_count_ * row_group_size_;
    EXPECT_EQ(raw_data_vec_size, columns_.at(0)->GetRawDataVectorSize());
}

TEST_F(TranslatableTableBaseTest, AppendTranslatableColumnMapRidealongCol)
{
    name_ = "table";
    row_group_size_ = 1000;
    result_ = table_.Configure(name_, row_group_size_, output_path_, thread_index_);
    EXPECT_TRUE(result_);

    // Append the first column as non-ridealong. Adds an element 0 to the colummns_ vector.
    is_ridealong_ = false;
    result_ = table_.AppendTranslatableColumn<uint16_t, float>(col_name_, is_ridealong_,
                                                               arrow_type_, icd_elem_, columns_);
    EXPECT_TRUE(result_);

    // Append a ridealong column, in index 1.
    is_ridealong_ = true;
    col_name_ = "ra";
    icd_elem_.elem_word_count_ = 1;
    icd_elem_.offset_ = 4;
    result_ = table_.AppendTranslatableColumn<uint16_t, float>(col_name_, is_ridealong_,
                                                               arrow_type_, icd_elem_, columns_);
    EXPECT_TRUE(result_);
    ASSERT_TRUE(table_.ridealong_col_name_to_index_map.size() == 1);
    ASSERT_TRUE(table_.ridealong_col_name_to_index_map.count(col_name_) == 1);
    EXPECT_TRUE(table_.ridealong_col_name_to_index_map.at(col_name_) == 1);
}

TEST_F(TranslatableTableBaseTest, GetColumnByIndexBadIndex)
{
    // columns_ is initialized with zero elements.
    std::shared_ptr<TranslatableColumnBase> col = table_.GetColumnByIndex(0);
    ASSERT_EQ(nullptr, col);

    // Create a valid table and add a column
    name_ = "table";
    row_group_size_ = 1000;
    result_ = table_.Configure(name_, row_group_size_, output_path_, thread_index_);
    EXPECT_TRUE(result_);

    result_ = table_.AppendTranslatableColumn<uint16_t, float>(col_name_, is_ridealong_,
                                                               arrow_type_, icd_elem_);
    EXPECT_TRUE(result_);
    ASSERT_EQ(1, table_.GetColumnCount());

    col = table_.GetColumnByIndex(1);
    EXPECT_EQ(nullptr, col);
}

TEST_F(TranslatableTableBaseTest, GetColumnByIndex)
{
    // Create a valid table and add a column
    name_ = "table";
    row_group_size_ = 1000;
    result_ = table_.Configure(name_, row_group_size_, output_path_, thread_index_);
    EXPECT_TRUE(result_);

    icd_elem_.elem_word_count_ = 2;
    result_ = table_.AppendTranslatableColumn<uint16_t, float>(col_name_, is_ridealong_,
                                                               arrow_type_, icd_elem_);
    EXPECT_TRUE(result_);
    ASSERT_EQ(1, table_.GetColumnCount());

    std::shared_ptr<TranslatableColumnBase> col = table_.GetColumnByIndex(0);
    EXPECT_EQ(icd_elem_.elem_word_count_, col->words_per_translated_value);
    EXPECT_EQ(col_name_, col->ColName());
    EXPECT_EQ(row_group_size_, col->GetTranslatedDataVectorSize());
}

TEST_F(TranslatableTableBaseTest, AppendRawDataAppendCountIncremented)
{
    name_ = "table";
    row_group_size_ = 1000;
    result_ = table_.Configure(name_, row_group_size_, output_path_, thread_index_);
    EXPECT_TRUE(result_);

    // Create and append two columns.
    icd_elem_.elem_word_count_ = 2;
    icd_elem_.offset_ = 2;
    col_name_ = "col1";
    result_ = table_.AppendTranslatableColumn<uint16_t, float>(col_name_, is_ridealong_,
                                                               arrow_type_, icd_elem_);
    ASSERT_TRUE(result_);

    col_name_ = "col2";
    icd_elem_.offset_ = 4;
    result_ = table_.AppendTranslatableColumn<uint16_t, int32_t>(col_name_, is_ridealong_,
                                                                 arrow_type_, icd_elem_);
    ASSERT_TRUE(result_);

    ASSERT_TRUE(table_.GetColumnCount() == 2);
    ASSERT_TRUE(table_.GetColumnByIndex(0)->raw_data_append_count == 0);
    ASSERT_TRUE(table_.GetColumnByIndex(1)->raw_data_append_count == 0);

    // Create fake data and add to columns.
    std::vector<uint8_t> fake_data(100);
    result_ = table_.AppendRawData(fake_data.data(), fake_data.size());
    EXPECT_TRUE(table_.GetColumnByIndex(0)->raw_data_append_count == 1);
    EXPECT_TRUE(table_.GetColumnByIndex(1)->raw_data_append_count == 1);
}

TEST_F(TranslatableTableBaseTest, AppendRawDataSkipRidealong)
{
    name_ = "table";
    row_group_size_ = 1000;
    result_ = table_.Configure(name_, row_group_size_, output_path_, thread_index_);
    EXPECT_TRUE(result_);

    // Create and append two columns.
    col_name_ = "col1";
    result_ = table_.AppendTranslatableColumn<uint16_t, float>(col_name_, is_ridealong_,
                                                               arrow_type_, icd_elem_);
    EXPECT_TRUE(result_);
    col_name_ = "col2";
    is_ridealong_ = true;
    result_ = table_.AppendTranslatableColumn<uint16_t, float>(col_name_, is_ridealong_,
                                                               arrow_type_, icd_elem_);
    EXPECT_TRUE(result_);

    ASSERT_TRUE(table_.GetColumnCount() == 2);
    ASSERT_TRUE(table_.GetColumnByIndex(0)->raw_data_append_count == 0);
    ASSERT_TRUE(table_.GetColumnByIndex(1)->raw_data_append_count == 0);

    // Create fake data and add to columns.
    std::vector<uint8_t> fake_data(100);
    result_ = table_.AppendRawData(fake_data.data(), fake_data.size());
    EXPECT_TRUE(table_.GetColumnByIndex(0)->raw_data_append_count == 1);
    EXPECT_TRUE(table_.GetColumnByIndex(1)->raw_data_append_count == 0);
}

TEST_F(TranslatableTableBaseTest, AppendRidealongColumnDataColNotInMap)
{
    name_ = "table";
    row_group_size_ = 1000;
    result_ = table_.Configure(name_, row_group_size_, output_path_, thread_index_);
    EXPECT_TRUE(result_);

    // I know this column doesn't exist because no columns have been appended
    // to this table.
    std::vector<uint8_t> fake_data(1);
    col_name_ = "col1";
    result_ = table_.AppendRidealongColumnData(fake_data.data(), fake_data.size(),
                                               col_name_);
    EXPECT_FALSE(result_);
}

TEST_F(TranslatableTableBaseTest, AppendRidealongColumnData)
{
    name_ = "table";
    row_group_size_ = 1000;
    result_ = table_.Configure(name_, row_group_size_, output_path_, thread_index_);
    EXPECT_TRUE(result_);

    // Add ridealong column.
    std::vector<uint8_t> fake_data(10);
    col_name_ = "col1";
    icd_elem_.elem_word_count_ = 2;
    icd_elem_.offset_ = 2;
    is_ridealong_ = true;
    result_ = table_.AppendTranslatableColumn<uint16_t, float>(col_name_, is_ridealong_,
                                                               arrow_type_, icd_elem_);
    EXPECT_TRUE(result_);

    icd_elem_.elem_word_count_ = 2;
    icd_elem_.offset_ = 4;
    result_ = table_.AppendRidealongColumnData(fake_data.data(), fake_data.size(),
                                               col_name_);
    EXPECT_TRUE(result_);
    EXPECT_TRUE(table_.GetColumnCount() == 1);
    EXPECT_EQ(table_.GetColumnByIndex(0)->translated_data_append_count, 1);
}

TEST_F(TranslatableTableBaseTest, ConfigurePqContextNotValid)
{
    std::shared_ptr<ParquetContext> pqctx = nullptr;
    bool is_valid = false;  // not valid
    row_group_size_ = 1000;
    result_ = table_.ConfigurePqContext(pqctx, columns_, is_valid, row_group_size_,
                                        output_path_);
    EXPECT_FALSE(result_);
}

TEST_F(TranslatableTableBaseTest, ConfigurePqContextEmptyColumnsVec)
{
    std::shared_ptr<ParquetContext> pqctx = nullptr;
    bool is_valid = true;
    row_group_size_ = 1000;

    // columns_ is initialized empty
    result_ = table_.ConfigurePqContext(pqctx, columns_, is_valid, row_group_size_,
                                        output_path_);
    EXPECT_FALSE(result_);
}

TEST_F(TranslatableTableBaseTest, ConfigurePqContextObjectPtrNotNull)
{
    // not nullptr
    std::shared_ptr<ParquetContext> pqctx = std::make_shared<ParquetContext>();
    bool is_valid = true;
    std::shared_ptr<TranslatableColumnBase> col = std::make_shared<TranslatableColumnBase>();
    columns_.push_back(col);
    row_group_size_ = 1000;
    result_ = table_.ConfigurePqContext(pqctx, columns_, is_valid, row_group_size_,
                                        output_path_);
    EXPECT_FALSE(result_);
}

TEST_F(TranslatableTableBaseTest, ConfigurePqContextRowGroupCountZero)
{
    std::shared_ptr<ParquetContext> pqctx = nullptr;
    bool is_valid = true;
    std::shared_ptr<TranslatableColumnBase> col = std::make_shared<TranslatableColumnBase>();
    columns_.push_back(col);
    row_group_size_ = 0;  // 0 count not allowed
    result_ = table_.ConfigurePqContext(pqctx, columns_, is_valid, row_group_size_,
                                        output_path_);
    EXPECT_FALSE(result_);
}

TEST_F(TranslatableTableBaseTest, ConfigurePqContextOutputPathEmptyString)
{
    std::shared_ptr<ParquetContext> pqctx = nullptr;
    bool is_valid = true;
    std::shared_ptr<TranslatableColumnBase> col = std::make_shared<TranslatableColumnBase>();
    columns_.push_back(col);
    row_group_size_ = 100;
    ManagedPath temp_path("");
    output_path_ = temp_path;
    result_ = table_.ConfigurePqContext(pqctx, columns_, is_valid, row_group_size_,
                                        output_path_);
    EXPECT_FALSE(result_);
}

TEST_F(TranslatableTableBaseTest, ConfigurePqContext)
{
    std::shared_ptr<ParquetContext> pqctx = nullptr;
    bool is_valid = true;
    row_group_size_ = 100;
    ManagedPath temp_path;  // CWD
    temp_path /= "my_test_file.parquet";
    output_path_ = temp_path;
    name_ = "test_table";
    result_ = table_.Configure(name_, row_group_size_, output_path_, thread_index_);

    col_name_ = "col1";
    icd_elem_.elem_word_count_ = 2;
    icd_elem_.offset_ = 4;
    arrow_type_ = arrow::float32();
    result_ = table_.AppendTranslatableColumn<uint16_t, float>(col_name_,
                                                               false, arrow_type_, icd_elem_, columns_);
    ASSERT_TRUE(result_);

    col_name_ = "col2";
    icd_elem_.elem_word_count_ = 2;
    icd_elem_.offset_ = 6;
    arrow_type_ = arrow::int32();
    result_ = table_.AppendTranslatableColumn<uint16_t, int32_t>(col_name_,
                                                                 false, arrow_type_, icd_elem_, columns_);
    ASSERT_TRUE(result_);

    result_ = table_.ConfigurePqContext(pqctx, columns_, is_valid, row_group_size_,
                                        output_path_);
    EXPECT_TRUE(result_);

    ASSERT_TRUE(pqctx != nullptr);
    EXPECT_EQ(row_group_size_, pqctx->row_group_count);
    EXPECT_EQ(columns_.size(), pqctx->GetColumnCount());

    // This test assumes that the path can be written to and that the file
    // is created. The option EnableEmptyFileDeletion() is called in
    // ConfigurePqContext. This automatically deletes an empty file, which
    // output_path_ is since no rows are written to that file after the
    // ParquetContext object is created. For this reason, we don't
    // verify that a file is created or have the need to remove a file.
    pqctx->Close();
}