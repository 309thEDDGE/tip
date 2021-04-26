#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ch10_tmats_component.h"

class Ch10TMATSComponentTest : public ::testing::Test
{
protected:
    Ch10TMATSCSDWFmt tmats_fmt_;
    Ch10TMATSComponent tmats_comp_;
    const uint8_t* data_ptr_;
    const uint8_t* orig_data_ptr_;
    const uint8_t* body_ptr_;
    Ch10Status status_;
    Ch10Context ctx_;
    std::vector<std::string> tmats_vec_;
    uint64_t abs_pos_;
    uint32_t pkt_size_;
    uint32_t body_size_;
    uint32_t rtc1_;
    uint32_t rtc2_;
    uint8_t intrapkt_ts_src_;
    uint8_t time_fmt_;
    Ch10PacketHeaderFmt hdr_fmt_;

    Ch10TMATSComponentTest() : data_ptr_(nullptr), orig_data_ptr_(nullptr),
        status_(Ch10Status::NONE), body_ptr_(nullptr), ctx_(0), tmats_comp_(&ctx_), abs_pos_(0),
        pkt_size_(0), body_size_(0), rtc1_(0), rtc2_(0), intrapkt_ts_src_(0), time_fmt_(1)
    {
        // Fill out values for fake tmats csdw data.
        tmats_fmt_.frmt = 0; // ascii
        tmats_fmt_.srcc = 0; // setup record has not changed
        tmats_fmt_.ch10ver = 0x0B; // 106-15
    }
};

TEST_F(Ch10TMATSComponentTest, ParseTMATSZeroLength)
{
    // Setup the context such that the body size is equal to the
    // CSDW and therefore zero bytes remain for TMATS matter.
    body_size_ = (uint32_t)tmats_comp_.tmats_csdw_elem.size;
    hdr_fmt_.data_size = body_size_;
    ctx_.UpdateContext(abs_pos_, &hdr_fmt_);

    // Set the data_ptr_ to the address of the csdw.
    data_ptr_ = (const uint8_t*)&tmats_fmt_;

    status_ = tmats_comp_.Parse(data_ptr_, tmats_vec_);

    // No additional data should be added to the vector.
    EXPECT_EQ(tmats_vec_.size(), 0);
}

TEST_F(Ch10TMATSComponentTest, ParseTMATSCorrectChars)
{
    // Setup the context such that the body size is greater than 
    // CSDW and therefore the packet body contains some TMATS matter.
    uint32_t extra_size = 100;
    uint32_t csdw_size = (uint32_t)tmats_comp_.tmats_csdw_elem.size;
    body_size_ = csdw_size + extra_size;
    hdr_fmt_.data_size = body_size_;
    std::vector<char> csdw_and_data(body_size_);
   
    ctx_.UpdateContext(abs_pos_, &hdr_fmt_);

    // Set the data_ptr_ to the beginning of the vector of char.
    data_ptr_ = (const uint8_t*)csdw_and_data.data();

    // Fill the data with random values.
    for (uint32_t i = 0; i < body_size_; i++)
        csdw_and_data[i] = (char)(rand() * (127.0 / RAND_MAX));

    // Make a string out of the body part of the vector.
    std::string orig_str(csdw_and_data.data() + csdw_size, extra_size);

    status_ = tmats_comp_.Parse(data_ptr_, tmats_vec_);

    // Confirm that the string added to the vector and original string are the same.
    ASSERT_EQ(tmats_vec_.size(), 1);
    EXPECT_EQ(tmats_vec_[0], orig_str);
}