#ifndef TRANSLATABLE_COLUMN_TEMPLATE_H_
#define TRANSLATABLE_COLUMN_TEMPLATE_H_

#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include "translatable_column_base.h"

template <typename RawType, typename TranslatedType>
class TranslatableColumnTemplate : public TranslatableColumnBase
{
   protected:
    // Container for the raw data which are appended as the un-translated
    // data files are read.
    std::vector<RawType> raw_data_;
    RawType* raw_data_ptr_;
    const RawType* source_raw_data_ptr_;

    // Container for translated data
    std::vector<TranslatedType> translated_data_;
    TranslatedType* translated_data_ptr_;
    const TranslatedType* source_translated_data_ptr_;

   public:
    const std::vector<RawType>& raw_data;
    const std::vector<TranslatedType>& translated_data;

    TranslatableColumnTemplate();
    virtual ~TranslatableColumnTemplate();

    virtual size_t GetRawDataVectorSize();
    virtual size_t GetTranslatedDataVectorSize();

    // See TranslatableColumnBase::SetParquetContextMemoryLocation
    virtual void ConfigureParquetContext(std::shared_ptr<ParquetContext>& pq_context);

    // See base class
    virtual bool AppendRawData(const uint8_t* data, const size_t& count);
    virtual bool AppendRidealongData(const uint8_t* data, const size_t& count);

    /*
    Configure base class and allocate memory for raw and translated
    data vectors.

    Args:
        col_name        --> name of column
        ridealong       --> True if column is ridealong column type
        arrow_type      --> Shared pointer to arrow output col type
        row_group_size  --> Size of row group
        icd_elem        --> Contains information about payload source words and 
                            bit configuration schema
        thread_index    --> Index of the thread in which the context is
                            processed, counting from zero for the first
                            thread and incrementing for each additional
                            thread.

    Return:
        True if TranslatableColumnBase::Configure returns true and
        vectors have been allocated with correct size.
    */
    virtual bool Configure(const std::string& name, bool ridealong,
                           std::shared_ptr<arrow::DataType> arrow_type, const size_t& row_group_size,
                           const ICDElement& icd_elem, const size_t& thread_index);

    // Translate specifically for this derived class. See TranslatableColumnBase.
    virtual bool Translate(ICDTranslate& icd_translate);
};

template <typename RawType, typename TranslatedType>
TranslatableColumnTemplate<RawType, TranslatedType>::TranslatableColumnTemplate() : TranslatableColumnBase(),
                                                                                    raw_data_ptr_(nullptr),
                                                                                    translated_data_ptr_(nullptr),
                                                                                    raw_data(raw_data_),
                                                                                    translated_data(translated_data_),
                                                                                    source_raw_data_ptr_(nullptr),
                                                                                    source_translated_data_ptr_(nullptr)
{
    raw_data_word_size_ = sizeof(RawType);
}

template <typename RawType, typename TranslatedType>
TranslatableColumnTemplate<RawType, TranslatedType>::~TranslatableColumnTemplate()
{
}

template <typename RawType, typename TranslatedType>
size_t TranslatableColumnTemplate<RawType, TranslatedType>::GetRawDataVectorSize()
{
    return raw_data_.size();
}

template <typename RawType, typename TranslatedType>
size_t TranslatableColumnTemplate<RawType, TranslatedType>::GetTranslatedDataVectorSize()
{
    return translated_data_.size();
}

template <typename RawType, typename TranslatedType>
void TranslatableColumnTemplate<RawType, TranslatedType>::ConfigureParquetContext(
    std::shared_ptr<ParquetContext>& pq_context)
{
    if (!pq_context->AddField(arrow_type_, col_name_))
    {
        SPDLOG_ERROR("{:02d} Failed to AddField for column {:s}", thread_index_, col_name_);
    }

    if (!pq_context->SetMemoryLocation(translated_data_, col_name_))
    {
        SPDLOG_ERROR("{:02d} Failed to SetMemoryLocation for column {:s}",
                     thread_index_, col_name_);
    }
}

template <typename RawType, typename TranslatedType>
bool TranslatableColumnTemplate<RawType, TranslatedType>::Configure(
    const std::string& name, bool ridealong,
    std::shared_ptr<arrow::DataType> arrow_type, const size_t& row_group_size,
    const ICDElement& icd_elem, const size_t& thread_index)
{
    if (!TranslatableColumnBase::Configure(name, ridealong, arrow_type,
                                           icd_elem, thread_index))
        return false;

    if (ridealong)
    {
        translated_data_.resize(row_group_size);
        translated_data_ptr_ = translated_data_.data();
    }
    else
    {
        size_t raw_data_vec_size = words_per_translated_value_ * row_group_size;
        raw_data_.resize(raw_data_vec_size, 0);
        translated_data_.resize(row_group_size, 0);
        raw_data_ptr_ = raw_data_.data();
        translated_data_ptr_ = translated_data_.data();
    }

    return true;
}

