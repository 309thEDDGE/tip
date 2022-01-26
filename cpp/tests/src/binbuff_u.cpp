#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <cstdint>
#include <cinttypes>
#include "binbuff.h"

class BinBuffTest : public ::testing::Test
{
   protected:
    BinBuff bb_;
    uint64_t result_read_size_;
    uint64_t seek_file_size_;
    uint64_t requested_read_pos_;
    uint64_t requested_read_size_;
    std::ifstream infile_;
    std::string temp_file_name_;
    size_t write_size_;

    BinBuffTest() : bb_(), result_read_size_(UINT64_MAX), seek_file_size_(UINT64_MAX), temp_file_name_("binbuff_test_temp_file.bin"), write_size_(0), requested_read_pos_(0) {}
    /*void SetUp() override
	{

	}*/

    void CreateTempFile(size_t write_count)
    {
        std::ofstream outfile(temp_file_name_.c_str(), std::ios::out | std::ios::binary);
        write_size_ = write_count;
        std::vector<uint8_t> out_data(write_size_, 0);
        outfile.write((char*)out_data.data(), write_size_);
        outfile.close();
    }

    void CreateByteIndexTempFile(size_t write_count)
    {
        std::ofstream outfile(temp_file_name_.c_str(), std::ios::out | std::ios::binary);
        write_size_ = write_count;
        std::vector<uint8_t> out_data(write_size_, 0);
        for (int i = 0; i < write_size_; ++i)
            out_data[i] = i;
        outfile.write((char*)out_data.data(), write_size_);
        outfile.close();
    }

    void CreateFindPatternTestTempFile()
    {
        std::ofstream outfile(temp_file_name_.c_str(), std::ios::out | std::ios::binary);
        write_size_ = 0;

        // pos 0: write int16
        int16_t i16val = -300;
        outfile.write((char*)&i16val, sizeof(i16val));
        write_size_ += sizeof(i16val);

        // pos 2: write 32 zero bytes
        uint8_t d = 0;
        size_t n_bytes = 32;
        size_t i = 0;
        for (i = 0; i < n_bytes; ++i)
            outfile.write((char*)&d, 1);
        write_size_ += n_bytes;

        // pos 34 : Write int32
        int32_t i32val = -300;
        outfile.write((char*)&i32val, sizeof(i32val));
        write_size_ += sizeof(i32val);

        // pos 38: write uint64_t
        uint64_t ui64val = 3462111;
        outfile.write((char*)&ui64val, sizeof(ui64val));
        write_size_ += sizeof(ui64val);

        // pos 46: write 64 zeros
        n_bytes = 64;
        for (i = 0; i < n_bytes; ++i)
            outfile.write((char*)&d, 1);
        write_size_ += n_bytes;

        // pos 110: write uint16 value
        uint16_t ui16val = 8112;
        outfile.write((char*)&ui16val, sizeof(ui16val));
        write_size_ += sizeof(ui16val);

        // pos 112: write 192 zeros
        n_bytes = 192;
        for (i = 0; i < n_bytes; ++i)
            outfile.write((char*)&d, 1);
        write_size_ += n_bytes;

        // pos 304: write int16_t matching previous
        i16val = -300;
        outfile.write((char*)&i16val, sizeof(i16val));
        write_size_ += sizeof(i16val);

        // pos 306: write 96 zeros
        n_bytes = 96;
        for (i = 0; i < n_bytes; ++i)
            outfile.write((char*)&d, 1);
        write_size_ += n_bytes;

        // pos 402: write uint32_t value
        uint32_t ui32val = 66069;
        outfile.write((char*)&ui32val, sizeof(ui32val));
        write_size_ += sizeof(ui32val);

        // pos 406: write float value
        float fval = 80021.345;
        outfile.write((char*)&fval, sizeof(fval));
        write_size_ += sizeof(fval);

        // pos 410: write 64 zeros
        n_bytes = 64;
        for (i = 0; i < n_bytes; ++i)
            outfile.write((char*)&d, 1);
        write_size_ += n_bytes;

        // pos 474: write matching float value
        outfile.write((char*)&fval, sizeof(fval));
        write_size_ += sizeof(fval);

        // pos 478: write 64 zeros
        n_bytes = 64;
        for (i = 0; i < n_bytes; ++i)
            outfile.write((char*)&d, 1);
        write_size_ += n_bytes;

        // pos 542: write another matching int16_t value
        i16val = -300;
        outfile.write((char*)&i16val, sizeof(i16val));
        write_size_ += sizeof(i16val);

        // pos 544: write 96 zeros
        n_bytes = 96;
        for (i = 0; i < n_bytes; ++i)
            outfile.write((char*)&d, 1);
        write_size_ += n_bytes;

        // pos 640: write matching uint32_t
        ui32val = 66069;
        outfile.write((char*)&ui32val, sizeof(ui32val));
        write_size_ += sizeof(ui32val);

        // pos 644
        //printf("write_size_ is %zu\n", write_size_);
        outfile.close();
    }

