// binbuff.cpp

#include "binbuff.h"

BinBuff::BinBuff() : source_pos_(0), read_count_(0), pos_(0), is_initialized_(false), position_(pos_), buffer_size_(buff_size_), buff_size_(0)
{
}

BinBuff::~BinBuff()
{
}

bool BinBuff::IsInitialized() const
{
    return is_initialized_;
}

uint64_t BinBuff::Initialize(std::ifstream& infile, const uint64_t& file_size,
                             const uint64_t& read_pos, const uint64_t& read_count)
{
    if (is_initialized_)
    {
        SPDLOG_DEBUG("already initialized -- deleting bytes");
        bytes_.clear();
        is_initialized_ = false;
    }
    source_pos_ = read_pos;
    pos_ = 0;
    read_count_ = read_count;
    file_size_ = file_size;
    buff_size_ = 0;

    if (source_pos_ > file_size_)
    {
        SPDLOG_WARN("BinBuff::Initialize: read position ({:d}) > file size ({:d})",
                    source_pos_, file_size_);
        return UINT64_MAX;
    }

    // Calculate maximum read size, which may be less than requested read count.
    uint64_t max_read_size = file_size_ - source_pos_;
    if (max_read_size < read_count_)
    {
        SPDLOG_DEBUG("max read size ({:d}) < requested read count ({:d})",
                     max_read_size, read_count_);

        // Adjust the read count for memory allocation.
        read_count_ = max_read_size;
    }

    infile.seekg(source_pos_);
    uint64_t curr_pos = static_cast<uint64_t>(infile.tellg());
    if (curr_pos != source_pos_)
    {
        SPDLOG_DEBUG("seeking to {:d} failed, position at {:d}", source_pos_, curr_pos);
        return UINT64_MAX;
    }

    // Allocate memory for array of bytes.
    bytes_.resize(read_count_);
    is_initialized_ = true;

    // Read bytes from file stream.
    infile.read(reinterpret_cast<char*>(bytes_.data()), read_count_);
    uint64_t actual_read_count = static_cast<uint64_t>(infile.gcount());
    if (actual_read_count < read_count_)
    {
        SPDLOG_DEBUG("Actual read count ({:d}) < requested read size ({:d})",
                     actual_read_count, read_count_);

        // Clear the error bits so the file can be read from again.
        infile.clear();

        // This indicates an error because the count of bytes available for reading
        // has already been computed and ought to succeed.
        return UINT64_MAX;
    }
    buff_size_ = static_cast<uint64_t>(bytes_.size());
    return read_count_;
}

const uint8_t* BinBuff::Data() const
{
    if (!is_initialized_)
        return nullptr;
    return bytes_.data() + pos_;
}

uint8_t BinBuff::AdvanceReadPos(const uint64_t& count)
{
    if (pos_ + count > read_count_ - 1)
    {
        SPDLOG_DEBUG("pos ({:d}) + count ({:d}) > {:d}", pos_, count, read_count_ - 1);
        return 1;
    }
    else
    {
        pos_ += count;
    }
    return 0;
}

uint8_t BinBuff::SetReadPos(const uint64_t& p)
{
    if (p > read_count_ - 1)
    {
        SPDLOG_DEBUG("position ({:d}) > ({:d})", p, read_count_ - 1);
        return 1;
    }
    else
    {
        pos_ = p;
    }
    return 0;
}

bool BinBuff::BytesAvailable(const uint64_t& count) const
{
    // + 1 because we count the byte at the current position.
    if (pos_ + count < read_count_ + 1)
        return true;
    return false;
}

uint64_t BinBuff::Size() const
{
    if (is_initialized_)
        return read_count_;
    else
        return uint64_t(0);
}

uint64_t BinBuff::Capacity() const
{
    return bytes_.capacity();
}

void BinBuff::Clear()
{
    // This is the one way to force deallocation of the memory
    // without allocating the vector on the stack then intentionally
    // going out of scope.
    // vector.clear() and vector.resize(0), change the size of the
    // vector but do not change its vector.capacity().
    bytes_ = std::vector<uint8_t>();
    buff_size_ = 0;
    pos_ = 0;
    read_count_ = 0;
    is_initialized_ = false;
}