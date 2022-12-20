#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "translate_tabular.h"

class TranslateTabularContextDerived : public TranslateTabularContextBase
{
   public:
    TranslateTabularContextDerived() {}
};

class TranslateTabularTest : public ::testing::Test
{
   protected:
    std::shared_ptr<TranslateTabularContextBase> context_;
    size_t n_threads_;
    TranslateTabular translate_;
    bool result_;
    std::vector<std::vector<ManagedPath>> thread_paths_;
    std::vector<ManagedPath> file_list_;
    ManagedPath output_dir_;
    ManagedPath output_base_name_;

    TranslateTabularTest() : n_threads_(1),
                             context_(std::make_shared<TranslateTabularContextDerived>()),
                             translate_(n_threads_, context_),
                             result_(false),
                             output_base_name_(""),
                             output_dir_("")
    {
    }

    void FillFileList(const std::vector<std::string>& input_strings)
    {
        file_list_.clear();
        for (int i = 0; i < input_strings.size(); i++)
        {
            ManagedPath temp(input_strings[i]);
            file_list_.push_back(temp);
        }
    }
};

TEST_F(TranslateTabularTest, CheckConfigurationContextNotConfigured)
{
    ASSERT_FALSE(context_->IsConfigured());
    EXPECT_EQ(EX_CONFIG, translate_.CheckConfiguration(context_));
}

TEST_F(TranslateTabularTest, CheckConfigurationSetInputFilesNotCalled)
{
    std::vector<std::string> ridealong_col_names;
    std::vector<std::string> data_col_names{"a", "b"};
    context_->SetColumnNames(ridealong_col_names, data_col_names);
    ASSERT_TRUE(context_->IsConfigured());
    EXPECT_EQ(EX_CONFIG, translate_.CheckConfiguration(context_));
}

TEST_F(TranslateTabularTest, CheckConfigurationFileNotExist)
{
    std::vector<std::string> ridealong_col_names;
    std::vector<std::string> data_col_names{"a", "b"};
    context_->SetColumnNames(ridealong_col_names, data_col_names);
    ASSERT_TRUE(context_->IsConfigured());

    // Path does not exist
    ManagedPath test_path2("my_file.txt");
    file_list_.push_back(test_path2);
    ASSERT_TRUE(translate_.SetInputFiles(file_list_, ".txt"));
    result_ = translate_.CheckConfiguration(context_);
    EXPECT_EQ(EX_NOINPUT, translate_.CheckConfiguration(context_));
}

TEST_F(TranslateTabularTest, AssignFilesToThreads)
{
    // Even file count, single thread
    std::vector<std::string> input1{"a", "b"};
    FillFileList(input1);
    n_threads_ = 1;
    size_t req_thread_count = 1000;
    translate_.AssignFilesToThreads(n_threads_, file_list_, thread_paths_, req_thread_count);
    EXPECT_EQ(n_threads_, req_thread_count);
    ASSERT_EQ(1, thread_paths_.size());
    EXPECT_EQ(2, thread_paths_[0].size());
    EXPECT_EQ("a", thread_paths_[0][0].RawString());
    EXPECT_EQ("b", thread_paths_[0][1].RawString());

    // File count and thread count equal
    n_threads_ = 2;
    thread_paths_.clear();
    translate_.AssignFilesToThreads(n_threads_, file_list_, thread_paths_, req_thread_count);
    EXPECT_EQ(n_threads_, req_thread_count);
    ASSERT_EQ(2, thread_paths_.size());
    EXPECT_EQ(1, thread_paths_[0].size());
    EXPECT_EQ(1, thread_paths_[1].size());
    EXPECT_EQ("a", thread_paths_[0][0].RawString());
    EXPECT_EQ("b", thread_paths_[1][0].RawString());

    // More threads than files
    n_threads_ = 3;
    thread_paths_.clear();
    translate_.AssignFilesToThreads(n_threads_, file_list_, thread_paths_, req_thread_count);
    EXPECT_EQ(2, req_thread_count);
    ASSERT_EQ(2, thread_paths_.size());
    EXPECT_EQ(1, thread_paths_[0].size());
    EXPECT_EQ(1, thread_paths_[1].size());
    EXPECT_EQ("a", thread_paths_[0][0].RawString());
    EXPECT_EQ("b", thread_paths_[1][0].RawString());
}