    bool OpenIFStream()
    {
        infile_.open(temp_file_name_, std::ios::binary | std::ios::ate);
        if (!(infile_.is_open()))
        {
            printf("Error opening file: %s\n", temp_file_name_.c_str());
            return false;
        }
        seek_file_size_ = infile_.tellg();

        if (seek_file_size_ != write_size_)
        {
            printf("Seek file size (%" PRIu64 ") not equal to write size (%zu)\n", seek_file_size_,
                   write_size_);
            return false;
        }

        return true;
    }

    void TearDown() override
    {
        remove(temp_file_name_.c_str());
    }
};

TEST_F(BinBuffTest, InitializeReadPositionGTFileSize)
{
    // File size
    size_t write_count = 1000;
    CreateTempFile(write_count);
    ASSERT_TRUE(OpenIFStream());

    // Requested read pos > file size.
    requested_read_pos_ = 1010;
    requested_read_size_ = 10;
    EXPECT_TRUE(bb_.Initialize(infile_, seek_file_size_,
                               requested_read_pos_, requested_read_size_) == UINT64_MAX);
    infile_.close();
}

TEST_F(BinBuffTest, InitializeActualReadLTExpected)
{
    // File size
    size_t write_count = 100;
    CreateTempFile(write_count);
    ASSERT_TRUE(OpenIFStream());

    requested_read_pos_ = 80;
    requested_read_size_ = 100;
    EXPECT_EQ(bb_.Initialize(infile_, seek_file_size_,
                             requested_read_pos_, requested_read_size_),
              20);
    infile_.close();
}

TEST_F(BinBuffTest, DataNotInitialized)
{
    const uint8_t* data_ptr = bb_.Data();
    EXPECT_EQ(data_ptr, nullptr);
}

TEST_F(BinBuffTest, InitializeBufferDataCorrect)
{
    // Note: this test also tests re-initialization.

    // File size
    size_t write_count = 1000;
    CreateByteIndexTempFile(write_count);
    ASSERT_TRUE(OpenIFStream());

    requested_read_pos_ = 0;
    requested_read_size_ = 100;
    std::vector<uint8_t> expected_data(requested_read_size_);
    for (int i = 0; i < requested_read_size_; ++i)
        expected_data[i] = i;
    EXPECT_EQ(bb_.Initialize(infile_, seek_file_size_,
                             requested_read_pos_, requested_read_size_),
              100);
    EXPECT_EQ(bb_.buffer_size_, requested_read_size_);
    EXPECT_THAT(std::vector<uint8_t>(bb_.Data(), bb_.Data() + requested_read_size_),
                ::testing::ElementsAreArray(expected_data));

    requested_read_pos_ = 100;
    for (int i = 0; i < requested_read_size_; ++i)
        expected_data[i] = i + 100;
    EXPECT_EQ(bb_.Initialize(infile_, seek_file_size_,
                             requested_read_pos_, requested_read_size_),
              100);
    EXPECT_THAT(std::vector<uint8_t>(bb_.Data(), bb_.Data() + requested_read_size_),
                ::testing::ElementsAreArray(expected_data));

    requested_read_pos_ = 950;
    expected_data.resize(50);
    for (int i = 0; i < 50; ++i)
        expected_data[i] = i + 950;
    EXPECT_EQ(bb_.Initialize(infile_, seek_file_size_,
                             requested_read_pos_, requested_read_size_),
              50);
    EXPECT_THAT(std::vector<uint8_t>(bb_.Data(), bb_.Data() + 50),
                ::testing::ElementsAreArray(expected_data));
    infile_.close();
}

TEST_F(BinBuffTest, AdvanceReadPos)
{
    // File size
    size_t write_count = 1000;
    CreateByteIndexTempFile(write_count);
    ASSERT_TRUE(OpenIFStream());

    requested_read_pos_ = 250;
    requested_read_size_ = 100;
    EXPECT_EQ(bb_.Initialize(infile_, seek_file_size_,
                             requested_read_pos_, requested_read_size_),
              requested_read_size_);

    EXPECT_EQ(bb_.AdvanceReadPos(50), 0);
    EXPECT_EQ(bb_.AdvanceReadPos(40), 0);
    EXPECT_EQ(bb_.AdvanceReadPos(20), 1);
    infile_.close();
}

