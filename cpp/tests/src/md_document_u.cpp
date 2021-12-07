#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "md_category.h"
#include "md_document.h"

class MDDocumentTest : public ::testing::Test
{
   protected:
    MDDocument md_;
    std::vector<std::shared_ptr<MDCategory>> category_vec_;
    std::vector<std::string> label_vec_;
    size_t nonmatched_count_;
    
    MDDocumentTest() : md_(), nonmatched_count_(0) 
    {
    }

    void SetUp() override
    {
    }
};

TEST_F(MDDocumentTest, CountMatchedLabelsEmptyVectors)
{
    label_vec_.push_back("label");

    ASSERT_TRUE(category_vec_.size() == 0);
    EXPECT_FALSE(md_.CountMatchedLabels(category_vec_, label_vec_, nonmatched_count_));

    label_vec_.clear();
    ASSERT_TRUE(label_vec_.size() == 0);
    std::shared_ptr<MDCategory> cat = std::make_shared<MDCategory>("my_cat");
    category_vec_.push_back(cat);
    EXPECT_FALSE(md_.CountMatchedLabels(category_vec_, label_vec_, nonmatched_count_));
}

TEST_F(MDDocumentTest, CountMatchedLabelsAllMatched)
{
    label_vec_.push_back("labela");
    label_vec_.push_back("b");
    label_vec_.push_back("labelc");

    std::shared_ptr<MDCategory> cat0 = std::make_shared<MDCategory>("labela");
    std::shared_ptr<MDCategory> cat1 = std::make_shared<MDCategory>("labelc");
    category_vec_.push_back(cat0);
    category_vec_.push_back(cat1);

    EXPECT_TRUE(md_.CountMatchedLabels(category_vec_, label_vec_, nonmatched_count_));
    EXPECT_EQ(0, nonmatched_count_);
}

TEST_F(MDDocumentTest, CountMatchedLabelsNotAllMatched)
{
    label_vec_.push_back("labela");
    label_vec_.push_back("b");
    label_vec_.push_back("labelc");

    std::shared_ptr<MDCategory> cat0 = std::make_shared<MDCategory>("labela");
    std::shared_ptr<MDCategory> cat1 = std::make_shared<MDCategory>("labelc");
    std::shared_ptr<MDCategory> cat2 = std::make_shared<MDCategory>("labelb");
    category_vec_.push_back(cat0);
    category_vec_.push_back(cat1);
    category_vec_.push_back(cat2);

    EXPECT_TRUE(md_.CountMatchedLabels(category_vec_, label_vec_, nonmatched_count_));
    EXPECT_EQ(1, nonmatched_count_);
}