TEST_F(TranslateTabularTest, AssignFilesToThreadsOneFewerThreads)
{
    std::vector<std::string> input{"1", "2", "3", "4", "5", "6", "7", "8", "9"};
    FillFileList(input);
    n_threads_ = 4;
    size_t req_thread_count = 1000;
    translate_.AssignFilesToThreads(n_threads_, file_list_, thread_paths_, req_thread_count);
    ASSERT_EQ(3, req_thread_count);
    EXPECT_EQ(3, thread_paths_[0].size());
    EXPECT_EQ(3, thread_paths_[1].size());
    EXPECT_EQ(3, thread_paths_[2].size());
    EXPECT_EQ("4", thread_paths_[1][0].RawString());
    EXPECT_EQ("5", thread_paths_[1][1].RawString());
    EXPECT_EQ("6", thread_paths_[1][2].RawString());
}

TEST_F(TranslateTabularTest, CreateTranslationManagerObjectsFailIfNotConfigured)
{
    ASSERT_FALSE(context_->IsConfigured());
    std::vector<std::shared_ptr<TranslationManager>> tm_obj;
    result_ = translate_.CreateTranslationManagerObjects(context_, n_threads_,
                                                         thread_paths_, output_dir_, output_base_name_, tm_obj);
    EXPECT_FALSE(result_);
}

TEST_F(TranslateTabularTest, CreateTranslationManagerObjectsThreadCountNotEqualPathsVectors)
{
    std::vector<std::shared_ptr<TranslationManager>> tm_obj;
    ASSERT_TRUE(thread_paths_.size() != n_threads_);
    result_ = translate_.CreateTranslationManagerObjects(context_, n_threads_,
                                                         thread_paths_, output_dir_, output_base_name_, tm_obj);
    EXPECT_FALSE(result_);
}

TEST_F(TranslateTabularTest, CreateTranslationManagerObjects)
{
    // one thread
    std::vector<std::string> data_col_names{"col1", "col2"};
    std::vector<std::string> ridealong_col_names;
    context_->SetColumnNames(ridealong_col_names, data_col_names);
    n_threads_ = 1;
    std::vector<std::shared_ptr<TranslationManager>> tm_obj_vec;
    std::vector<std::string> file_names{"a", "b"};
    FillFileList(file_names);
    thread_paths_.push_back(file_list_);
    result_ = translate_.CreateTranslationManagerObjects(context_, n_threads_,
                                                         thread_paths_, output_dir_, output_base_name_, tm_obj_vec);
    EXPECT_TRUE(result_);
    EXPECT_EQ(1, tm_obj_vec.size());
    EXPECT_EQ(0, tm_obj_vec.at(0)->thread_index);

    // two threads
    tm_obj_vec.clear();
    std::vector<std::string> file_names2{"c", "d"};
    FillFileList(file_names2);
    thread_paths_.push_back(file_list_);
    n_threads_ = 2;
    result_ = translate_.CreateTranslationManagerObjects(context_, n_threads_,
                                                         thread_paths_, output_dir_, output_base_name_, tm_obj_vec);
    EXPECT_TRUE(result_);
    EXPECT_EQ(2, tm_obj_vec.size());
    EXPECT_EQ(0, tm_obj_vec.at(0)->thread_index);
    EXPECT_EQ(1, tm_obj_vec.at(1)->thread_index);
}

