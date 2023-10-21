#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "translate_tabular_context_base.h"

class TranslateTabularContextBaseTest : public ::testing::Test
{
   protected:
    std::string input_file_ext_;
    size_t word_size_;
    bool result_;
    TranslateTabularContextBase context_;
    std::unordered_map<size_t, std::shared_ptr<TranslatableTableBase>> table_map_;
    std::string table_name_;
    std::string col_name_;
    size_t row_group_size_;
    size_t table_ind_;
    size_t words_per_value_;
    size_t payload_offset_;
    size_t thread_index_;
    std::shared_ptr<TranslatableTableBase> table_;
    ManagedPath temp_path_;

   public:
    TranslateTabularContextBaseTest() : context_(), input_file_ext_(""), word_size_(0), result_(false), table_name_(""), row_group_size_(0), table_ind_(0), table_(nullptr), col_name_(""), words_per_value_(0), payload_offset_(0), temp_path_({".", "temp_dir"}), thread_index_(0)
    {
    }

    ~TranslateTabularContextBaseTest()
    {
        if (temp_path_.is_directory())
            temp_path_.remove();
    }

    bool Configure()
    {
        // SetColumnNames is called, data_col_names has non-zero size
        std::vector<std::string> ridealong;
        std::vector<std::string> data_col{"col1", "col2"};
        context_.SetColumnNames(ridealong, data_col);

        return context_.IsConfigured();
    }

    bool CreateTable()
    {
        if (!Configure())
            return false;
        table_name_ = "table";
        row_group_size_ = 1000;
        table_ind_ = 2;
        if (!temp_path_.create_directory())
            return false;
        ManagedPath outpath(temp_path_ / "outfile.parquet");
        result_ = context_.CreateTranslatableTable(table_name_, row_group_size_,
                                                   table_ind_, table_map_, outpath, thread_index_);
        if (!result_)
            return false;

        if (table_map_.size() != 1)
            return false;
        return true;
    }
};

TEST_F(TranslateTabularContextBaseTest, Clone)
{
    ASSERT_TRUE(Configure());
    std::shared_ptr<TranslateTabularContextBase> ctx = context_.Clone();
    EXPECT_THAT(context_.data_col_names, ::testing::ElementsAreArray(ctx->data_col_names));
}

TEST_F(TranslateTabularContextBaseTest, CreateTranslatableTableNotConfigured)
{
    // table_name_ is init as empty string, which will cause configure to fail
    ASSERT_FALSE(context_.IsConfigured());
    ManagedPath output_path("");
    result_ = context_.CreateTranslatableTable(table_name_, row_group_size_,
                                               table_ind_, table_map_, output_path, thread_index_);
    EXPECT_FALSE(result_);
}

TEST_F(TranslateTabularContextBaseTest, CreateTranslatableTableDirNonexistant)
{
    // table_name_ is init as empty string, which will cause configure to fail
    ASSERT_TRUE(Configure());
    ASSERT_TRUE(context_.IsConfigured());
    ManagedPath output_path("data/file.txt");
    result_ = context_.CreateTranslatableTable(table_name_, row_group_size_,
                                               table_ind_, table_map_, output_path, thread_index_);
    EXPECT_FALSE(result_);
}

TEST_F(TranslateTabularContextBaseTest, CreateTranslatableTableTableNotConfigured)
{
    ASSERT_TRUE(Configure());

    // table_name_ is init as empty string, which will cause configure to fail
    ManagedPath output_path("data/file.txt");
    result_ = context_.CreateTranslatableTable(table_name_, row_group_size_,
                                               table_ind_, table_map_, output_path, thread_index_);
    EXPECT_FALSE(result_);
}

TEST_F(TranslateTabularContextBaseTest, CreateTranslatableTableVectorFilled)
{
    ASSERT_TRUE(Configure());

    table_name_ = "table";
    row_group_size_ = 1000;
    table_ind_ = 21;
    ManagedPath output_path;
    output_path /= "temp_dir_name_for_testing";
    output_path /= "file.txt";
    ASSERT_TRUE(output_path.parent_path().create_directory());
    result_ = context_.CreateTranslatableTable(table_name_, row_group_size_,
                                               table_ind_, table_map_, output_path, thread_index_);
    ASSERT_TRUE(result_);
    ASSERT_TRUE(output_path.parent_path().remove());
    ASSERT_EQ(1, table_map_.size());
    ASSERT_TRUE(table_map_.find(table_ind_) != table_map_.end());
    ASSERT_EQ(table_name_, table_map_.at(table_ind_)->name);
}

TEST_F(TranslateTabularContextBaseTest, AppendColumnPreCheckNullTable)
{
    result_ = context_.AppendColumnPreCheck(table_);
    EXPECT_FALSE(result_);
}

TEST_F(TranslateTabularContextBaseTest, AppendColumnPreCheckNotConfigured)
{
    table_ = std::make_shared<TranslatableTableBase>();
    result_ = context_.AppendColumnPreCheck(table_);
    EXPECT_FALSE(result_);
}

TEST_F(TranslateTabularContextBaseTest, AppendColumnPreCheckTableNotConfigured)
{
    table_ = std::make_shared<TranslatableTableBase>();
    ASSERT_TRUE(Configure());
    result_ = context_.AppendColumnPreCheck(table_);
    EXPECT_FALSE(result_);
}

TEST_F(TranslateTabularContextBaseTest, AppendColumnPreCheck)
{
    ASSERT_TRUE(CreateTable());
    result_ = context_.AppendColumnPreCheck(table_map_.at(table_ind_));
    EXPECT_TRUE(result_);
}

