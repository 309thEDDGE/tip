#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ch10_videof0_component.h"

class Ch10VideoF0ComponentTest : public ::testing::Test
{
protected:
    Ch10VideoF0Component component_;
    Ch10Context context_;
    VideoF0CSDWFmt format_;
    const uint8_t* data_ptr_;

    Ch10VideoF0ComponentTest() : context_(0), component_(&context_), data_ptr_(nullptr)
     {
     }
};

TEST_F(Ch10VideoF0ComponentTest, ParserBasicCall)
{
    Ch10Status status;

    status = component_.Parse(data_ptr_);

    ASSERT_EQ(Ch10Status::OK, status);
}