TEST_F(TranslateTabularTest, StartThreads)
{
    std::vector<std::string> data_col_names{"col1", "col2"};
    std::vector<std::string> ridealong_col_names;
    context_->SetColumnNames(ridealong_col_names, data_col_names);
    n_threads_ = 2;
    std::vector<std::shared_ptr<TranslationManager>> tm_obj_vec;
    std::vector<std::string> file_names{"a", "b"};
    FillFileList(file_names);
    thread_paths_.push_back(file_list_);
    std::vector<std::string> file_names2{"c", "d"};
    FillFileList(file_names2);
    thread_paths_.push_back(file_list_);
    result_ = translate_.CreateTranslationManagerObjects(context_, n_threads_,
                                                         thread_paths_, output_dir_, output_base_name_, tm_obj_vec);
    EXPECT_TRUE(result_);

    // Start threads and immediately confirm that they are started.
    std::vector<std::thread> threads;
    translate_.StartThreads(threads, tm_obj_vec);
    ASSERT_EQ(n_threads_, threads.size());
    EXPECT_TRUE(threads[0].joinable());
    EXPECT_TRUE(threads[1].joinable());
    threads[0].join();
    threads[1].join();
}

TEST_F(TranslateTabularTest, JoinThreads)
{
    std::vector<std::string> data_col_names{"col1", "col2"};
    std::vector<std::string> ridealong_col_names;
    context_->SetColumnNames(ridealong_col_names, data_col_names);
    n_threads_ = 2;
    std::vector<std::shared_ptr<TranslationManager>> tm_obj_vec;
    std::vector<std::string> file_names{"a", "b"};
    FillFileList(file_names);
    thread_paths_.push_back(file_list_);
    std::vector<std::string> file_names2{"c", "d"};
    FillFileList(file_names2);
    thread_paths_.push_back(file_list_);
    result_ = translate_.CreateTranslationManagerObjects(context_, n_threads_,
                                                         thread_paths_, output_dir_, output_base_name_, tm_obj_vec);
    EXPECT_TRUE(result_);

    // Start threads and immediately confirm that they are started.
    std::vector<std::thread> threads;
    translate_.StartThreads(threads, tm_obj_vec);
    ASSERT_EQ(n_threads_, threads.size());
    EXPECT_TRUE(threads[0].joinable());
    EXPECT_TRUE(threads[1].joinable());

    // Join threads then confirm that threads are not joinable.
    translate_.JoinThreads(threads, tm_obj_vec);
    EXPECT_FALSE(threads[0].joinable());
    EXPECT_FALSE(threads[1].joinable());
}

TEST_F(TranslateTabularTest, SetInputFilesZeroSize)
{
    std::string extension = ".txt";
    // file_list_ initialized empty
    result_ = translate_.SetInputFiles(file_list_, extension);
    EXPECT_FALSE(result_);
}

TEST_F(TranslateTabularTest, SetInputFilesIncorrectExtension)
{
    std::string extension = ".txt";
    ManagedPath file1("input.txt");
    ManagedPath file2("another.data");  // incorrect ext
    file_list_.push_back(file1);
    file_list_.push_back(file2);
    result_ = translate_.SetInputFiles(file_list_, extension);
    EXPECT_FALSE(result_);
}

TEST_F(TranslateTabularTest, SetInputFilesExtensionMissingLeadingPeriod)
{
    // Incorrect. Ought to be ".txt"
    std::string extension = "txt";
    ManagedPath file1("input.txt");
    file_list_.push_back(file1);
    result_ = translate_.SetInputFiles(file_list_, extension);
    EXPECT_FALSE(result_);
}

TEST_F(TranslateTabularTest, SetInputFiles)
{
    std::string extension = ".txt";
    ManagedPath file1("input.txt");
    file_list_.push_back(file1);
    result_ = translate_.SetInputFiles(file_list_, extension);
    EXPECT_TRUE(result_);
    ASSERT_TRUE(translate_.file_list.size() == 1);
    EXPECT_EQ(file1.RawString(), translate_.file_list.at(0).RawString());
    EXPECT_TRUE(translate_.is_file_list_valid);
}