TEST_F(TranslateTabularContextBaseTest, GetArrowTypeUndefinedSpecialization)
{
    // Initialize to something other than nullptr
    std::shared_ptr<arrow::DataType> arrow_type = arrow::int16();
    arrow_type = context_.GetArrowType<void>();
    EXPECT_EQ(nullptr, arrow_type);
}

TEST_F(TranslateTabularContextBaseTest, GetArrowTypeBool)
{
    std::shared_ptr<arrow::DataType> arrow_type = nullptr;
    arrow_type = context_.GetArrowType<bool>();
    EXPECT_EQ(arrow::boolean(), arrow_type);
}

TEST_F(TranslateTabularContextBaseTest, GetArrowTypeInt8)
{
    std::shared_ptr<arrow::DataType> arrow_type = nullptr;
    arrow_type = context_.GetArrowType<int8_t>();
    EXPECT_EQ(arrow::int8(), arrow_type);
}

TEST_F(TranslateTabularContextBaseTest, GetArrowTypeInt16)
{
    std::shared_ptr<arrow::DataType> arrow_type = nullptr;
    arrow_type = context_.GetArrowType<int16_t>();
    EXPECT_EQ(arrow::int16(), arrow_type);
}

TEST_F(TranslateTabularContextBaseTest, GetArrowTypeUInt8)
{
    std::shared_ptr<arrow::DataType> arrow_type = nullptr;
    arrow_type = context_.GetArrowType<uint8_t>();
    EXPECT_EQ(arrow::int16(), arrow_type);
}

TEST_F(TranslateTabularContextBaseTest, GetArrowTypeInt32)
{
    std::shared_ptr<arrow::DataType> arrow_type = nullptr;
    arrow_type = context_.GetArrowType<int32_t>();
    EXPECT_EQ(arrow::int32(), arrow_type);
}

TEST_F(TranslateTabularContextBaseTest, GetArrowTypeUInt16)
{
    std::shared_ptr<arrow::DataType> arrow_type = nullptr;
    arrow_type = context_.GetArrowType<uint16_t>();
    EXPECT_EQ(arrow::int32(), arrow_type);
}

TEST_F(TranslateTabularContextBaseTest, GetArrowTypeInt64)
{
    std::shared_ptr<arrow::DataType> arrow_type = nullptr;
    arrow_type = context_.GetArrowType<int64_t>();
    EXPECT_EQ(arrow::int64(), arrow_type);
}

TEST_F(TranslateTabularContextBaseTest, GetArrowTypeUInt64)
{
    std::shared_ptr<arrow::DataType> arrow_type = nullptr;
    arrow_type = context_.GetArrowType<uint64_t>();
    EXPECT_EQ(arrow::int64(), arrow_type);
}

TEST_F(TranslateTabularContextBaseTest, GetArrowTypeFloat)
{
    std::shared_ptr<arrow::DataType> arrow_type = nullptr;
    arrow_type = context_.GetArrowType<float>();
    EXPECT_EQ(arrow::float32(), arrow_type);
}

TEST_F(TranslateTabularContextBaseTest, GetArrowTypeDouble)
{
    std::shared_ptr<arrow::DataType> arrow_type = nullptr;
    arrow_type = context_.GetArrowType<double>();
    EXPECT_EQ(arrow::float64(), arrow_type);
}

TEST_F(TranslateTabularContextBaseTest, GetArrowTypeChar)
{
    std::shared_ptr<arrow::DataType> arrow_type = nullptr;
    arrow_type = context_.GetArrowType<char>();
    EXPECT_EQ(arrow::utf8(), arrow_type);
}

TEST_F(TranslateTabularContextBaseTest, AppendColumnFailPrecheck)
{
    // table_ is nullptr, ought to fail precheck.
    ICDElement icd_elem;
    result_ = context_.AppendColumn<uint16_t, int32_t>(table_,
                                                       col_name_, false, icd_elem);
    ASSERT_FALSE(result_);
}

TEST_F(TranslateTabularContextBaseTest, AppendColumnBadArrowType)
{
    // Prepare context and table to pass precheck.
    ASSERT_TRUE(CreateTable());

    // TranslatedType is void, which is an undefined specialization.
    ICDElement icd_elem;
    result_ = context_.AppendColumn<uint16_t, int>(table_map_.at(2),
                                                   col_name_, false, icd_elem);
    ASSERT_FALSE(result_);
}

TEST_F(TranslateTabularContextBaseTest, AppendColumnAppendTranslatableColumnFailure)
{
    // Prepare context and table to pass precheck.
    ASSERT_TRUE(CreateTable());

    // TranslatedType should result in valid arrow type.
    // col_name_ and words_per_value_ are not valid.
    ICDElement icd_elem;
    icd_elem.elem_word_count_ = 0;
    result_ = context_.AppendColumn<uint16_t, int32_t>(table_map_.at(2),
                                                       col_name_, false, icd_elem);
    ASSERT_FALSE(result_);
}

TEST_F(TranslateTabularContextBaseTest, AppendColumn)
{
    // Prepare context and table to pass precheck.
    ASSERT_TRUE(CreateTable());

    col_name_ = "col1";
    table_ = table_map_.at(2);
    ICDElement icd_elem;
    icd_elem.elem_word_count_ = 2;
    result_ = context_.AppendColumn<uint16_t, int32_t>(table_,
                                                       col_name_, false, icd_elem);
    ASSERT_TRUE(result_);

    EXPECT_TRUE(table_->is_valid);
    ASSERT_TRUE(table_->GetColumnCount() == 1);
    EXPECT_EQ(col_name_, table_->GetColumnByIndex(0)->col_name);
}