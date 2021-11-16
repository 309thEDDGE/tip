#include "translatable_column_base.h"

TranslatableColumnBase::TranslatableColumnBase() : is_ridealong_(false),
                                                   raw_data_append_count_(0),
                                                   translated_data_append_count_(0),
                                                   col_name_(""),
                                                   have_set_memory_location_(false),
                                                   arrow_type_(nullptr),
                                                   col_name(col_name_),
                                                   raw_data_word_size_(0),
                                                   is_ridealong(is_ridealong_),
                                                   words_per_translated_value_(0),
                                                   words_per_translated_value(words_per_translated_value_),
                                                   payload_offset_(0),
                                                   payload_offset(payload_offset_),
                                                   raw_data_append_count(raw_data_append_count_),
                                                   translated_data_append_count(translated_data_append_count_),
                                                   icd_elem_(),
                                                   thread_index_(0),
                                                   thread_index(thread_index_)
{
}

bool TranslatableColumnBase::Configure(const std::string& name, bool ridealong,
                                       std::shared_ptr<arrow::DataType> arrow_type, const ICDElement& icd_elem,
                                       const size_t& thread_index)
{
    if (name == "")
    {
        SPDLOG_WARN("{:02d} col_name is an empty string", thread_index);
        return false;
    }

    if (arrow_type == nullptr)
    {
        SPDLOG_WARN("{:02d} arrow_type is a null pointer", thread_index);
        return false;
    }

    if (icd_elem.elem_word_count_ < 1)
    {
        SPDLOG_WARN("{:02d} words_per_value < 1", thread_index);
        return false;
    }

    col_name_.assign(name);
    is_ridealong_ = ridealong;
    arrow_type_ = arrow_type;
    words_per_translated_value_ = icd_elem.elem_word_count_;
    payload_offset_ = icd_elem.offset_;
    icd_elem_ = icd_elem;
    thread_index_ = thread_index;

    return true;
}

void TranslatableColumnBase::LogSchema()
{
    SPDLOG_DEBUG(
        "{:02d} Column \"{:s}\": schema({:s}), pyldwords({:02d}), offset({:02d}), "
        "pyldwordsz({:d})",
        thread_index_, col_name_, arrow_type_->ToString(),
        words_per_translated_value_, payload_offset_, raw_data_word_size_);
}
