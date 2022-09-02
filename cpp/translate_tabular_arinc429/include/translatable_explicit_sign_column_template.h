#ifndef TRANSLATABLE_EXPLICIT_SIGN_COLUMN_TEMPLATE_H_
#define TRANSLATABLE_EXPLICIT_SIGN_COLUMN_TEMPLATE_H_

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include "translatable_column_base.h"
#include "translatable_column_template.h"

template <typename RawType, typename TranslatedType>
class TranslatableExplicitSignColumnTemplate : public TranslatableColumnTemplate<RawType, TranslatedType>
{
    private:
        std::vector<int8_t> explicit_sign_;

    public:
        TranslatableExplicitSignColumnTemplate();
        virtual ~TranslatableExplicitSignColumnTemplate();

        virtual bool Translate(ICDTranslate& icd_translate);
        virtual bool Configure(const std::string& name, bool ridealong,
                           std::shared_ptr<arrow::DataType> arrow_type, const size_t& row_group_size,
                           const ICDElement& icd_elem, const size_t& thread_index);
        virtual bool AppendRawData(const uint8_t* data, const size_t& count, int8_t sign);
};


template <typename RawType, typename TranslatedType>
TranslatableExplicitSignColumnTemplate<RawType, TranslatedType>::TranslatableExplicitSignColumnTemplate() :
    TranslatableColumnTemplate<RawType, TranslatedType>()
{ }

template <typename RawType, typename TranslatedType>
TranslatableExplicitSignColumnTemplate<RawType, TranslatedType>::~TranslatableExplicitSignColumnTemplate()
{ }

template <typename RawType, typename TranslatedType>
bool TranslatableExplicitSignColumnTemplate<RawType, TranslatedType>::Translate(
    ICDTranslate& icd_translate)
{
    if (this->is_ridealong_)
    {
        this->translated_data_append_count_ = 0;
        return false;
    }

    this->raw_data_append_count_ = 0;
    if (!icd_translate.TranslateArray(this->raw_data_, this->translated_data_, this->icd_elem_,
        explicit_sign_))
    {
        SPDLOG_WARN("{:02d} Failed to translate column with explici sign \"{:s}\"",
                    this->thread_index_, this->col_name_);
        return false;
    }

    return true;
}

template <typename RawType, typename TranslatedType>
bool TranslatableExplicitSignColumnTemplate<RawType, TranslatedType>::Configure(
    const std::string& name, bool ridealong,
    std::shared_ptr<arrow::DataType> arrow_type, const size_t& row_group_size,
    const ICDElement& icd_elem, const size_t& thread_index)
{
    if (!TranslatableColumnBase::Configure(name, ridealong, arrow_type,
                                           icd_elem, thread_index))
        return false;

    if (ridealong)
    {
        this->translated_data_.resize(row_group_size);
        this->translated_data_ptr_ = this->translated_data_.data();
    }
    else
    {
        size_t raw_data_vec_size = this->words_per_translated_value_ * row_group_size;
        this->raw_data_.resize(raw_data_vec_size, 0);
        this->translated_data_.resize(row_group_size, 0);
        this->raw_data_ptr_ = this->raw_data_.data();
        this->translated_data_ptr_ = this->translated_data_.data();
        explicit_sign_.resize(row_group_size, 1);
    }

    return true;
}

template <typename RawType, typename TranslatedType>
bool TranslatableExplicitSignColumnTemplate<RawType, TranslatedType>::AppendRawData(
    const uint8_t* data, const size_t& count, int8_t sign)
{
    explicit_sign_[this->raw_data_append_count_] = sign;

    if(!TranslatableColumnTemplate<RawType, TranslatedType>::AppendRawData(
        data, count))
        return false;

    return true;
}

#endif  // TRANSLATABLE_EXPLICIT_SIGN_COLUMN_TEMPLATE_H_
