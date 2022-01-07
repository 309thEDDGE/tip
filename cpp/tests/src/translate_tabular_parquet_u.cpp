#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "translate_tabular_parquet.h"

class TranslateTabularParquetTest : public ::testing::Test
{
   protected:
    bool result_;
    TranslateTabularParquet context_;

    TranslateTabularParquetTest() : result_(false), context_()
    {
    }
};


TEST_F(TranslateTabularParquetTest, CreateTableOutputPathEmptyBaseName)
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

TEST_F(TranslateTabularParquetTest, CreateTableOutputPath)
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

TEST_F(TranslateTabularParquetTest, ReadRowGroupContinueIfAllGroupsRead)
{
    size_t row_group_count = 10;
    size_t row_group_index = 10;
    size_t thread_index = 3;

    // count and index are equal, expect continue
    ASSERT_EQ(TranslateStatus::CONTINUE, context_.ReadRowGroup(thread_index, 
        row_group_count, row_group_index));
}

TEST_F(TranslateTabularParquetTest, ReadRowGroupIndexIncremented)
{
    size_t row_group_count = 10;
    size_t row_group_index = 8;
    size_t curr_row_group_ind = row_group_index;
    size_t thread_index = 3;

    // count and index are equal, expect continue
    ASSERT_EQ(TranslateStatus::OK, context_.ReadRowGroup(thread_index, 
        row_group_count, row_group_index));
    EXPECT_EQ(curr_row_group_ind + 1, row_group_index);    
}