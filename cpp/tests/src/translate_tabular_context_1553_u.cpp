#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "translate_tabular_context_1553.h"

class TranslateTabularContext1553Test : public ::testing::Test
{
   protected:
    std::string input_file_ext_;
    size_t word_size_;
    bool result_;
    ICDData icd_data_;
    std::set<std::string> selected_msg_names_;
    TranslateTabularContext1553 context_;

    TranslateTabularContext1553Test() : icd_data_(), input_file_ext_(""), word_size_(0), result_(false), context_(icd_data_, selected_msg_names_)
    {
    }
};

TEST_F(TranslateTabularContext1553Test, Clone)
{
    std::vector<std::string> data_col_names{"a", "b"};
    std::vector<std::string> ridealong_col_names;
    context_.SetColumnNames(ridealong_col_names, data_col_names);
    ASSERT_TRUE(context_.IsConfigured());
    std::shared_ptr<TranslateTabularContextBase> ctx = context_.Clone();
    ASSERT_TRUE(ctx->IsConfigured());
    EXPECT_THAT(data_col_names, ctx->data_col_names);
}

TEST_F(TranslateTabularContext1553Test, CreateTableOutputPathEmptyBaseName)
{
    ManagedPath outdir("data");
    outdir /= "translated";
    ManagedPath base_name("");
    std::string table_name = "table";
    size_t index = 3;
    ManagedPath expected("data");
    expected /= "translated";
    expected /= (table_name + ".parquet");
    expected /= "03.parquet";

    EXPECT_EQ(expected.RawString(), context_.CreateTableOutputPath(outdir,
                                                                   base_name, table_name, index)
                                        .RawString());
}

TEST_F(TranslateTabularContext1553Test, CreateTableOutputPath)
{
    ManagedPath outdir("data");
    outdir /= "translated";
    ManagedPath base_name("table");
    std::string table_name = "table";

    size_t index = 3;
    ManagedPath expected("data");
    expected /= "translated";
    expected /= (table_name + ".parquet");
    expected /= "table03.parquet";

    EXPECT_EQ(expected.RawString(), context_.CreateTableOutputPath(outdir,
                                                                   base_name, table_name, index)
                                        .RawString());
}