template <typename RawType, typename TranslatedType>
bool TranslatableColumnTemplate<RawType, TranslatedType>::AppendRawData(
    const uint8_t* data, const size_t& count)
{
    /*
    Note: May need higher fidelity on return type than bool for this method,
    AppendRidealongData, and Translate. At least three possible condition
    need to be indicated: successful, failed, continue. For example, 
    if(is_ridealong_) ought to convey continue, not that a problem occurred.
    Similar for the following check for sufficient values in source data.
    However, the third check, for size of raw_data_ vector, ought to return
    the fail condition. This will allow tests to be simple and calling
    functions to appropriately handle conditional results, such as marking
    a TranslatableTable as invalid or continuing processing. 
    */

    // Ridealong columns have no need for raw data.
    if (is_ridealong_)
    {
        // No log entry here to avoid cluttering.
        return false;
    }

    // Check for sufficient quantity of values in source data.
    if ((payload_offset_ + words_per_translated_value_) > count)
    {
        SPDLOG_WARN(
            "{:02d} Column {:s}: Insufficient raw data count ({:d}) given "
            "payload offset ({:d}) and raw words per translated value ({:d})",
            thread_index_, col_name_, count, payload_offset_, words_per_translated_value_);

        // Increment the append count because data should have been added
        // for the current data set (via data ptr). We want the added
        // data to be aligned by index with each call to AppendRawData.
        raw_data_append_count_++;
        return false;
    }

    // Check if column has been configured, i.e., if memory
    // in raw_data_ vector has been allocated.
    if (raw_data_.size() == 0)
    {
        SPDLOG_WARN(
            "{:02d} Column {:s}: raw_data_ vector size is 0. This column "
            "has not been configured. See Configure()",
            thread_index_, col_name_);
        return false;
    }

    // Check for sufficient remaining allocated vector elements.
    if ((raw_data_append_count_ * words_per_translated_value_ + words_per_translated_value_) > raw_data_.size())
    {
        SPDLOG_WARN(
            "{:02d} Column {:s}: Insufficient allocated size in raw_data_"
            " vector ({:d}) given payload offsest ({:d}) and raw words per "
            "translated value ({:d})",
            thread_index_, col_name_,
            raw_data_.size(), payload_offset_, words_per_translated_value_);
        return false;
    }

    // Copy data into vector and update append count.
    source_raw_data_ptr_ = reinterpret_cast<const RawType*>(data);
    std::copy(source_raw_data_ptr_ + payload_offset_,
              source_raw_data_ptr_ + payload_offset_ + words_per_translated_value_,
              raw_data_ptr_ + raw_data_append_count_ * words_per_translated_value_);

    raw_data_append_count_++;

    return true;
}

template <typename RawType, typename TranslatedType>
bool TranslatableColumnTemplate<RawType, TranslatedType>::AppendRidealongData(
    const uint8_t* data, const size_t& count)
{
    // Must be a ridealong column
    if (!is_ridealong_)
    {
        SPDLOG_WARN(
            "{:02d} Column {:s} is a NOT ridealong column. "
            "AppendRidealongData not permitted.",
            thread_index_, col_name_);
        return false;
    }

    // Check if column has been configured, i.e., if memory
    // in raw_data_ vector has been allocated.
    if (translated_data_.size() == 0)
    {
        SPDLOG_WARN(
            "{:02d} Column {:s}: translated_data_ vector size is 0. This column "
            "has not been configured. See Configure()",
            thread_index_, col_name_);
        return false;
    }

    // Check for sufficient remaining allocated vector elements.
    if ((translated_data_append_count_ + 1) > translated_data_.size())
    {
        SPDLOG_WARN(
            "{:02d} Column {:s}: Insufficient allocated size in translated_data_"
            " vector ({:d}) given append_count ({:d})",
            thread_index_, col_name_,
            translated_data_.size(), translated_data_append_count_);
        return false;
    }

    // Copy data into vector and update append count.
    source_translated_data_ptr_ = reinterpret_cast<const TranslatedType*>(data);
    std::copy(source_translated_data_ptr_,
              source_translated_data_ptr_ + 1,
              translated_data_ptr_ + translated_data_append_count_);

    translated_data_append_count_++;

    return true;
}

template <typename RawType, typename TranslatedType>
bool TranslatableColumnTemplate<RawType, TranslatedType>::Translate(
    ICDTranslate& icd_translate)
{
    if (is_ridealong_)
    {
        translated_data_append_count_ = 0;
        return false;
    }

    raw_data_append_count_ = 0;

    if (!icd_translate.TranslateArray(raw_data_,
                                                               translated_data_, icd_elem_))
    {
        SPDLOG_WARN("{:02d} Failed to translate column \"{:s}\"",
                    thread_index_, col_name_);
        return false;
    }

    return true;
}

#endif  // TRANSLATABLE_COLUMN_TEMPLATE_H_