TEST_F(BinBuffTest, SetReadPos)
{
    // File size
    size_t write_count = 100;
    CreateByteIndexTempFile(write_count);
    ASSERT_TRUE(OpenIFStream());

    requested_read_pos_ = 50;
    requested_read_size_ = 50;
    EXPECT_EQ(bb_.Initialize(infile_, seek_file_size_,
                             requested_read_pos_, requested_read_size_),
              requested_read_size_);

    EXPECT_EQ(bb_.SetReadPos(20), 0);
    EXPECT_EQ(bb_.SetReadPos(49), 0);
    EXPECT_EQ(bb_.SetReadPos(50), 1);
    infile_.close();
}

TEST_F(BinBuffTest, BytesAvailable)
{
    // File size
    size_t write_count = 100;
    CreateByteIndexTempFile(write_count);
    ASSERT_TRUE(OpenIFStream());

    requested_read_pos_ = 50;
    requested_read_size_ = 50;
    EXPECT_EQ(bb_.Initialize(infile_, seek_file_size_,
                             requested_read_pos_, requested_read_size_),
              requested_read_size_);

    EXPECT_EQ(bb_.AdvanceReadPos(10), 0);
    EXPECT_EQ(bb_.SetReadPos(40), 0);
    EXPECT_EQ(bb_.BytesAvailable(10), true);
    EXPECT_EQ(bb_.BytesAvailable(11), false);
    infile_.close();
}

TEST_F(BinBuffTest, FindPatternFirstOccurrenceDiffTypes)
{
    CreateFindPatternTestTempFile();
    ASSERT_TRUE(OpenIFStream());

    requested_read_pos_ = 0;

    // No additional data types after run of zeros written
    // at about this location.
    requested_read_size_ = 500;
    EXPECT_EQ(bb_.Initialize(infile_, seek_file_size_,
                             requested_read_pos_, requested_read_size_),
              requested_read_size_);

    // Note that if the positions of these two values were swapped
    // in the output file, then we would search for i32 first,
    // then i16 second. In this case, the first two bytes of the i32 value
    // -300 are the same as the first (and only) two bytes of the i16 value
    // -300 and the i16 value would have been found at position 0 as well.
    // This is a limitation of little-endianness in this case.
    int16_t i16val = -300;
    EXPECT_EQ(bb_.FindPattern(i16val), 0);
    int32_t i32val = -300;
    EXPECT_EQ(bb_.FindPattern(i32val), 34);

    uint64_t ui64val = 3462111;
    EXPECT_EQ(bb_.FindPattern(ui64val), 38);

    uint16_t ui16val = 8112;
    EXPECT_EQ(bb_.FindPattern(ui16val), 110);

    uint32_t ui32val = 66069;
    EXPECT_EQ(bb_.FindPattern(ui32val), 402);

    float fval = 80021.345;
    EXPECT_EQ(bb_.FindPattern(fval), 406);
    infile_.close();
}

TEST_F(BinBuffTest, FindPattern2ndOccurrence)
{
    CreateFindPatternTestTempFile();
    ASSERT_TRUE(OpenIFStream());

    requested_read_pos_ = 304;

    // No additional data types after run of zeros written
    // at about this location.
    requested_read_size_ = 500;
    EXPECT_EQ(bb_.Initialize(infile_, seek_file_size_,
                             requested_read_pos_, requested_read_size_),
              write_size_ - requested_read_pos_);

    int16_t i16val = -300;
    EXPECT_EQ(bb_.FindPattern(i16val), 0);

    float fval = 80021.345;
    EXPECT_EQ(bb_.FindPattern(fval), 102);

    // Advance to position prior to second float value.
    EXPECT_EQ(bb_.AdvanceReadPos(165), 0);
    EXPECT_EQ(bb_.FindPattern(fval), 170);

    // Locate the secon value of -300.
    EXPECT_EQ(bb_.FindPattern(i16val), 238);

    uint32_t ui32val = 66069;
    EXPECT_EQ(bb_.FindPattern(ui32val), 336);
    infile_.close();
}

TEST_F(BinBuffTest, FindPatternCutOffLastValue)
{
    CreateFindPatternTestTempFile();
    ASSERT_TRUE(OpenIFStream());

    requested_read_pos_ = 0;

    // Cut off last byte.
    requested_read_size_ = 404;
    EXPECT_EQ(bb_.Initialize(infile_, seek_file_size_,
                             requested_read_pos_, requested_read_size_),
              requested_read_size_);

    uint32_t ui32val = 66069;
    EXPECT_EQ(bb_.FindPattern(ui32val), UINT64_MAX);
    infile_.close();
}

