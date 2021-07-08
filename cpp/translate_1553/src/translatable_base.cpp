#include "translatable_base.h"

TranslatableBase::TranslatableBase(const ICDElement* icd_elem_ptr, bool is_ride_along,
                                   std::shared_ptr<arrow::DataType> arrow_type) : is_ride_along_(is_ride_along),
                                                                                  arrow_type_(arrow_type),
                                                                                  append_count_(0),
                                                                                  raw_data_ptr_(nullptr),
                                                                                  raw_data_ref_(raw_data_),
                                                                                  have_set_memory_location_(false),
                                                                                  translated_data_append_count_(0),
                                                                                  icd_elem_ptr_(icd_elem_ptr),
                                                                                  col_name_("")
{
    col_name_ = icd_elem_ptr_->elem_name_;
}

TranslatableBase::TranslatableBase(std::string& col_name, bool is_ride_along,
                                   std::shared_ptr<arrow::DataType> arrow_type) : is_ride_along_(is_ride_along),
                                                                                  arrow_type_(arrow_type),
                                                                                  col_name_(col_name),
                                                                                  append_count_(0),
                                                                                  raw_data_ptr_(nullptr),
                                                                                  raw_data_ref_(raw_data_),
                                                                                  have_set_memory_location_(false),
                                                                                  translated_data_append_count_(0)
{
}

void TranslatableBase::reset_append_count()
{
    append_count_ = 0;
}

void TranslatableBase::reset_translated_data_append_count()
{
    translated_data_append_count_ = 0;
}

TranslatableBase::~TranslatableBase()
{
}

const std::string& TranslatableBase::name()
{
    return col_name_;
}

uint8_t TranslatableBase::offset()
{
    return icd_elem_ptr_->offset_;
}

void TranslatableBase::append_raw_values(const int32_t* data_arr)
{
    std::copy(data_arr, data_arr + icd_elem_ptr_->elem_word_count_, raw_data_ptr_ + append_count_);
    /*for (int i = 0; i < words_per_datum_; i++)
		raw_data_ptr_[append_count_ + i] = data_arr[i];*/
    append_count_ += icd_elem_ptr_->elem_word_count_;
}

uint32_t TranslatableBase::effective_append_count()
{
    return append_count_ / icd_elem_ptr_->elem_word_count_;
}

bool TranslatableBase::is_bitlevel()
{
    return icd_elem_ptr_->is_bitlevel_;
}

const uint32_t& TranslatableBase::append_count()
{
    return append_count_;
}

//arrow::Type::type TranslatableBase::arrow_type()
//{
//	return arrow_type_id_;
//}

std::shared_ptr<arrow::DataType> TranslatableBase::arrow_type()
{
    return arrow_type_;
}

void TranslatableBase::memory_location_configured()
{
    have_set_memory_location_ = true;
}
