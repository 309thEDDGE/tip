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

TEST_F(TranslateTabularContext1553Test, IsSelectedMessageNotEnabled)
{
    bool should_select_msg = false;
    std::set<size_t> selected_tables;
    size_t table_ind = 0;
    size_t thread_index = 2;
    ASSERT_TRUE(context_.IsSelectedMessage(thread_index, should_select_msg,
        selected_tables, table_ind));
}

TEST_F(TranslateTabularContext1553Test, IsSelectedMessageNotSelected)
{
    bool should_select_msg = true;
    std::set<size_t> selected_tables{10, 23};
    size_t table_ind = 8;
    size_t thread_index = 2;
    ASSERT_FALSE(context_.IsSelectedMessage(thread_index, should_select_msg,
        selected_tables, table_ind));
}

TEST_F(TranslateTabularContext1553Test, IsSelectedMessageIsSelected)
{
    bool should_select_msg = true;
    std::set<size_t> selected_tables{10, 23};
    size_t table_ind = 23;
    size_t thread_index = 2;
    ASSERT_TRUE(context_.IsSelectedMessage(thread_index, should_select_msg,
        selected_tables, table_ind));
}