TEST_F(BinBuffTest, FindPatternWithStartPos)
{
    CreateFindPatternTestTempFile();
    ASSERT_TRUE(OpenIFStream());

    requested_read_pos_ = 0;

    // Cut off last byte.
    requested_read_size_ = write_size_;
    EXPECT_EQ(bb_.Initialize(infile_, seek_file_size_,
                             requested_read_pos_, requested_read_size_),
              requested_read_size_);

    // Search after first occurrence of int16.
    int16_t i16val = -300;
    uint64_t search_pos = 112;
    EXPECT_EQ(bb_.FindPattern(i16val, search_pos), 304);

    // Search after second occurrence of int16.
    search_pos = 402;
    EXPECT_EQ(bb_.FindPattern(i16val, search_pos), 542);

    // Search for second occurrence of uint32.
    search_pos = 540;
    uint32_t ui32val = 66069;
    EXPECT_EQ(bb_.FindPattern(ui32val, search_pos), 640);
    infile_.close();
}

TEST_F(BinBuffTest, FindAllPatternI16)
{
    CreateFindPatternTestTempFile();
    ASSERT_TRUE(OpenIFStream());

    requested_read_pos_ = 0;

    // Cut off last byte.
    requested_read_size_ = write_size_;
    EXPECT_EQ(bb_.Initialize(infile_, seek_file_size_,
                             requested_read_pos_, requested_read_size_),
              requested_read_size_);

    // Note element at position 34. This is the value -300 written as a
    // int32. In the case of little-endian bytes, -300 has the same
    // first two bytes.
    std::vector<uint64_t> expected = {0, 34, 304, 542};
    int16_t i16val = -300;
    EXPECT_THAT(bb_.FindAllPattern(i16val), ::testing::ElementsAreArray(expected));

    // No calls have been made to change the read position. Confirm it
    // hasn't changed.
    EXPECT_EQ(bb_.position_, requested_read_pos_);
    infile_.close();
}

TEST_F(BinBuffTest, FindAllPatternUI32)
{
    CreateFindPatternTestTempFile();
    ASSERT_TRUE(OpenIFStream());

    requested_read_pos_ = 0;

    // Cut off last byte.
    requested_read_size_ = write_size_;
    EXPECT_EQ(bb_.Initialize(infile_, seek_file_size_,
                             requested_read_pos_, requested_read_size_),
              requested_read_size_);

    // Note element at position 34. This is the value -300 written as a
    // int32. In the case of little-endian bytes, -300 has the same
    // first two bytes.
    std::vector<uint64_t> expected = {402, 640};
    uint32_t ui32val = 66069;
    EXPECT_THAT(bb_.FindAllPattern(ui32val), ::testing::ElementsAreArray(expected));

    // No calls have been made to change the read position. Confirm it
    // hasn't changed.
    EXPECT_EQ(bb_.position_, requested_read_pos_);
    infile_.close();
}

TEST_F(BinBuffTest, FindAllPatternFloat)
{
    CreateFindPatternTestTempFile();
    ASSERT_TRUE(OpenIFStream());

    requested_read_pos_ = 0;

    // Cut off last byte.
    requested_read_size_ = write_size_;
    EXPECT_EQ(bb_.Initialize(infile_, seek_file_size_,
                             requested_read_pos_, requested_read_size_),
              requested_read_size_);

    // Note element at position 34. This is the value -300 written as a
    // int32. In the case of little-endian bytes, -300 has the same
    // first two bytes.
    std::vector<uint64_t> expected = {406, 474};
    float fval = 80021.345;
    EXPECT_THAT(bb_.FindAllPattern(fval), ::testing::ElementsAreArray(expected));

    // No calls have been made to change the read position. Confirm it
    // hasn't changed.
    EXPECT_EQ(bb_.position_, requested_read_pos_);
    infile_.close();
}

TEST_F(BinBuffTest, Clear)
{
    // File size
    size_t write_count = 100;
    CreateByteIndexTempFile(write_count);
    ASSERT_TRUE(OpenIFStream());

    requested_read_pos_ = 50;
    requested_read_size_ = 50;
    EXPECT_EQ(bb_.Initialize(infile_, seek_file_size_,
                             requested_read_pos_, requested_read_size_),
              requested_read_size_);

    // Confirm data were read into the buffer as expected
    EXPECT_EQ(bb_.BytesAvailable(50), true);

    // Clear, then confirm variables are reset
    bb_.Clear();
    EXPECT_EQ(bb_.buffer_size_, 0);
    EXPECT_FALSE(bb_.IsInitialized());
    EXPECT_EQ(bb_.Size(), 0);
    EXPECT_EQ(bb_.Capacity(), 0);
    EXPECT_EQ(bb_.BytesAvailable(5), false);
    EXPECT_EQ(bb_.Data(), nullptr);

    infile_.close();
}
