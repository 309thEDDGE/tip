#ifndef TRANSLATABLE_COLUMN_H
#define TRANSLATABLE_COLUMN_H

#include <vector>
#include <string>
#include <memory>
#include "translatable_base.h"

//std::unique_ptr<TranslatableBase> make_translatable_column(std::shared_ptr<arrow::DataType> arrow_dtype,
//	std::string& name);

//std::unique_ptr<TranslatableBase> make_translatable_column(CometDatumSchema& comet_dtype,
//	std::string& name);

template <typename T>
class TranslatableColumn : public TranslatableBase
{
   private:
    std::vector<T> translated_data_;

   public:
    std::vector<T>& translated_data_ref_;
    TranslatableColumn(std::string& name, bool is_ride_along, std::shared_ptr<arrow::DataType> arrow_type,
                       const int& row_alloc_count);
    TranslatableColumn(const ICDElement* icd_elem_ptr, bool is_ride_along,
                       std::shared_ptr<arrow::DataType> arrow_type, const int& row_alloc_count);
    void set_alloc_count(int& row_alloc_count);
    void append_translated_value(T& val);

    //void fill_translation_attribs(CometDatumSchema& cds, uint8_t& wordno, uint8_t& bitmsb, bool& is_bitlevel, uint8_t* byte_count);
    //uint8_t translate();
    /*static std::unique_ptr<TranslatableBase> make(std::shared_ptr<arrow::DataType> arrow_dtype, 
		std::string& name, bool is_ride_along);*/
};

template <typename T>
TranslatableColumn<T>::TranslatableColumn(std::string& name, bool is_ride_along, std::shared_ptr<arrow::DataType> arrow_type,
                                          const int& row_alloc_count) : TranslatableBase(name, is_ride_along, arrow_type),
                                                                        translated_data_ref_(translated_data_)
{
    // It is necessary to resize the translated data vector.
    // This constructor is used for columns which don't require
    // translation, such as ridealong data, i.e., the time column
    // or some message metadata that we wish to include with
    // with translated data. In this case, we place the ridealong
    // data directly in the translated_data vector in preparation
    // for writing to Parquet. Therefore, it is resized now to receive
    // values.
    translated_data_.resize(row_alloc_count);
}

template <typename T>
TranslatableColumn<T>::TranslatableColumn(const ICDElement* icd_elem_ptr, bool is_ride_along,
                                          std::shared_ptr<arrow::DataType> arrow_type, const int& row_alloc_count) : TranslatableBase(icd_elem_ptr, is_ride_along, arrow_type), translated_data_ref_(translated_data_)
{
    // Do not resize to allocate memory. This constructor is used for columns
    // that require translation. The CometTranslationUnit functions that
    // compute translated values and fill the output vector automatically
    // resize the output vector.
    //
    // The above message is now invalid because I've configured CometTranslationUnit
    // to NOT resize the output vector.
    translated_data_.resize(row_alloc_count);
    raw_data_.resize(row_alloc_count * icd_elem_ptr_->elem_word_count_);
    raw_data_ptr_ = raw_data_.data();
}

template <typename T>
void TranslatableColumn<T>::set_alloc_count(int& row_alloc_count)
{
    translated_data_.resize(row_alloc_count);
    raw_data_.resize(row_alloc_count * icd_elem_ptr_->elem_word_count_);
    raw_data_ptr_ = raw_data_.data();
}

template <typename T>
void TranslatableColumn<T>::append_translated_value(T& val)
{
    translated_data_[translated_data_append_count_] = val;
    translated_data_append_count_++;
}

//template <typename T>
//void TranslatableColumn<T>::reset_translated_data_append_count()
//{
//	translated_data_append_count_ = 0;
//}

//template <typename T>
//void TranslatableColumn<T>::fill_translation_attribs(CometDatumSchema& cds, uint8_t& wordno,
//	uint8_t& bitmsb, bool& is_bitlevel, uint8_t* byte_count)
//{
//	cds = cds_;
//	wordno = offset_in_payload_ + 1;
//	bitmsb = bitlevel_msb_;
//	is_bitlevel = is_bitlevel_;
//	byte_count = byte_count_;
//}

//template <typename T>
//static std::unique_ptr<TranslatableBase> TranslatableColumn<T>::make(
//	std::shared_ptr<arrow::DataType> arrow_dtype, std::string& name, bool is_ride_along)
//{
//	return std::unique_ptr<TranslatableBase>(new TranslatableColumn<T>(name, is_ride_along));
//}

//std::unique_ptr<TranslatableBase> make_translatable_column(std::shared_ptr<arrow::DataType> arrow_dtype,
//	std::string& name)
//{
//	switch (arrow_dtype->id())
//	{
//	case arrow::Int64Type::type_id:
//	{
//		return std::unique_ptr<TranslatableBase>(new TranslatableColumn<int64_t>(name, true, arrow_dtype->id()));
//	}
//	}
//}

#endif